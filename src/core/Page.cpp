#include "core/Page.h"

Page::Page(size_t pageID, size_t pageLen, IPAddress srcIP, IPAddress dstIP)
    : pageID(pageID), pageLen(pageLen), srcIP(srcIP), dstIP(dstIP) {
    if (!dstIP.isValid()) {
        throw std::invalid_argument("destinationIP must be valid (not 0.0)");
    }
    if (!srcIP.isValid()) {
        throw std::invalid_argument("originIP must be valid (not 0.0)");
    }
}

Page::Page(List<Packet>&& packets) {
    if (packets.isEmpty()) {
        throw std::invalid_argument("Cannot create Page from empty packet list");
    }

    const Packet& firstPacket = packets[0];

    pageID  = firstPacket.getPageID();
    pageLen = firstPacket.getPageLen();
    srcIP   = firstPacket.getSrcIP();
    dstIP   = firstPacket.getDstIP();

    if (packets.size() != pageLen) {
        throw std::invalid_argument("Packet count (" + std::to_string(packets.size()) +
                                    ") does not match page length (" + std::to_string(pageLen) +
                                    ")");
    }

    for (int i = 0; i < packets.size(); ++i) {
        const Packet& packet = packets[i];

        if (packet.getPageID() != pageID) {
            throw std::invalid_argument(
                "Packet " + std::to_string(i) + " has inconsistent pageID: " +
                std::to_string(packet.getPageID()) + " vs " + std::to_string(pageID));
        }

        if (packet.getPageLen() != pageLen) {
            throw std::invalid_argument("Packet " + std::to_string(i) +
                                        " has inconsistent pageLength");
        }

        if (packet.getSrcIP() != srcIP) {
            throw std::invalid_argument("Packet " + std::to_string(i) +
                                        " has inconsistent originIP");
        }

        if (packet.getDstIP() != dstIP) {
            throw std::invalid_argument("Packet " + std::to_string(i) +
                                        " has inconsistent destinationIP");
        }

        if (packet.getPagePos() != i) {
            throw std::invalid_argument(
                "Packet at index " + std::to_string(i) +
                " has wrong position: " + std::to_string(packet.getPagePos()));
        }
    }
}

List<Packet> Page::toPackets(size_t expTick) const {
    List<Packet> packets;

    for (int pos = 0; pos < pageLen; ++pos) {
        packets.pushBack(Packet(pageID, pos, pageLen, srcIP, dstIP, expTick));
    }

    return packets;
}

std::string Page::toString() const {
    std::ostringstream oss;
    oss << "Page{ID: " << pageID << " | Len: " << pageLen << " | " << srcIP << " -> " << dstIP
        << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Page& page) {
    os << page.toString();
    return os;
}
