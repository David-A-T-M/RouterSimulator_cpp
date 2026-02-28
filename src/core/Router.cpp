#include <algorithm>
#include <ranges>

#include "core/Router.h"
#include "core/Terminal.h"

Router::Router(IPAddress ip, size_t terminals, const Config& cfg)
    : routerIP(ip),
      outBufferCap(cfg.outBufferCap),
      inBuffer(cfg.inBufferCap),
      inProcCap(cfg.inProcCap),
      locBuffer(cfg.locBufferCap),
      locBufferBW(cfg.locBW),
      outBufferBW(cfg.outBW),
      packetsReceived(0),
      packetsDropped(0),
      packetsTimedOut(0),
      packetsForwarded(0),
      packetsDelivered(0) {
    if (!routerIP.isRouter()) {
        throw std::invalid_argument("Router IP must have terminalID = 0");
    }
    initializeTerminals(terminals);
}

Router::~Router() = default;

bool Router::connectTerminal(TerminalPtr terminal) {
    if (!terminal) {
        throw std::invalid_argument("Terminal cannot be nullptr");
    }

    if (terminalIsConnected(terminal->getTerminalIP())) {
        throw std::invalid_argument("Terminal already connected");
    }

    if (terminal->getTerminalIP().getRouterIP() != getIP().getRouterIP()) {
        throw std::invalid_argument("Terminal does not belong to this router");
    }

    const IPAddress terminalIP = terminal->getTerminalIP();
    terminals[terminalIP]      = std::move(terminal);

    return true;
}

bool Router::connectRouter(Router* neighbor) {
    if (!neighbor) {
        throw std::invalid_argument("Neighbor router cannot be nullptr");
    }

    if (neighbor == this) {
        return false;
    }

    auto [it, inserted] =
        connections.try_emplace(neighbor->getIP(), RtrConnection(neighbor, outBufferCap));
    return inserted;
}

bool Router::receivePacket(const Packet& packet) {
    packetsReceived++;

    if (!inBuffer.enqueue(packet)) {
        packetsDropped++;
        return false;
    }

    return true;
}

size_t Router::processOutputBuffers(size_t currentTick) {
    size_t totalSent = 0;

    for (auto& [rtr, buff] : connections | std::views::values) {
        size_t sent = 0;
        while (sent < outBufferBW && !buff.isEmpty()) {
            Packet packet = buff.dequeue();

            if (packet.getTimeout() <= currentTick) {
                packetsTimedOut++;
                continue;
            }

            if (rtr) {
                rtr->receivePacket(packet);
                sent++;
                packetsForwarded++;
            } else {
                packetsDropped++;
            }
        }

        totalSent += sent;
    }

    return totalSent;
}

size_t Router::processLocalBuffer(size_t currentTick) {
    size_t delivered = 0;

    while (delivered < locBufferBW && !locBuffer.isEmpty()) {
        Packet packet = locBuffer.dequeue();

        if (packet.getTimeout() <= currentTick) {
            packetsTimedOut++;
            continue;
        }

        auto it = terminals.find(packet.getDstIP());

        if (it != terminals.end()) {
            it->second->receivePacket(packet);
            packetsDelivered++;
            delivered++;
        } else {
            packetsDropped++;
        }
    }
    return delivered;
}

void Router::tickTerminals(size_t currentTick) {
    for (auto& terminal : terminals | std::views::values) {
        if (terminal) {
            terminal->tick(currentTick);
        }
    }
}

size_t Router::processInputBuffer(size_t currentTick) {
    size_t processed = 0;

    while (processed < inProcCap && !inBuffer.isEmpty()) {
        processed++;
        Packet packet = inBuffer.dequeue();

        if (packet.getTimeout() <= currentTick) {
            packetsTimedOut++;
            continue;
        }

        routePacket(packet);
    }
    return processed;
}

