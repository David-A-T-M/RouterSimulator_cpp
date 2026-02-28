#include "core/Network.h"

#include "core/Terminal.h"

Network::Network(const Config& config) : currentTick(1) {
    generateRandomNetwork(config.routerCount, config.maxTerminalCount, config.complexity,
                          config.trafficProbability, config.maxPageLen);
    recalculateAllRoutes();
}

void Network::generateRandomNetwork(uint8_t routerCount, uint8_t TerminalCount, size_t complexity,
                                    float probability, size_t pageLen) {
    for (size_t i = 0; i < routerCount; i++) {
        addRouter(i, TerminalCount, probability, pageLen);
    }

    for (auto& rtr : routers) {
        auto ips = rtr->getTerminalIPs();
        for (auto ip : ips) {
            addressBook.pushBack(ip);
        }
    }

    connectMinimal(routerCount);
    addAdditionalConnections(complexity);
}

void Network::connectMinimal(uint8_t routerCount) {
    for (size_t i = 1; i < routerCount; i++) {
        std::uniform_int_distribution<size_t> connectedDist(0, i - 1);
        establishLink(routers[i].get(), routers[connectedDist(m_rng)].get());
    }
}

void Network::addAdditionalConnections(size_t complexity) {
    if (complexity == 0 || routers.isEmpty())
        return;

    std::uniform_int_distribution<size_t> dist(0, routers.size() - 1);

    for (size_t i = 0; i < routers.size(); i++) {
        Router* source = routers[i].get();
        for (size_t c = 0; c < complexity; c++) {
            Router* target = routers[dist(m_rng)].get();
            establishLink(source, target);
        }
    }
}

void Network::establishLink(Router* rtrA, Router* rtrB) {
    if (rtrA == rtrB)
        return;
    rtrA->connectRouter(rtrB);
    rtrB->connectRouter(rtrA);
}

void Network::simulate(size_t ticks) {
    for (size_t i = 0; i < ticks; i++) {
        tick();
        if (i % 5 == 0) {
            recalculateAllRoutes();
        }
    }
    recalculateAllRoutes();
}

NetworkStats Network::getStats() const {
    NetworkStats stats;
    stats.currentTick = currentTick - 1;
    for (auto& rtr : routers) {
        stats.totalRouters++;
        stats.totalTerminals += rtr->getTerminalCount();
        stats.packetsDropped += rtr->getPacketsDropped();
        stats.packetsTimedOut += rtr->getPacketsTimedOut();
        stats.packetsInFlight += rtr->getPacketsInPending();
        stats.packetsInFlight += rtr->getPacketsOutPending();
        stats.packetsInFlight += rtr->getPacketsLocPending();
        for (auto& terminal : rtr->getTerminals()) {
            stats.pagesCreated += terminal->getPagesCreated();
            stats.pagesDropped += terminal->getPagesDropped();
            stats.pagesCompleted += terminal->getPagesCompleted();
            stats.pagesTimedOut += terminal->getPagesTimedOut();
            stats.packetsGenerated += terminal->getPacketsGenerated();
            stats.packetsSent += terminal->getPacketsSent();
            stats.packetsDropped += terminal->getPacketsInDropped();
            stats.packetsDropped += terminal->getPacketsOutDropped();
            stats.packetsTimedOut += terminal->getPacketsOutTimedOut();
            stats.packetsTimedOut += terminal->getPacketsInTimedOut();
            stats.packetsInFlight += terminal->getPacketsInPending();
            stats.packetsInFlight += terminal->getPacketsOutPending();
            stats.packetsDelivered += terminal->getPacketsSuccProcessed();
        }
    }
    return stats;
}

void Network::addRouter(uint8_t rtrID, uint8_t TerminalCount, float probability, size_t PageLen) {
    auto rtr = std::make_unique<Router>(IPAddress{rtrID}, TerminalCount);
    rtr->shareAddressBook(&addressBook);
    rtr->shareRandomGenerator(&m_rng);
    rtr->shareTrafficProbability(probability);
    rtr->shareMaxPageLength(PageLen);
    cRouters.pushBack(rtr.get());
    routers.pushBack(std::move(rtr));
}

void Network::recalculateAllRoutes() {
    for (auto& rtr : routers) {
        rtr->setRoutingTable(DijkstraAlgorithm::computeRoutingTable(cRouters, rtr->getIP()));
    }
}

void Network::tick() {
    for (auto& router : routers) {
        router->tick(currentTick);
    }
    currentTick++;
}
