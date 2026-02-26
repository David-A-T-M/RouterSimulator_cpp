#pragma once

#include <memory>
#include <unordered_map>

#include "IPAddress.h"
#include "PacketBuffer.h"
#include "RoutingTable.h"

// Forward declarations
class Terminal;
class Router;

/**
 * @class Router
 * @brief Represents a network router that manages connections,
 * packet routing, and bandwidth allocation for a network simulation.
 */
class Router {
public:
    static constexpr size_t DEF_INPUT_PROC  = 10; /**< Default input processing rate */
    static constexpr size_t DEF_OUTPUT_BW   = 5;  /**< Default output bandwidth */
    static constexpr size_t DEF_LOC_BW      = 10; /**< Default local bandwidth */
    static constexpr size_t DEF_OUT_BUF_CAP = 0;  /**< Default output buffer capacity */
    static constexpr size_t DEF_IN_BUF_CAP  = 0;  /**< Default input buffer capacity */
    static constexpr size_t DEF_LOC_BUF_CAP = 0;  /**< Default local buffer capacity */

    /**
     * @struct Config
     * @brief Represents configuration settings for buffering and processing in a system.
     *
     * This structure is used to define the capacity of input and output buffers, as well
     * as the bandwidth and processing constraints.
     */
    struct Config {
        size_t inBufferCap; /**< Input buffer capacity for the router */
        size_t inProcCap;   /**< Input processing rate, packets that can be processed from the input
                               buffer per cycle */
        size_t locBufferCap; /**< Local buffer capacity for packets destined to local terminals */
        size_t locBW;        /**< Local bandwidth, number of packets that can be sent to each local
                                terminal per cycle */
        size_t outBufferCap; /**< Output buffer capacity for the router */
        size_t outBW; /**< Output bandwidth, number of packets that can be sent to the router per
                         cycle */

        /**
         * @brief Default constructor for Config, initializes with default values.
         */
        Config()
            : inBufferCap(DEF_IN_BUF_CAP),
              inProcCap(DEF_INPUT_PROC),
              locBufferCap(DEF_LOC_BUF_CAP),
              locBW(DEF_LOC_BW),
              outBufferCap(DEF_OUT_BUF_CAP),
              outBW(DEF_OUTPUT_BW) {}

        Config(size_t inBufferCap, size_t inProcCap, size_t locBufferCap, size_t locBW,
               size_t outBufferCap, size_t outBW)
            : inBufferCap(inBufferCap),
              inProcCap(inProcCap),
              locBufferCap(locBufferCap),
              locBW(locBW),
              outBufferCap(outBufferCap),
              outBW(outBW) {}
    };

private:
    /**
     * @struct RtrConnection
     * @brief Represents a connection to a neighbor router.
     */
    struct RtrConnection {
        Router* neighborRouter; /**< Pointer to neighbor router */
        PacketBuffer outBuffer; /**< Output buffer for this neighbor */

        /**
         * @brief Constructor for RouterConnection.
         * @param r Pointer to the neighbor router (must not be nullptr).
         * @param capacity Capacity of the output buffer for this connection (default 0).
         */
        explicit RtrConnection(Router* r, size_t capacity = 0)
            : neighborRouter(r), outBuffer(PacketBuffer{r->getIP(), capacity}) {}

        /**
         * @brief Move constructor - defaulted to allow moving of RtrConnection objects without
         * copying.
         */
        RtrConnection(RtrConnection&&) noexcept = default;

        /**
         * @brief Move assignment operator - defaulted to allow moving of RtrConnection objects
         * without copying.
         * @return Reference to this object after the move assignment.
         */
        RtrConnection& operator=(RtrConnection&&) noexcept = default;
    };

    /**< Unique pointer type for managing connected terminals */
    using TerminalPtr  = std::unique_ptr<Terminal>;
    /**< Unordered map of connected terminals, keyed by their IP address for efficient lookup */
    using TerminalList = std::unordered_map<IPAddress, TerminalPtr>;
    /**< List of connections to neighbor routers, each containing a pointer to the neighbor and its
     * output buffer */
    using RoutersList  = std::unordered_map<IPAddress, RtrConnection>;

    IPAddress routerIP;        /**< Router's IP address */
    RoutingTable routingTable; /**< Routing table for packet forwarding */
    TerminalList terminals;    /**< Connected terminals */
    RoutersList connections;   /**< Connections to neighbor routers */
    size_t outBufferCap;       /**< Capacity of output buffers */

    PacketBuffer inBuffer;  /**< FIFO buffer for incoming packets */
    size_t inProcCap;       /**< Packets per cycle able to process from the input buffer */
    PacketBuffer locBuffer; /**< Buffer for packets destined to local terminals */
    size_t locBufferBW;     /**< Packets per cycle to each local terminal */
    size_t outBufferBW;     /**< Packets per cycle to each neighbor router */

