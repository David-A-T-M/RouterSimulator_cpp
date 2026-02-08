#include "core/Packet.h"

Packet::Packet(int pageID, int pagePosition, int pageLength, int routerPriority, IPAddress destinationIP,
               IPAddress originIP)
    : pageID(pageID),
      pagePosition(pagePosition),
      pageLength(pageLength),
      routerPriority(routerPriority),
      destinationIP(destinationIP),
      originIP(originIP) {

    if (pagePosition >= pageLength) {
        throw std::invalid_argument("pagePosition must be in the range [0, pageLength)");
    }
    if (!destinationIP.isValid()) {
        throw std::invalid_argument("destinationIP must be valid (not 0.0)");
    }
    if (!originIP.isValid()) {
        throw std::invalid_argument("originIP must be valid (not 0.0)");
    }
}

std::string Packet::toString() const {
    std::ostringstream oss;

    // Format: Dest: RouterIP - ID: PageID(9 digits)-Position
    // Example: "Dest: 5 - ID: 000000123-4"
    oss << "Dest: " << destinationIP << " - ID: " << std::setw(9) << std::setfill('0') << pageID << "-" << pagePosition;

    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Packet& packet) {
    os << packet.toString();
    return os;
}
