#pragma once

#include "IPAddress.h"
#include "structures/list.h"

class RoutingTable {
    struct Routes {
        IPAddress destRouterIP; /**< Neighbor router IP address */
        IPAddress nextHopIP;    /**< Next hop router IP address to reach destination */
    };
    List<Routes> entries; /**< List of routing entries */
public:
    /**
     * @brief Retrieves the next hop IP for a given destination IP.
     * @param destIP Destination IP address.
     * @return Next hop IP address, or invalid IP if not found.
     */
    IPAddress getNextHopIP(IPAddress destIP);

    /**
     * @brief Sets the next hop IP for a given destination IP.
     * @param destIP Destination IP address.
     * @param nextHop Next hop IP address.
     */
    void setNextHopIP(IPAddress destIP, IPAddress nextHop);

    /**
     * @brief Returns the number of entries in the routing table.
     * @return Number of routing entries.
     */
    size_t size() const;
};

inline size_t RoutingTable::size() const {
    return entries.size();
}
