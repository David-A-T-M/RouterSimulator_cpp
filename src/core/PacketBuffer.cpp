#include "core/PacketBuffer.h"

#include <algorithm>
#include <limits>
#include <sstream>

// =============== Constructors & Destructor ===============
PacketBuffer::PacketBuffer(Mode mode, size_t capacity) : mode(mode), capacity(capacity) {}

// =============== Getters ===============
int PacketBuffer::getMaxPriority() const {
    if (isEmpty())
        return -1;

    int maxPri = packets[0].getRouterPriority();
    for (const auto& packet : packets) {
        if (packet.getRouterPriority() > maxPri) {
            maxPri = packet.getRouterPriority();
        }
    }
    return maxPri;
}

int PacketBuffer::getMinPriority() const {
    if (isEmpty())
        return -1;

    int minPri = packets[0].getRouterPriority();
    for (const auto& packet : packets) {
        if (packet.getRouterPriority() < minPri) {
            minPri = packet.getRouterPriority();
        }
    }
    return minPri;
}

// =============== Queue Operations ===============
bool PacketBuffer::enqueue(const Packet& packet) {
    if (isFull()) {
        return false;
    }

    if (mode == Mode::FIFO) {
        packets.pushBack(packet);
    } else {
        const size_t position = findPriorityPosition(packet);
        packets.insertAt(packet, position);
    }

    return true;
}

Packet PacketBuffer::dequeue() {
    if (isEmpty()) {
        throw std::runtime_error("Cannot dequeue from empty buffer");
    }

    const Packet front = packets[0];
    packets.popFront();
    return front;
}

// =============== Query methods ===============
size_t PacketBuffer::availableSpace() const noexcept {
    if (capacity == 0) {
        return std::numeric_limits<int>::max();
    }
    return capacity - packets.size();
}

double PacketBuffer::getUtilization() const noexcept {
    if (capacity == 0) {
        return 0.0;
    }
    return static_cast<double>(packets.size()) / static_cast<double>(capacity);
}

bool PacketBuffer::contains(int pageID, int pagePosition) const {
    return std::any_of(packets.begin(), packets.end(), [pageID, pagePosition](const Packet& packet) noexcept {
        return packet.getPageID() == pageID && packet.getPagePosition() == pagePosition;
    });
}

size_t PacketBuffer::countPacketsFromPage(int pageID) const {
    return std::count_if(packets.begin(), packets.end(),
                         [pageID](const Packet& packet) { return packet.getPageID() == pageID; });
}

// =============== Buffer Management ===============
void PacketBuffer::clear() noexcept {
    packets.clear();
}

void PacketBuffer::setCapacity(int newCapacity) {
    if (newCapacity < 0) {
        throw std::invalid_argument("Capacity cannot be negative");
    }
    if (newCapacity > 0 && packets.size() > newCapacity) {
        throw std::invalid_argument("Cannot set capacity lower than current size");
    }
    capacity = newCapacity;
}

void PacketBuffer::removeAt(int index) {
    if (index < 0 || index >= packets.size()) {
        throw std::out_of_range("Index out of range");
    }
    packets.removeAt(index);
}

List<Packet> PacketBuffer::extractPacketsByDestinationRouter(uint8_t routerIP) {
    List<Packet> extracted;

    for (size_t i = packets.size(); i > 0; --i) {
        if (packets[i].getDestinationIP().getRouterIP() == routerIP) {
            extracted.pushFront((packets[i - 1]));
            packets.removeAt(i);
        }
    }

    return extracted;
}

int PacketBuffer::transferPacketsByDestination(uint8_t routerIP, PacketBuffer& targetBuffer) {
    int transferred = 0;

    for (size_t i = packets.size(); i > 0; --i) {
        if (packets[i].getDestinationIP().getRouterIP() == routerIP) {
            if (targetBuffer.enqueue((packets[i - 1]))) {
                packets.removeAt(i);
                transferred++;
            }
        }
    }

    return transferred;
}

// =============== Utilities ===============
std::string PacketBuffer::toString() const {
    std::ostringstream oss;
    oss << "PacketBuffer{Mode=";
    oss << (mode == Mode::FIFO ? "FIFO" : "PRIORITY");
    oss << ", Size=" << packets.size();
    if (capacity > 0) {
        oss << "/" << capacity;
    }
    oss << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const PacketBuffer& buffer) {
    os << buffer.toString();
    return os;
}

// =============== Private Methods ===============
size_t PacketBuffer::findPriorityPosition(const Packet& packet) const {
    const int newPriority = packet.getRouterPriority();

    for (int i = 0; i < packets.size(); ++i) {
        if (newPriority < packets[i].getRouterPriority()) {
            return i;
        }
    }

    return packets.size();
}