    size_t packetsReceived;  /**< Total packets received */
    size_t packetsDropped;   /**< Total packets dropped due to buffer overflow or no route */
    size_t packetsTimedOut;  /**< Total packets dropped due to expiration while in the buffers */
    size_t packetsForwarded; /**< Total packets forwarded */
    size_t packetsDelivered; /**< Total packets delivered to local terminals */

public:
    /**
     * @brief Constructor for Router.
     * @param ip Router's IP address (must have terminalID = 0).
     * @param terminals Number of terminals to initialize (default 0).
     * @param cfg Configuration struct for bandwidth and buffer settings (optional).
     */
    explicit Router(IPAddress ip, size_t terminals = 0, const Config& cfg = Config{});

    /**
     * @brief Default constructor - deleted
     */
    Router() = delete;

    /**
     * @brief Destructor for Router.
     */
    ~Router();

    /**
     * @brief Copy constructor - deleted to prevent copying.
     */
    Router(const Router&) = delete;

    /**
     * @brief Copy assignment operator - deleted to prevent copying.
     * @return Reference to this object.
     */
    Router& operator=(const Router&) = delete;

    /**
     * @brief Move constructor - deleted to prevent moving.
     */
    Router(Router&&) = delete;

    /**
     * @brief Move assignment operator - deleted to prevent moving.
     * @return Reference to this object.
     */
    Router& operator=(Router&&) = delete;

    // =============== Connection Management ===============
    /**
     * @brief Connects a terminal to this router.
     * @param terminal Unique pointer to the terminal to connect (must not be nullptr).
     * @return true if connected successfully, false otherwise.
     * @throws std::invalid_argument if the terminal is nullptr or wrong router.
     */
    bool connectTerminal(TerminalPtr terminal);

    /**
     * @brief Connects a neighbor router to this router.
     * @param neighbor Pointer to the neighbor router to connect (must not be nullptr).
     * @return true if connected successfully, false if the neighbor is nullptr or already
     * connected.
     * @throws std::invalid_argument if the neighbor router is nullptr.
     */
    bool connectRouter(Router* neighbor);

    // =============== Transmission ===============
    /**
     * @brief Receives a packet from the network and enqueues it in the input buffer.
     * @param packet The packet to receive.
     * @return true if the packet was buffered successfully, false if dropped due to input buffer
     * overflow.
     */
    bool receivePacket(const Packet& packet);

    // =============== Processing ===============
    /**
     * @brief Processes packets in output buffers for neighbor routers, sending them out and
     * updating statistics.
     * @param currentTick The current system tick for processing timeouts and expirations.
     * @return Total number of packets sent to neighbor routers.
     */
    size_t processOutputBuffers(size_t currentTick);

    /**
     * @brief Processes packets in the local buffer, delivering them to connected terminals and
     * updating statistics.
     * @param currentTick The current system tick for processing timeouts and expirations.
     * @return Total number of packets delivered to local terminals.
     */
    size_t processLocalBuffer(size_t currentTick);

    /**
     * @brief Advances the state of connected terminals by one tick, allowing them to process their
     * input and output buffers and update their internal state.
     * @param currentTick The current system tick for processing timeouts and expirations.
     */
    void tickTerminals(size_t currentTick);

    /**
     * @brief Processes packets in the input buffer, routing them to the appropriate output buffers
     * or local buffer based on the routing table, and updating statistics.
     * @param currentTick The current system tick for processing timeouts and expirations.
     * @return Total number of packets processed from the input buffer.
     */
    size_t processInputBuffer(size_t currentTick);

    /**
     * @brief Advances the state of the router by one tick, processing output buffers, local buffer,
     * terminals, and input buffer in the correct order to simulate a full cycle of operation.
     * @param currentTick The current system tick for processing timeouts and expirations.
     */
    void tick(size_t currentTick);

    // =============== Configuration ===============
    /**
     * @brief Sets the input processing capacity of the router.
     * @param proCap New processing capacity (packets per cycle).
     */
    void setInProcCap(size_t proCap) noexcept;

    /**
     * @brief Sets the local buffer bandwidth of the router.
     * @param bw New local buffer bandwidth (packets per cycle).
     */
    void setLocBufferBW(size_t bw) noexcept;

    /**
     * @brief Sets the output buffer bandwidth of the router.
     * @param bw New output buffer bandwidth (packets per cycle).
     */
    void setOutBufferBW(size_t bw) noexcept;

    /**
     * @brief Sets the routing table for the router.
     * @param table New routing table to use for packet forwarding.
     */
    void setRoutingTable(RoutingTable&& table) noexcept;

    // =============== Getters ===============

    /**
     * @brief Gets the IP address of the router.
     * @return Router's IP address.
     */
    [[nodiscard]] IPAddress getIP() const noexcept;

    /**
     * @brief Gets the number of terminals currently connected to this router.
     * @return Number of connected terminals.
     */
    [[nodiscard]] size_t getTerminalCount() const noexcept;

    /**
     * @brief Gets the number of neighbor routers currently connected to this router.
     * @return Number of connected neighbor routers.
     */
    [[nodiscard]] size_t getRouterCount() const noexcept;

    /**
     * @brief Gets the input processing capacity of the router.
     * @return Input processing capacity (packets per cycle).
     */
    [[nodiscard]] size_t getInProcCap() const noexcept;

