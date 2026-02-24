#include "core/PageReassembler.h"

// =============== Constructors & Destructor ===============
PageReassembler::PageReassembler(size_t id, size_t length, size_t expTick)
    : pageID(id), total(length), count(0), expTick(expTick), packets(nullptr) {
    if (length == 0) {
        throw std::invalid_argument("expectedPackets must be positive");
    }

    packets = new Packet*[length];
    for (int i = 0; i < length; ++i) {
        packets[i] = nullptr;
    }
}

PageReassembler::PageReassembler(PageReassembler&& other) noexcept
    : pageID(other.pageID), total(other.total), count(other.count), expTick(other.expTick), packets(other.packets) {
    other.packets = nullptr;
}

PageReassembler& PageReassembler::operator=(PageReassembler&& other) noexcept {
    if (this != &other) {
        if (packets) {
            for (int i = 0; i < total; ++i) {
                delete packets[i];
            }
            delete[] packets;
        }

        pageID  = other.pageID;
        total   = other.total;
        count   = other.count;
        expTick = other.expTick;
        packets = other.packets;

        other.packets = nullptr;
    }

    return *this;
}

PageReassembler::~PageReassembler() {
    if (packets != nullptr) {
        for (int i = 0; i < total; ++i) {
            delete packets[i];
        }
        delete[] packets;
    }
}

// =============== Query methods ===============
bool PageReassembler::hasPacketAt(size_t position) const {
    if (position >= total) {
        throw std::out_of_range("Position out of range in hasPacketAt");
    }
    return packets[position] != nullptr;
}

// =============== Modifiers ===============
bool PageReassembler::addPacket(const Packet& p) {
    if (p.getPageID() != pageID) {
        return false;
    }

    if (p.getPageLen() != total) {
        return false;
    }

    const size_t pos = p.getPagePos();

    if (pos >= total) {
        return false;
    }

    if (packets[pos] != nullptr) {
        return false;
    }

    packets[pos] = new Packet(p);
    count++;

    return true;
}

List<Packet> PageReassembler::package() {
    if (!isComplete()) {
        throw std::runtime_error("Cannot package incomplete page: " + std::to_string(count) + "/" +
                                 std::to_string(total) + " packets received");
    }

    List<Packet> readyList;

    for (int i = 0; i < total; ++i) {
        if (packets[i] != nullptr) {
            readyList.pushBack(*packets[i]);

            delete packets[i];
            packets[i] = nullptr;
        } else {
            throw std::runtime_error("Missing packet at position " + std::to_string(i));
        }
    }

    count = 0;

    return readyList;
}

void PageReassembler::reset() {
    for (int i = 0; i < total; ++i) {
        delete packets[i];
        packets[i] = nullptr;
    }
    count = 0;
}

std::string PageReassembler::toString() const {
    std::ostringstream oss;
    oss << "PageReassembler{ID: " << pageID << " | " << count << "/" << total << " packets received | ExpTick: }"
        << expTick;
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const PageReassembler& reassembler) {
    os << reassembler.toString();
    return os;
}

bool PageReassembler::operator==(const PageReassembler& other) const noexcept {
    return pageID == other.pageID;
}

bool PageReassembler::operator!=(const PageReassembler& other) const noexcept {
    return !(*this == other);
}
