#pragma once

#include <memory>
#include "IPAddress.h"
#include "PacketBuffer.h"
#include "RoutingTable.h"
#include "structures/list.h"

// Forward declarations
class Terminal;
using TerminalPtr = std::unique_ptr<Terminal>;
class Router;

/**
 * @struct RouterConnection
 * @brief Represents a connection to a neighbor router.
 */
struct RouterConnection {
    Router* router;            /**< Pointer to neighbor router */
    PacketBuffer outputBuffer; /**< Output buffer for this neighbor */

    explicit RouterConnection(Router* r);
};

class Router {
    IPAddress ip;                       /**< Router's IP address */
    PacketBuffer inputBuffer;           /**< FIFO buffer for incoming packets */
    PacketBuffer localBuffer;           /**< Buffer for packets destined to local terminals */
    List<RouterConnection> connections; /**< Connections to neighbor routers */
    List<TerminalPtr> terminals;        /**< Connected terminals */
    RoutingTable routingTable;          /**< Routing table for packet forwarding */

    size_t internalBW; /**< Packets per cycle from input to output buffers */
    size_t externalBW; /**< Packets per cycle to each neighbor router */

    size_t packetsReceived; /**< Total packets received */
    size_t packetsDropped;  /**< Total packets dropped due to buffer overflow or no route */

public:
    /**
     * @brief Constructor for Router.
     * @param routerIP Router's IP address (must have terminalID = 0).
     * @param inputCapacity Input buffer capacity (default 200).
     * @param internalBW Internal bandwidth (default 10).
     * @param externalBW External bandwidth per connection (default 5).
     */
    explicit Router(IPAddress routerIP, size_t inputCapacity = 200, size_t internalBW = 10, size_t externalBW = 5);

    /**
     * @brief Destructor for Router.
     */
    ~Router() = default;

    /**
     * @brief Copy constructor - deleted to prevent copying.
     */
    Router(const Router&) = delete;

    /**
     * @brief Copy assignment operator - deleted to prevent copying.
     */
    Router& operator=(const Router&) = delete;

    /**
     * @brief Move constructor - deleted to prevent moving.
     */
    Router(Router&&) = delete;

    /**
     * @brief Move assignment operator - deleted to prevent moving.
     */
    Router& operator=(Router&&) = delete;

    // =============== Connection Management ===============
    /**
     * @brief Connects a terminal to this router.
     * @param terminal Pointer to terminal.
     * @throws std::invalid_argument if terminal is nullptr or wrong router.
     */
    void connectTerminal(TerminalPtr terminal);

    /**
     * @brief Connects to a neighbor router.
     *
     * Creates an output buffer for this connection.
     *
     * @param neighbor Pointer to neighbor router.
     * @throws std::invalid_argument if neighbor is nullptr or already connected.
     */
    void connectRouter(Router* neighbor);

    /**
     * @brief Disconnects from a neighbor router.
     * @param neighbor Pointer to neighbor router.
     * @return true if disconnected, false if not found.
     */
    bool disconnectRouter(const Router* neighbor);

    // =============== Packet Reception ===============
    /**
     * @brief Receives a packet from a terminal or neighbor router.
     *
     * Assigns priority and enqueues in input buffer.
     *
     * @param packet The packet to receive.
     * @return true if packet was buffered, false if dropped.
     */
    bool receivePacket(Packet packet);

    // =============== Processing ===============
    /**
     * @brief Main tick function - processes one simulation cycle.
     *
     * Executes in order:
     * 1. Process input buffer (route to output buffers)
     * 2. Process output buffers (send to neighbors)
     */
    void tick();

    /**
     * @brief Routes packets from input buffer to appropriate output buffers.
     * @return Number of packets routed.
     */
    size_t processInputBuffer();

    /**
     * @brief Sends packets from all output buffers to neighbor routers.
     * @return Total number of packets sent.
     */
    size_t processOutputBuffers();

    /**
     * @brief Processes packets destined for local terminals.
     * @return Number of packets delivered to local terminals.
     */
    size_t processLocalBuffer();

