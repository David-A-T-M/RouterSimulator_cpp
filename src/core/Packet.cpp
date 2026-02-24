#include "../../include/core/Packet.h"

Packet::Packet(size_t pageID, size_t pagePos, size_t pageLen, IPAddress srcIP, IPAddress dstIP, size_t expTick)
    : pageID(pageID), pagePos(pagePos), pageLen(pageLen), expTick(expTick), srcIP(srcIP), dstIP(dstIP) {

    if (pagePos >= pageLen) {
        throw std::invalid_argument("pagePosition must be in the range [0, pageLength)");
    }
    if (!dstIP.isValid()) {
        throw std::invalid_argument("destinationIP must be valid (not 0.0)");
    }
    if (!srcIP.isValid()) {
        throw std::invalid_argument("originIP must be valid (not 0.0)");
    }
}

std::string Packet::toString() const {
    std::ostringstream oss;

    // Format:   Src: srcIP   -> Dst: dstIP   | ID: PageID(6 digits)-pagePos/pageLen
    // Example: "Src: 001.001 -> Dst: 002.002 | ID: 000010          -2     /3"
    oss << "Src: " << srcIP << " -> Dst: " << dstIP << " | ID: " << std::setw(6) << std::setfill('0') << pageID << "-"
        << pagePos << "/" << pageLen;

    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Packet& packet) {
    os << packet.toString();
    return os;
}
