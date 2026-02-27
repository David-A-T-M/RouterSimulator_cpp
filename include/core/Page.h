#pragma once

#include "IPAddress.h"
#include "Packet.h"
#include "structures/list.h"

/**
 * @class Page
 * @brief Represents a page of data to be transmitted in the network, consisting of multiple
 * packets.
 *
 * A Page is created at the origin terminal with a unique pageID, a specified length in packets, and
 * source/destination IPs. The Page can be fragmented into packets for transmission and can also be
 * reassembled from a list of received packets. The class provides methods for creating packets from
 * the page, as well as utilities for comparison and string representation.
 */
class Page {
    size_t pageID;   /**< Unique identifier for the page (must be >= 0) */
    size_t pageLen;  /**< Number of packets the page will contain (must be > 0) */
    IPAddress srcIP; /**< Source Terminal IP */
    IPAddress dstIP; /**< Destination Terminal IP */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a Page with specified parameters.
     *
     * @param pageID Unique identifier for the page (must be >= 0).
     * @param pageLen The number of packets the page will contain (must be > 0).
     * @param srcIP The source IP address for the page.
     * @param dstIP The destination IP address for the page.
     * @throws std::invalid_argument if pageID is negative, pageLen is not positive, or if srcIP or
     * dstIP are invalid.
     */
    Page(size_t pageID, size_t pageLen, IPAddress srcIP, IPAddress dstIP);

    /**
     * @brief Constructor for creating a Page from a list of packets that have been received and
     * reassembled.
     *
     * @param packets List of packets that belong to the same page. The constructor will validate
     * that all packets have the same pageID, pageLen, srcIP, and dstIP, and that their page
     * positions are consistent with their order in the list.
     * @throws std::invalid_argument if the packet list is empty, if packets have inconsistent
     * pageID, pageLen, srcIP, dstIP, or if page positions are incorrect.
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
     *
     * @return Reference to this page after assignment.
     */
    Page& operator=(const Page&) = default;

    /**
     * @brief Default Move Assignment Operator
     *
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
     *
     * @return The unique identifier of the page.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Retrieves the length of the page in terms of the number of packets.
     *
     * @return The number of packets in the page.
     */
    [[nodiscard]] size_t getPageLen() const noexcept;

    /**
     * @brief Retrieves the source IP address of the page.
     *
     * @return The source IP address.
     */
    [[nodiscard]] IPAddress getSrcIP() const noexcept;

    /**
     * @brief Retrieves the destination IP address of the page.
     *
     * @return The destination IP address.
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    // =============== Page Operations ===============
    /**
     * @brief Generates a list of packets that represent this page, with the specified expiration
     * tick. Each packet will have the same pageID, pageLen, srcIP, and dstIP as the page, and will
     * have a page position corresponding to its order in the list (starting from 0).
     *
     * @param expTick The is the system tick at which the packets should be considered expired and
     * dropped if not delivered.
     * @return List of packets that represent this page, ready for transmission. The list will
     * contain pageLen packets, each with the correct page position and expiration tick.
     */
    [[nodiscard]] List<Packet> toPackets(size_t expTick) const;

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the page.
     *
     * @return String describing the page.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     *
     * @param os Output stream to write to.
     * @param page The page to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Page& page);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality comparison operator.
     *
     * @param other The page to compare against.
     * @return true if the page IDs are the same, false otherwise.
     */
    [[nodiscard]] bool operator==(const Page& other) const noexcept;

    /**
     * @brief Inequality comparison operator.
     *
     * @param other The page to compare against.
     * @return true if the page IDs are different, false otherwise.
     */
    [[nodiscard]] bool operator!=(const Page& other) const noexcept;

    /**
     * @brief Less-than comparison operator.
     *
     * @param other The page to compare against.
     * @return true if this page's ID is less than the other page's ID, false otherwise.
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