void Router::tick(size_t currentTick) {
    processOutputBuffers(currentTick);
    processLocalBuffer(currentTick);
    tickTerminals(currentTick);
    processInputBuffer(currentTick);
}

size_t Router::getPacketsOutPending() const noexcept {
    return std::accumulate(connections.begin(), connections.end(), size_t{0},
                           [](size_t acc, const auto& conn) {
                               const auto& [rtr, rtrConn] = conn;
                               return acc + rtrConn.outBuffer.size();
                           });
}

size_t Router::getNeighborBufferUsage(IPAddress neighborIP) const {
    if (const auto it = connections.find(neighborIP); it != connections.end()) {
        return it->second.outBuffer.size();
    }

    return 0;
}

const Terminal* Router::getTerminal(IPAddress ip) const noexcept {
    if (const auto it = terminals.find(ip); it != terminals.end()) {
        return it->second.get();
    }
    return nullptr;
}

List<const Terminal*> Router::getTerminals() const noexcept {
    List<const Terminal*> list;
    for (const auto& terminal : terminals | std::views::values) {
        list.pushBack(terminal.get());
    }
    return list;
}

List<IPAddress> Router::getNeighborIPs() const {
    List<IPAddress> ips;
    for (const auto& ip : connections | std::views::keys) {
        ips.pushBack(ip);
    }
    return ips;
}

List<IPAddress> Router::getTerminalIPs() const {
    List<IPAddress> ips;
    for (const auto& ip : terminals | std::views::keys) {
        ips.pushBack(ip);
    }
    return ips;
}

void Router::shareAddressBook(List<IPAddress>* terminalIPs) {
    for (const auto& ip : terminals | std::views::values) {
        ip->setAddressBook(terminalIPs);
    }
}

void Router::shareRandomGenerator(std::mt19937* r_gen) {
    for (const auto& ip : terminals | std::views::values) {
        ip->setRandomGenerator(r_gen);
    }
}

void Router::shareTrafficProbability(float probability) {
    for (const auto& ip : terminals | std::views::values) {
        ip->setTrafficProbability(probability);
    }
}

void Router::shareMaxPageLength(size_t pageLen) {
    for (const auto& ip : terminals | std::views::values) {
        ip->setMaxPageLength(pageLen);
    }
}

std::string Router::toString() const {
    std::ostringstream oss;
    oss << "Router{IP: " << routerIP << " | ConnectedTerminals: " << terminals.size()
        << " | ConnectedRouters: " << connections.size() << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Router& router) {
    os << router.toString();
    return os;
}

void Router::initializeTerminals(size_t count) {
    for (size_t i = 1; i <= count; ++i) {
        auto terminal = std::make_unique<Terminal>(this, i);
        IPAddress terminalIP(routerIP.getRouterIP(), i);
        terminals[terminalIP] = std::move(terminal);
    }
}

PacketBuffer* Router::getOutputBuffer(IPAddress nextIP) {
    if (const auto it = connections.find(nextIP); it != connections.end()) {
        return &(it->second.outBuffer);
    }
    return nullptr;
}

bool Router::routePacket(const Packet& packet) {
    const IPAddress destIP = packet.getDstIP();

    if (destIP.getRouterIP() == routerIP.getRouterIP()) {
        if (locBuffer.enqueue(packet)) {
            return true;
        }
        packetsDropped++;
        return false;
    }

    const IPAddress nextHopIP = routingTable.getNextHopIP(destIP);
    PacketBuffer* outBuffer   = getOutputBuffer(nextHopIP);

    if (!outBuffer) {
        packetsDropped++;
        return false;
    }

    if (outBuffer->enqueue(packet)) {
        return true;
    }
    packetsDropped++;
    return false;
}

bool Router::routerIsConnected(const IPAddress& neighborIP) const {
    return connections.contains(neighborIP);
}

bool Router::terminalIsConnected(const IPAddress& terminalIP) const {
    return terminals.contains(terminalIP);
}