    // =============== Configuration ===============
    /**
     * @brief Sets internal bandwidth.
     */
    void setInternalBW(size_t bw) noexcept;

    /**
     * @brief Sets external bandwidth.
     */
    void setExternalBW(size_t bw) noexcept;

    /**
     * @brief Sets the routing table.
     */
    void setRoutingTable(RoutingTable table);

    // =============== Getters ===============
    /**
     * @brief Gets the router's IP address.
     */
    [[nodiscard]] IPAddress getIP() const noexcept;

    /**
     * @brief Gets the internal bandwidth.
     */
    [[nodiscard]] size_t getInternalBW() const noexcept;

    /**
     * @brief Gets the external bandwidth.
     */
    [[nodiscard]] size_t getExternalBW() const noexcept;

    /**
     * @brief Gets the total number of packets received.
     */
    [[nodiscard]] size_t getPacketsReceived() const noexcept;

    /**
     * @brief Gets the total number of packets dropped.
     */
    [[nodiscard]] size_t getPacketsDropped() const noexcept;

    /**
     * @brief Gets the number of connected terminals.
     */
    [[nodiscard]] size_t getConnectedTerminals() const noexcept;

    /**
     * @brief Gets the number of connected routers.
     */
    [[nodiscard]] size_t getConnectedRouters() const noexcept;

    /**
     * @brief Gets the size of the local buffer.
     */
    [[nodiscard]] size_t getLocalBufferUsage() const noexcept;

    /**
     * @brief Gets the size of the output buffer for a specific neighbor router.
     * @param neighborIP IP address of the neighbor router.
     * @return Size of the output buffer, or 0 if neighbor not found.
     */
    [[nodiscard]] size_t getNeighborBufferUsage(IPAddress neighborIP) const;

    // =============== Utilities ===============
    /**
     * @brief Gets string representation.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const Router& router);

private:
    // =============== Private Helpers ===============
    /**
     * @brief Assigns priority to a packet based on arrival order.
     */
    void assignPriority(Packet& packet) const;

    /**
     * @brief Gets a pointer to the output buffer for a specific neighbor router.
     * @param nextIP IP address of the neighbor router.
     * @return Pointer to the output buffer, or nullptr if neighbor not found.
     */
    PacketBuffer* getOutputBuffer(IPAddress nextIP);

    /**
     * @brief Routes a single packet to appropriate destination.
     * @return true if routed successfully, false if dropped.
     */
    bool routePacket(const Packet& packet);

    /**
     * @brief Checks if a router with given IP is already connected.
     * @param neighborIP IP address of the neighbor router.
     * @return true if connected, false otherwise.
     */
    [[nodiscard]] bool routerIsConnected(IPAddress neighborIP) const;

    /**
     * @brief Checks if a terminal with given IP is already connected.
     * @param terminalIP IP address of the terminal.
     * @return true if connected, false otherwise.
     */
    [[nodiscard]] bool terminalIsConnected(IPAddress terminalIP) const;
};

inline void Router::setRoutingTable(RoutingTable table) {
    routingTable = std::move(table);
}

inline IPAddress Router::getIP() const noexcept {
    return ip;
}

inline size_t Router::getInternalBW() const noexcept {
    return internalBW;
}

inline size_t Router::getExternalBW() const noexcept {
    return externalBW;
}

inline size_t Router::getPacketsReceived() const noexcept {
    return packetsReceived;
}

inline size_t Router::getPacketsDropped() const noexcept {
    return packetsDropped;
}

inline size_t Router::getConnectedTerminals() const noexcept {
    return terminals.size();
}

inline size_t Router::getConnectedRouters() const noexcept {
    return connections.size();
}

inline size_t Router::getLocalBufferUsage() const noexcept {
    return localBuffer.size();
}

inline void Router::setInternalBW(size_t bw) noexcept {
    internalBW = bw;
}

inline void Router::setExternalBW(size_t bw) noexcept {
    externalBW = bw;
}
