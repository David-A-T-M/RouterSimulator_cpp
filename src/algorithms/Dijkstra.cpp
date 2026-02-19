#include "algorithms/Dijkstra.h"

#include <algorithm>

#include "core/Router.h"

RoutingTable DijkstraAlgorithm::computeRoutingTable(const List<const Router*>& routers, IPAddress sourceIP) {
    const size_t routerCount = routers.size();
    // Initialize distances, visited and parents
    List<DistanceInfo> distances;
    for (size_t i = 0; i < routerCount; ++i) {
        distances.pushBack(DistanceInfo{});
    }

    // Get index of source router
    const size_t sourceIndex = getRouterIndex(routers, sourceIP);

    // Set source router distance to 0 and parent to itself
    distances[sourceIndex].distance = 0;
    distances[sourceIndex].parent   = sourceIP;

    // Dijkstra's algorithm main loop
    for (size_t i = 0; i < routerCount; ++i) {
        // Find unvisited router with minimum distance
        const size_t currentIndex = findMinDistance(distances, routerCount);

        if (currentIndex == -1) {
            break;  // All reachable routers visited, exit main loop
        }

        distances[currentIndex].visited = true;  // Mark current router as visited

        // Get connections of the current router
        auto conn = routers[currentIndex]->getConnections();
        // Update distances for neighbors
        for (const auto& connection : conn) {
            const IPAddress neighborIP = connection.router->getIP();
            const size_t neighborIndex = getRouterIndex(routers, neighborIP);

            if (distances[neighborIndex].visited) {
                continue;  // Skip already visited neighbors
            }

            const size_t newDist = distances[currentIndex].distance + connection.outputBuffer.size();
            // If a shorter path through current router is found, update distance and parent
            if (newDist < distances[neighborIndex].distance) {
                distances[neighborIndex].distance = newDist;
                distances[neighborIndex].parent   = routers[currentIndex]->getIP();
            }
        }
    }

    // Build routing table from distances
    RoutingTable routingTable{};

    for (size_t i = 0; i < routerCount; ++i) {
        // Ignore the source router itself and unreachable routers
        if (i == sourceIndex || distances[i].distance == INF) {
            continue;
        }

        // Get the destination router IP
        const IPAddress destIP = routers[i]->getIP();

        // Trace back from destination to source to find the next hop
        IPAddress currentIP = destIP;
        IPAddress parentIP  = distances[i].parent;

        while (parentIP != sourceIP) {
            currentIP = parentIP;

            // Get index of the parent router to continue tracing back
            const size_t pIdx = getRouterIndex(routers, parentIP);
            parentIP          = distances[pIdx].parent;
        }

        // Set the next hop for this destination in the routing table
        routingTable.setNextHopIP(destIP, currentIP);
    }

    return routingTable;
}

void DijkstraAlgorithm::computeAllRoutingTables(const List<const Router*>& routers, List<RoutingTable>& tables) {
    tables.clear();

    for (const auto* router : routers) {
        RoutingTable table = computeRoutingTable(routers, router->getIP());

        tables.pushBack(table);
    }
}

size_t DijkstraAlgorithm::getRouterIndex(const List<const Router*>& routers, IPAddress routerIP) {
    const auto it =
        std::find_if(routers.begin(), routers.end(), [&routerIP](const auto& r) { return r->getIP() == routerIP; });

    if (it == routers.end()) {
        throw std::runtime_error("No such router");
    }

    return std::distance(routers.begin(), it);
}

size_t DijkstraAlgorithm::findMinDistance(const List<DistanceInfo>& distances, size_t routerCount) {
    size_t minDistance  = INF;
    size_t currentIndex = -1;
    for (size_t j = 0; j < routerCount; j++) {
        if (!distances[j].visited && distances[j].distance < minDistance) {
            minDistance  = distances[j].distance;
            currentIndex = j;
        }
    }
    return currentIndex;
}
