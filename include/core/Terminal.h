#pragma once
#include "IPAddress.h"
#include "PacketBuffer.h"
#include "Page.h"
#include "PageReassembler.h"

class Router;  // Forward declaration

class Terminal {
    IPAddress ip;            /**< IP address of the terminal */
    Router* connectedRouter; /**< Pointer to the router connected to the terminal */

    PacketBuffer outputBuffer;          /**< Priority queue for outgoing packets */
    PacketBuffer inputBuffer;           /**< FIFO queue for incoming packets */
    List<PageReassembler> reassemblers; /**< Active page reassemblers (usar punteros para TTL) */

    size_t externalBW; /**< Packets per cycle to router (default 4) */
    size_t internalBW; /**< Packets per cycle from input buffer (default 8) */

    size_t sentPages;     /**< Total pages successfully sent */
    size_t receivedPages; /**< Total pages successfully received */
    size_t nextPageID;    /**< ID for the next page to be sent */
public:
    /**
     * @brief Constructor for Terminal.
     * @param ip Terminal's IP address.
     * @param router Pointer to connected router.
     * @param outputCapacity Output buffer capacity (default 50).
     * @param inputCapacity Input buffer capacity (default 100).
     * @param externalBW Packets per cycle to router (default 4).
     * @param internalBW Packets per cycle from input (default 8).
     */
    Terminal(IPAddress ip, Router* router, size_t outputCapacity = 0, size_t inputCapacity = 0, size_t externalBW = 4,
             size_t internalBW = 8);

    /**
     * @brief Destructor - cleans up all active reassemblers.
     */
    ~Terminal() = default;

    /**
     * @brief Copy constructor - deleted to prevent copying.
     */
    Terminal(const Terminal&) = delete;

    /**
     * @brief Copy assignment operator - deleted to prevent copying.
     */
    Terminal& operator=(const Terminal&) = delete;

    /**
     * @brief Move constructor - deleted to prevent moving.
     */
    Terminal(Terminal&&) = delete;

    /**
     * @brief Move assignment operator - deleted to prevent moving.
     */
    Terminal& operator=(Terminal&&) = delete;

    /**
     * @brief Creates a page and fragments it into packets for transmission.
     *
     * @param length Number of packets in the page (must be > 0).
     * @param destIP Destination IP address.
     * @return true if all packets were enqueued, false if some were dropped.
     */
    bool sendPage(size_t length, IPAddress destIP);

    /**
     * @brief Receives a packet from the network.
     *
     * @param packet The packet to receive.
     * @return true if packet was buffered, false if dropped (buffer full).
     */
    bool receivePacket(const Packet& packet);

    /**
     * @brief Main tick function - call once per simulation cycle.
     *
     * Executes in order:
     * 1. Process output buffer (send to router)
     * 2. Process input buffer (reassemble pages)
     * 3. Tick all reassemblers (decrement TTL) - Not yet implemented
     * 4. Cleanup expired reassemblers - Not yet implemented
     */
    void tick();

    /**
     * @brief Sends up to externalBW packets from output buffer to router.
     * @return Number of packets actually sent.
     */
    size_t processOutputBuffer();

    /**
     * @brief Processes up to internalBW packets from input buffer.
     * @return Number of packets processed.
     */
    size_t processInputBuffer();

    /**
     * @brief Sets external bandwidth (packets per cycle to router).
     * @param bw New bandwidth value.
     */
    void setExternalBW(size_t bw) noexcept;

    /**
     * @brief Sets internal bandwidth (packets per cycle from input).
     * @param bw New bandwidth value.
     */
    void setInternalBW(size_t bw) noexcept;

    /**
     * @brief Gets the number of pages received.
     * @return Number of pages received.
     */
    [[nodiscard]] size_t getReceivedPages() const noexcept;

    /**
     * @brief Gets the number of pages sent.
     * @return Number of pages sent.
     */
    [[nodiscard]] size_t getSentPages() const noexcept;

    /**
     * @brief Gets the terminal's IP address.
     * @return Terminal's IP address.
     */
    [[nodiscard]] IPAddress getTerminalIP() const noexcept;

    /**
     * @brief Gets the external bandwidth (packets per cycle to router).
     * @return External bandwidth value.
     */
    [[nodiscard]] size_t getExternalBW() const noexcept;

    /**
     * @brief Gets the internal bandwidth (packets per cycle from input).
     * @return Internal bandwidth value.
     */
    [[nodiscard]] size_t getInternalBW() const noexcept;

    /**
     * @brief Generates a string representation of the terminal, including its IP address.
     * @return A string representation of the terminal
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const Terminal& terminal);

private:
    // ===== Private helpers =====
    /**
     * @brief Finds existing reassembler or creates new one.
     * @return Pointer to reassembler, or nullptr on failure.
     */
    PageReassembler* findOrCreateReassembler(int pageID, int pageLength);

    /**
     * @brief Handles a completed page.
     */
    void handleCompletedPage(PageReassembler* reassembler);

    /**
     * @brief Removes specific reassembler from list.
     */
    void removeReassembler(const PageReassembler* reassembler);
};

inline size_t Terminal::getReceivedPages() const noexcept {
    return receivedPages;
}

inline size_t Terminal::getSentPages() const noexcept {
    return sentPages;
}

inline IPAddress Terminal::getTerminalIP() const noexcept {
    return ip;
}

inline size_t Terminal::getExternalBW() const noexcept {
    return externalBW;
}

inline size_t Terminal::getInternalBW() const noexcept {
    return internalBW;
}

inline void Terminal::setExternalBW(size_t bw) noexcept {
    externalBW = bw;
}

inline void Terminal::setInternalBW(size_t bw) noexcept {
    internalBW = bw;
}
