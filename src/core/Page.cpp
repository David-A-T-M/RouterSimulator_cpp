#include "core/Page.h"

Page::Page(int pageID, int pageLength, IPAddress originIP, IPAddress destinationIP)
    : pageID(pageID), pageLength(pageLength), originIP(originIP), destinationIP(destinationIP) {
    if (pageID < 0) {
        throw std::invalid_argument("pageID must be non-negative");
    }
    if (pageLength <= 0) {
        throw std::invalid_argument("pageLength must be positive");
    }
    if (!destinationIP.isValid()) {
        throw std::invalid_argument("destinationIP must be valid (not 0.0)");
    }
    if (!originIP.isValid()) {
        throw std::invalid_argument("originIP must be valid (not 0.0)");
    }
}

Page::Page(List<Packet>&& completedPackets) {
    if (completedPackets.isEmpty()) {
        throw std::invalid_argument("Cannot create Page from empty packet list");
    }

    const Packet& firstPacket = completedPackets[0];

    pageID        = firstPacket.getPageID();
    pageLength    = firstPacket.getPageLength();
    originIP      = firstPacket.getOriginIP();
    destinationIP = firstPacket.getDestinationIP();

    if (completedPackets.size() != pageLength) {
        throw std::invalid_argument("Packet count (" + std::to_string(completedPackets.size()) +
                                    ") does not match page length (" + std::to_string(pageLength) + ")");
    }

    for (int i = 0; i < completedPackets.size(); ++i) {
        const Packet& packet = completedPackets[i];

        if (packet.getPageID() != pageID) {
            throw std::invalid_argument("Packet " + std::to_string(i) + " has inconsistent pageID: " +
                                        std::to_string(packet.getPageID()) + " vs " + std::to_string(pageID));
        }

        if (packet.getPageLength() != pageLength) {
            throw std::invalid_argument("Packet " + std::to_string(i) + " has inconsistent pageLength");
        }

        if (packet.getOriginIP() != originIP) {
            throw std::invalid_argument("Packet " + std::to_string(i) + " has inconsistent originIP");
        }

        if (packet.getDestinationIP() != destinationIP) {
            throw std::invalid_argument("Packet " + std::to_string(i) + " has inconsistent destinationIP");
        }

        if (packet.getPagePosition() != i) {
            throw std::invalid_argument("Packet at index " + std::to_string(i) +
                                        " has wrong position: " + std::to_string(packet.getPagePosition()));
        }
    }
}

List<Packet> Page::fragmentToPackets() const {
    return fragmentToPackets(0);
}

List<Packet> Page::fragmentToPackets(int initialPriority) const {
    List<Packet> packets;

    for (int position = 0; position < pageLength; ++position) {
        packets.pushBack(Packet(pageID, position, pageLength, initialPriority, destinationIP, originIP));
    }

    return packets;
}

std::string Page::toString() const {
    std::ostringstream oss;
    oss << "Page{ID=" << pageID << ", Length=" << pageLength << ", From=" << originIP << ", To=" << destinationIP
        << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Page& page) {
    os << page.toString();
    return os;
}
