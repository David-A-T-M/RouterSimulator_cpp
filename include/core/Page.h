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
    size_t pageID;              /**< Unique identifier for the page */
    size_t pageLength;       /**< Page length in packets */
    IPAddress originIP;      /**< Sender Terminal IP */
    IPAddress destinationIP; /**< Destination Terminal IP */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a new page at origin terminal.
     * @param pageID Unique identifier for the page (must be >= 0).
     * @param pageLength The number of packets the page will contain (must be > 0).
     * @param originIP The origin IP address for the page.
     * @param destinationIP The destination IP address for the page.
     * @throws std::invalid_argument if parameters are invalid.
     */
    Page(size_t pageID, size_t pageLength, IPAddress originIP, IPAddress destinationIP);

    /**
     * @brief Constructor that creates a Page from a list of completed packets.
     *
     * This constructor is used when reassembling a page from received packets.
     * It takes ownership of the packets (move semantics).
     *
     * @param completedPackets A list of packets that make up the completed page.
     * @throws std::invalid_argument if packet list is empty or packets are inconsistent.
     * @note The packet list is moved and will be empty after this call.
     * @note The List<Packet> must contain all packets for the page, with correct positions.
     */
    explicit Page(List<Packet>&& completedPackets);

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
     */
    Page& operator=(const Page&) = default;

    /**
     * @brief Default Move Assignment Operator
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
    [[nodiscard]] size_t getPageLength() const noexcept;

    /**
     * @brief Retrieves the origin IP address of the page.
     * @return The origin IP address.
     */
    [[nodiscard]] IPAddress getOriginIP() const noexcept;

    /**
     * @brief Retrieves the destination IP address of the page.
     * @return The destination IP address.
     */
    [[nodiscard]] IPAddress getDestinationIP() const noexcept;

    // =============== Page Operations ===============
    /**
     * @brief Fragments this page into a list of packets ready for transmission.
     *
     * Creates a list of packets, each containing the page metadata and their
     * position within the page. The initial router priority is set to 0.
     *
     * @return A list of packets representing this page.
     */
    [[nodiscard]] List<Packet> fragmentToPackets() const;

    /**
     * @brief Fragments this page into a list of packets with specified initial priority.
     *
     * @param initialPriority Initial router priority for all packets.
     * @return A list of packets representing this page.
     */
    [[nodiscard]] List<Packet> fragmentToPackets(int initialPriority) const;

    // =============== Query Methods ===============
    /**
     * @brief Checks if this page is addressed to a specific destination.
     * @param destination The IP address to check.
     * @return true if destinationIP matches.
     */
    [[nodiscard]] bool isAddressedTo(IPAddress destination) const noexcept;

    /**
     * @brief Checks if this page originated from a specific source.
     * @param source The IP address to check.
     * @return true if originIP matches.
     */
    [[nodiscard]] bool isFrom(IPAddress source) const noexcept;

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
     */
    [[nodiscard]] bool operator==(const Page& other) const noexcept;

    /**
     * @brief Inequality comparison.
     */
    [[nodiscard]] bool operator!=(const Page& other) const noexcept;

    /**
     * @brief Less-than comparison (for sorting).
     * Orders by: pageID.
     */
    [[nodiscard]] bool operator<(const Page& other) const noexcept;
};

inline size_t Page::getPageID() const noexcept {
    return pageID;
}

inline size_t Page::getPageLength() const noexcept {
    return pageLength;
}

inline IPAddress Page::getOriginIP() const noexcept {
    return originIP;
}

inline IPAddress Page::getDestinationIP() const noexcept {
    return destinationIP;
}

inline bool Page::isAddressedTo(IPAddress destination) const noexcept {
    return destinationIP == destination;
}

inline bool Page::isFrom(IPAddress source) const noexcept {
    return originIP == source;
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
