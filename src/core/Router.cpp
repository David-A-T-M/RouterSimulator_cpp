#include "core/Router.h"
#include "core/Terminal.h"

RouterConnection::RouterConnection(Router* r) {
    router       = r;
    outputBuffer = PacketBuffer{r->getIP()};
}

Router::Router(IPAddress routerIP, size_t inputCapacity, size_t internalBW, size_t externalBW)
    : ip(routerIP),
      inputBuffer(PacketBuffer::Mode::FIFO, inputCapacity),
      internalBW(internalBW),
      externalBW(externalBW),
      packetsReceived(0),
      packetsDropped(0) {
    if (!routerIP.isRouter()) {
        throw std::invalid_argument("Router IP must have terminalID = 0");
    }
}

void Router::connectTerminal(TerminalPtr terminal) {
    if (!terminal) {
        throw std::invalid_argument("Terminal cannot be nullptr");
    }

    if (terminalIsConnected(terminal->getTerminalIP())) {
        throw std::invalid_argument("Terminal already connected");
    }

    if (terminal->getTerminalIP().getRouterIP() != getIP().getRouterIP()) {
        throw std::invalid_argument("Terminal does not belong to this router");
    }

    terminals.pushBack(std::move(terminal));
}

void Router::connectRouter(Router* neighbor) {
    if (!neighbor) {
        throw std::invalid_argument("Neighbor router cannot be nullptr");
    }

    if (routerIsConnected(neighbor->getIP())) {
        throw std::invalid_argument("Router already connected");
    }

    connections.pushBack(RouterConnection(neighbor));
}

bool Router::disconnectRouter(const Router* neighbor) {
    if (!neighbor) {
        return false;
    }

    const IPAddress neighborIP = neighbor->getIP();

    for (int i = 0; i < connections.size(); ++i) {
        if (connections[i].router->getIP() == neighborIP) {
            connections.removeAt(i);
            return true;
        }
    }

    return false;
}

bool Router::receivePacket(Packet packet) {
    assignPriority(packet);

    if (inputBuffer.enqueue(packet)) {
        packetsReceived++;
        return true;
    }
    packetsDropped++;
    return false;
}

void Router::tick() {
    processInputBuffer();
    processOutputBuffers();
    processLocalBuffer();
}

size_t Router::processInputBuffer() {
    size_t processed = 0;

    while (processed < internalBW && !inputBuffer.isEmpty()) {
        Packet packet = inputBuffer.dequeue();

        routePacket(packet);
        processed++;
    }
    return processed;
}

size_t Router::processOutputBuffers() {
    size_t totalSent = 0;

    for (auto& conn : connections) {
        size_t sent = 0;

        while (sent < externalBW && !conn.outputBuffer.isEmpty()) {
            Packet packet = conn.outputBuffer.dequeue();

            if (conn.router->receivePacket(packet)) {
                sent++;
            } else {
                packetsDropped++;
            }
        }

        totalSent += sent;
    }

    return totalSent;
}

size_t Router::processLocalBuffer() {
    size_t delivered = 0;

    while (delivered < internalBW && !localBuffer.isEmpty()) {
        bool found = false;

        Packet packet = localBuffer.dequeue();
        for (auto& terminal : terminals) {
            if (terminal->getTerminalIP() == packet.getDestinationIP()) {
                terminal->receivePacket(packet);
                delivered++;
                found = true;
                break;
            }
        }
        if (!found) {
            packetsDropped++;
        }
    }
    return delivered;
}

size_t Router::getNeighborBufferUsage(IPAddress neighborIP) const {
    for (const auto& conn : connections) {
        if (conn.router->getIP() == neighborIP) {
            return conn.outputBuffer.size();
        }
    }
    return 0;
}

std::string Router::toString() const {
    std::ostringstream oss;
    oss << "Router{IP=" << ip << ", InternalBW=" << internalBW << ", ExternalBW=" << externalBW
        << ", PacketsReceived=" << packetsReceived << ", ConnectedTerminals=" << terminals.size()
        << ", ConnectedRouters=" << connections.size() << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Router& router) {
    os << router.toString();
    return os;
}

void Router::assignPriority(Packet& packet) const {
    packet.setRouterPriority(packetsReceived % 1000000);
}

PacketBuffer* Router::getOutputBuffer(IPAddress nextIP) {
    for (auto& conn : connections) {
        if (conn.router->getIP() == nextIP) {
            return &conn.outputBuffer;
        }
    }
    return nullptr;
}

bool Router::routePacket(Packet packet) {
    IPAddress destIP = packet.getDestinationIP();

    if (destIP.getRouterIP() == getIP().getRouterIP()) {
        if (localBuffer.enqueue(packet)) {
            return true;
        }
        packetsDropped++;
        return false;
    }

    IPAddress nextHopIP     = routingTable.getNextHopIP(destIP);
    PacketBuffer* outBuffer = getOutputBuffer(nextHopIP);

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

bool Router::routerIsConnected(IPAddress neighborIP) const {
    for (const auto& conn : connections) {
        if (conn.router->getIP() == neighborIP) {
            return true;
        }
    }
    return false;
}
bool Router::terminalIsConnected(IPAddress terminalIP) const {
    for (const auto& terminal : terminals) {
        if (terminal->getTerminalIP() == terminalIP) {
            return true;
        }
    }
    return false;
}