    /**
     * @brief Gets the local buffer bandwidth of the router.
     * @return Local buffer bandwidth (packets per cycle).
     */
    [[nodiscard]] size_t getLocBufferBW() const noexcept;

    /**
     * @brief Gets the output buffer bandwidth of the router.
     * @return Output buffer bandwidth (packets per cycle).
     */
    [[nodiscard]] size_t getOutBufferBW() const noexcept;

    /**
     * @brief Gets the total number of packets that have been received by the router.
     * @return Total packets received.
     */
    [[nodiscard]] size_t getPacketsReceived() const noexcept;

    /**
     * @brief Gets the total number of packets that have been dropped due to buffer overflow or no
     * route.
     * @return Total packets dropped.
     */
    [[nodiscard]] size_t getPacketsDropped() const noexcept;

    /**
     * @brief Gets the total number of packets that have timed out while in the router's buffers.
     * @return Total packets that have expired and been dropped due to timeout.
     */
    [[nodiscard]] size_t getPacketsTimedOut() const noexcept;

    /**
     * @brief Gets the total number of packets forwarded to neighbor routers.
     * @return Total packets forwarded to neighbor routers.
     */
    [[nodiscard]] size_t getPacketsForwarded() const noexcept;

    /**
     * @brief Gets the total number of packets delivered to local terminals.
     * @return Total packets delivered to local terminals.
     */
    [[nodiscard]] size_t getPacketsDelivered() const noexcept;
    [[nodiscard]] size_t getPacketsInPending() const noexcept;
    [[nodiscard]] size_t getPacketsOutPending() const noexcept;
    [[nodiscard]] size_t getPacketsLocPending() const noexcept;

    /**
     * @brief Gets the current usage of the local buffer (number of packets currently in the local
     * buffer).
     * @return Number of packets currently in the local buffer.
     */
    [[nodiscard]] size_t getLocalBufferUsage() const noexcept;

    /**
     * @brief Gets the size of the output buffer for a specific neighbor router.
     * @param neighborIP IP address of the neighbor router.
     * @return Size of the output buffer, or 0 if neighbor not found.
     */
    [[nodiscard]] size_t getNeighborBufferUsage(IPAddress neighborIP) const;

    /**
     * @brief Gets a pointer to a connected terminal by its IP address.
     * @param ip IP address of the terminal to retrieve.
     * @return Pointer to the Terminal if found, or nullptr if no terminal with the given IP is
     * connected.
     */
    [[nodiscard]] const Terminal* getTerminal(IPAddress ip) const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Gets string representation.
     * @return A string representation of the router, including its IP address and number of
     * connected terminals and routers.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const Router& router);

private:
    // =============== Private Helpers ===============
    /**
     * @brief Initializes a specified number of terminals with sequential terminal IDs. The
     * terminals are added to the list of terminals.
     * @param count Number of terminals to initialize.
     */
    void initializeTerminals(size_t count);

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
     * @brief Checks if a router with a given IP is already connected.
     * @param neighborIP IP address of the neighbor router.
     * @return true if connected, false otherwise.
     */
    [[nodiscard]] bool routerIsConnected(const IPAddress& neighborIP) const;

    /**
     * @brief Checks if a terminal with a given IP is already connected.
     * @param terminalIP IP address of the terminal.
     * @return true if connected, false otherwise.
     */
    [[nodiscard]] bool terminalIsConnected(const IPAddress& terminalIP) const;
};

inline void Router::setInProcCap(size_t proCap) noexcept {
    inProcCap = proCap;
}

inline void Router::setLocBufferBW(size_t bw) noexcept {
    locBufferBW = bw;
}

inline void Router::setOutBufferBW(size_t bw) noexcept {
    outBufferBW = bw;
}

inline void Router::setRoutingTable(RoutingTable&& table) noexcept {
    routingTable = table;
}

inline IPAddress Router::getIP() const noexcept {
    return routerIP;
}

inline size_t Router::getTerminalCount() const noexcept {
    return terminals.size();
}

inline size_t Router::getRouterCount() const noexcept {
    return connections.size();
}

inline size_t Router::getInProcCap() const noexcept {
    return inProcCap;
}

inline size_t Router::getLocBufferBW() const noexcept {
    return locBufferBW;
}

inline size_t Router::getOutBufferBW() const noexcept {
    return outBufferBW;
}

inline size_t Router::getPacketsReceived() const noexcept {
    return packetsReceived;
}

inline size_t Router::getPacketsDropped() const noexcept {
    return packetsDropped;
}

inline size_t Router::getPacketsTimedOut() const noexcept {
    return packetsTimedOut;
}

inline size_t Router::getPacketsForwarded() const noexcept {
    return packetsForwarded;
}

inline size_t Router::getPacketsDelivered() const noexcept {
    return packetsDelivered;
}

inline size_t Router::getPacketsInPending() const noexcept {
    return inBuffer.size();
}

inline size_t Router::getPacketsLocPending() const noexcept {
    return locBuffer.size();
}


inline size_t Router::getLocalBufferUsage() const noexcept {
    return locBuffer.size();
}
