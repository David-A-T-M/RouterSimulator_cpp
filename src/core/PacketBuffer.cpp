#include "core/PacketBuffer.h"

#include <algorithm>
#include <limits>
#include <sstream>

// =============== Constructors & Destructor ===============
PacketBuffer::PacketBuffer(size_t capacity) : capacity(capacity), dstIP(IPAddress{}) {}

PacketBuffer::PacketBuffer(IPAddress dstIP, size_t capacity) : capacity(capacity), dstIP(dstIP) {}

// =============== Queue Operations ===============
bool PacketBuffer::enqueue(const Packet& packet) {
    if (isFull()) {
        return false;
    }
    packets.pushBack(packet);
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

bool PacketBuffer::contains(size_t pageID, size_t pagePos) const {
    return std::any_of(packets.begin(), packets.end(), [pageID, pagePos](const Packet& packet) noexcept {
        return packet.getPageID() == pageID && packet.getPagePos() == pagePos;
    });
}

// =============== Buffer Management ===============
void PacketBuffer::clear() noexcept {
    packets.clear();
}

void PacketBuffer::setCapacity(size_t newCapacity) {
    if (newCapacity > 0 && packets.size() > newCapacity) {
        throw std::invalid_argument("Cannot set capacity lower than current size");
    }
    capacity = newCapacity;
}

void PacketBuffer::removeAt(size_t index) {
    if (index >= packets.size()) {
        throw std::out_of_range("Index out of range");
    }
    packets.removeAt(index);
}

// =============== Utilities ===============
std::string PacketBuffer::toString() const {
    std::ostringstream oss;
    oss << "PacketBuffer{Usage: " << packets.size();
    if (capacity > 0) {
        oss << "/" << capacity;
    }
    oss << "Packets | DstIP: " << dstIP << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const PacketBuffer& buffer) {
    os << buffer.toString();
    return os;
}
