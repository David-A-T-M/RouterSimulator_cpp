#include "core/RoutingTable.h"

IPAddress RoutingTable::getNextHopIP(IPAddress destIP) {
    const IPAddress routerIP(destIP.getRouterIP());
    for (const auto& [destRouterIP, nextHopIP] : entries) {
        if (destRouterIP == routerIP) {
            return nextHopIP;
        }
    }
    return {};  // Return invalid IP if not found
}

void RoutingTable::setNextHopIP(IPAddress destIP, IPAddress nextHop) {
    for (auto& [destRouterIP, nextHopIP] : entries) {
        if (destRouterIP == destIP) {
            nextHopIP = nextHop;
            return;
        }
    }
    entries.pushBack(Routes{destIP, nextHop});
}
