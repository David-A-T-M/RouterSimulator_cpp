#pragma once

#include "IPAddress.h"
#include "Packet.h"
#include "structures/list.h"

/**
 * @class Page
 * @brief Represents a page of data that can be fragmented into packets for transmission.
 *
 * A Page is the high-level unit of data in the network. It can be:
 * 1. Created at a terminal and fragmented into packets for transmission
 * 2. Reassembled from received packets at the destination
 */
class Page {
    size_t pageID;   /**< Unique identifier for the page */
    size_t pageLen;  /**< Page length in packets */
    IPAddress srcIP; /**< Source Terminal IP */
    IPAddress dstIP; /**< Destination Terminal IP */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a new page at origin terminal.
     * @param pageID Unique identifier for the page (must be >= 0).
     * @param pageLen The number of packets the page will contain (must be > 0).
     * @param srcIP The origin IP address for the page.
     * @param dstIP The destination IP address for the page.
     * @throws std::invalid_argument if parameters are invalid.
     */
    Page(size_t pageID, size_t pageLen, IPAddress srcIP, IPAddress dstIP);

    /**
     * @brief Constructor that creates a Page from a list of completed packets.
     *
     * This constructor is used when reassembling a page from received packets.
     * It takes ownership of the packets (move semantics).
     *
     * @param packets A list of packets that make up the completed page.
     * @throws std::invalid_argument if a packet list is empty or packets are inconsistent.
     * @note The packet list is moved and will be empty after this call.
     * @note The List<Packet> must contain all packets for the page, with correct positions.
     */
    explicit Page(List<Packet>&& packets);

    /**
     * @brief Default Copy Constructor
     */
    Page(const Page&) = default;

    /**
     * @brief Default Move Constructor
     */
    Page(Page&&) noexcept = default;

    /**
     * @brief Default Copy Assignment Operator
     * @return Reference to this page after assignment.
     */
    Page& operator=(const Page&) = default;

    /**
     * @brief Default Move Assignment Operator
     * @return Reference to this page after move assignment.
     */
    Page& operator=(Page&&) noexcept = default;

    /**
     * @brief Default destructor
     */
    ~Page() = default;

    // =============== Getters ===============
    /**
     * @brief Retrieves the page ID.
     * @return The unique identifier of the page.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Retrieves the length of the page in terms of the number of packets.
     * @return The number of packets in the page.
     */
    [[nodiscard]] size_t getPageLen() const noexcept;

    /**
     * @brief Retrieves the origin IP address of the page.
     * @return The origin IP address.
     */
    [[nodiscard]] IPAddress getSrcIP() const noexcept;

    /**
     * @brief Retrieves the destination IP address of the page.
     * @return The destination IP address.
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    // =============== Page Operations ===============
    /**
     * @brief Fragments this page into a list of packets ready for transmission.
     *
     * Creates a list of packets, where each packet contains a portion of the page data. The packets are numbered from 0
     * to pageLen-1 in their page position. Each packet includes the pageID, pageLen, srcIP, dstIP, and the expected
     * tick for delivery (expTick).
     *
     * @param expTick The tick of expiration for the packets.
     *
     * @return A list of packets representing this page.
     */
    [[nodiscard]] List<Packet> toPackets(size_t expTick) const;

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the page.
     * @return String describing the page.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const Page& page);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality comparison based on pageID.
     * @param other The page to compare against.
     * @return true if the page IDs are the same.
     */
    [[nodiscard]] bool operator==(const Page& other) const noexcept;

    /**
     * @brief Inequality comparison.
     * @param other The page to compare against.
     * @return true if the page IDs are different.
     */
    [[nodiscard]] bool operator!=(const Page& other) const noexcept;

    /**
     * @brief Less-than comparison (for sorting).
     * @param other The page to compare against.
     * @return true if this page's ID is less than the other page's ID.
     */
    [[nodiscard]] bool operator<(const Page& other) const noexcept;
};

inline size_t Page::getPageID() const noexcept {
    return pageID;
}

inline size_t Page::getPageLen() const noexcept {
    return pageLen;
}

inline IPAddress Page::getSrcIP() const noexcept {
    return srcIP;
}

inline IPAddress Page::getDstIP() const noexcept {
    return dstIP;
}

inline bool Page::operator==(const Page& other) const noexcept {
    return pageID == other.pageID;
}

inline bool Page::operator!=(const Page& other) const noexcept {
    return !(*this == other);
}

inline bool Page::operator<(const Page& other) const noexcept {
    return pageID < other.pageID;
}
