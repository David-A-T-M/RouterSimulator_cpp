#pragma once

#include <limits>
#include "core/RoutingTable.h"
#include "structures/list.h"

// Forward declaration
class Router;

/**
 * @class DijkstraAlgorithm
 * @brief Implements Dijkstra's shortest path algorithm for network routing.
 *
 * Calculates the shortest paths between routers and generates routing tables.
 */
class DijkstraAlgorithm {
    static constexpr size_t INF = std::numeric_limits<size_t>::max();

    /**
     * @struct DistanceInfo
     * @brief Stores distance and parent information for Dijkstra's algorithm.
     */
    struct DistanceInfo {
        size_t distance;
        IPAddress parent;
        bool visited;

        DistanceInfo() : distance(INF), parent(IPAddress{}), visited(false) {}
    };

public:
    /**
     * @brief Computes shortest paths from a source router to all others.
     *
     * @param routers List of all routers in the network.
     * @param sourceIP IP of the source router.
     * @return Routing table for the source router.
     * @throws std::invalid_argument if sourceIP not found.
     */
    static RoutingTable computeRoutingTable(const List<const Router*>& routers, IPAddress sourceIP);

    /**
     * @brief Computes routing tables for all routers in the network.
     *
     * @param routers List of all routers.
     * @param tables Output list of routing tables (cleared first).
     */
    static void computeAllRoutingTables(const List<const Router*>& routers,
                                        List<RoutingTable>& tables);

private:
    /**
     * @brief Finds the unvisited router with minimum distance.
     *
     * @param distances Array of distance information.
     * @param routerCount Number of routers.
     * @return Index of minimum distance router, or -1 if none found.
     */
    static size_t findMinDistance(const List<DistanceInfo>& distances, size_t routerCount);

    /**
     * @brief Gets index of router in a list.
     *
     * @param routers List of routers.
     * @param routerIP IP to find.
     * @return Index, or -1 if not found.
     */
    static size_t getRouterIndex(const List<const Router*>& routers, IPAddress routerIP);
};
