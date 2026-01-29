#include "core/PageReassembler.h"

// =============== Constructors & Destructor ===============
PageReassembler::PageReassembler(int id, int length)
    : pageID(id), expectedPackets(length), currentPackets(0), packetArray(nullptr) {
    if (id < 0) {
        throw std::invalid_argument("pageID must be non-negative");
    }
    if (length <= 0) {
        throw std::invalid_argument("expectedPackets must be positive");
    }

    packetArray = new Packet*[length];
    for (int i = 0; i < length; ++i) {
        packetArray[i] = nullptr;
    }
}

PageReassembler::~PageReassembler() {
    if (packetArray != nullptr) {
        for (int i = 0; i < expectedPackets; ++i) {
            delete packetArray[i];
        }
        delete[] packetArray;
    }
}

// =============== Query methods ===============
bool PageReassembler::hasPacketAt(int position) const {
    if (position < 0 || position >= expectedPackets) {
        return false;
    }
    return packetArray[position] != nullptr;
}

// =============== Modifiers ===============
bool PageReassembler::addPacket(const Packet& p) {
    if (p.getPageID() != pageID) {
        return false;
    }

    if (p.getPageLength() != expectedPackets) {
        return false;
    }

    const int pos = p.getPagePosition();

    if (pos < 0 || pos >= expectedPackets) {
        return false;
    }

    if (packetArray[pos] != nullptr) {
        return false;
    }

    packetArray[pos] = new Packet(p);
    currentPackets++;

    return true;
}

List<Packet> PageReassembler::package() {
    if (!isComplete()) {
        throw std::runtime_error("Cannot package incomplete page: " + std::to_string(currentPackets) + "/" +
                                 std::to_string(expectedPackets) + " packets received");
    }

    List<Packet> readyList;

    for (int i = 0; i < expectedPackets; ++i) {
        if (packetArray[i] != nullptr) {
            readyList.pushBack(*packetArray[i]);

            delete packetArray[i];
            packetArray[i] = nullptr;
        } else {
            throw std::runtime_error("Missing packet at position " + std::to_string(i));
        }
    }

    currentPackets = 0;

    return readyList;
}

void PageReassembler::reset() {
    for (int i = 0; i < expectedPackets; ++i) {
        delete packetArray[i];
        packetArray[i] = nullptr;
    }
    currentPackets = 0;
}
