#pragma once

#include "IPAddress.h"
#include "structures/list.h"

/**
 * @class RoutingTable
 * @brief Represents a routing table for a network router, mapping destination router IPs to next
 * hop IPs.
 *
 * The routing table maintains a list of entries, where each entry consists of a destination router
 * IP and the corresponding next hop IP address to reach that destination. The class provides
 * methods to retrieve and set next hop IPs for given destination IPs, as well as to get the number
 * of entries in the routing table.
 */
class RoutingTable {
    /**
     * @struct Routes
     * @brief Represents a routing entry mapping a destination router IP to a next hop IP.
     */
    struct Routes {
        IPAddress destRouterIP; /**< Neighbor router IP address */
        IPAddress nextHopIP;    /**< Next hop router IP address to reach destination */
    };

    List<Routes> entries; /**< List of routing entries */

public:
    /**
     * @brief Retrieves the next hop IP for a given destination IP.
     *
     * @param destIP Destination IP address.
     * @return Next hop IP address if found, or an invalid IP if no route exists for the
     * destination.
     */
    IPAddress getNextHopIP(IPAddress destIP);

    /**
     * @brief Sets the next hop IP for a given destination IP. If an entry for the destination
     * already exists, it updates the next hop IP. If no entry exists, it adds a new routing entry
     * to the table.
     *
     * @param destIP Destination IP address.
     * @param nextHop Next hop IP address.
     */
    void setNextHopIP(IPAddress destIP, IPAddress nextHop);

    /**
     * @brief Gets the number of routing entries currently stored in the routing table.
     *
     * @return Number of routing entries in the table.
     */
    [[nodiscard]] size_t size() const;
};

inline size_t RoutingTable::size() const {
    return entries.size();
}
