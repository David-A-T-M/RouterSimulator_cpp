#pragma once

#include <iomanip>
#include <sstream>
#include "IPAddress.h"

/**< Maximum TTL (Time To Live) for a packet to prevent infinite loops in routing. */
constexpr size_t PACKET_TTL = 100;

/**
 * @class Packet
 * @brief Represents a packet that forms part of a page.
 *
 * Stores all the information from the page it belongs to, including its position within the page,
 * and a priority assigned by the router. This priority is used to determine the packet's
 * transmission priority. Packets are the fundamental units used by routers for data transmission.
 */
class Packet {
    size_t pageID;   /**< ID of the page this packet belongs to. */
    size_t pagePos;  /**< Position of this packet within the page. */
    size_t pageLen;  /**< Total number of packets on the page. */
    size_t timeout;  /**< Simulation tick at which this packet should expire. */
    IPAddress srcIP; /**< Reference to the source terminal IP. */
    IPAddress dstIP; /**< Reference to the destination terminal IP. */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a packet.
     *
     * The constructor initializes a packet with the given page information, source and destination
     * IPs, and expiration tick. It validates the parameters to ensure they are consistent with the
     * expected values for a packet.
     *
     * @param pageID ID of the page this packet belongs to.
     * @param pagePos Position of this packet within the page [0, pageLen-1].
     * @param pageLen Total number of packets in the page (must be > 0).
     * @param srcIP Source terminal IP address.
     * @param dstIP Destination terminal IP address.
     * @param timeout Simulation tick at which this packet should expire.
     *
     * @throws std::invalid_argument if pagePos is out of range, if pageLen is 0, or if srcIP/dstIP
     * are invalid.
     */
    Packet(size_t pageID, size_t pagePos, size_t pageLen, IPAddress srcIP, IPAddress dstIP,
           size_t timeout);

    /**
     * @brief Default Copy Constructor.
     */
    Packet(const Packet&) = default;

    /**
     * @brief Default Move Constructor.
     */
    Packet(Packet&&) noexcept = default;

    /**
     * @brief Default Copy Assignment Operator.
     *
     * @return Reference to this packet after the copy assignment.
     */
    Packet& operator=(const Packet&) = default;

    /**
     * @brief Default Move Assignment Operator.
     *
     * @return Reference to this packet after the move assignment.
     */
    Packet& operator=(Packet&&) noexcept = default;

    /**
     * @brief Default Destructor.
     */
    ~Packet() = default;

    // =============== Getters ===============
    /**
     * @brief Gets the page ID.
     *
     * @return Page ID that this packet belongs to.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Gets the position of this packet within its page.
     *
     * @return Position of this packet in the page (0-based index).
     */
    [[nodiscard]] size_t getPagePos() const noexcept;

    /**
     * @brief Gets the total number of packets in the page.
     *
     * @return Total number of packets in the page.
     */
    [[nodiscard]] size_t getPageLen() const noexcept;

    /**
     * @brief Gets the source IP address.
     *
     * @return Source terminal IP address from which this packet originated.
     */
    [[nodiscard]] IPAddress getSrcIP() const noexcept;

    /**
     * @brief Gets the destination IP address.
     *
     * @return Destination terminal IP address to which this packet is being sent.
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    /**
     * @brief Gets the expiration tick for this packet.
     *
     * @return Simulation tick at which this packet should expire and be dropped if not delivered.
     */
    [[nodiscard]] size_t getTimeout() const noexcept;

    // =============== Query Methods ===============
    /**
     * @brief Checks if this is the first packet in the page.
     *
     * @return true if pagePosition == 0, indicating this is the first packet of the page.
     */
    [[nodiscard]] bool isFirstPacket() const noexcept;

    /**
     * @brief Checks if this is the last packet in the page.
     *
     * @return true if pagePosition == pageLen - 1, indicating this is the last packet of the page.
     */
    [[nodiscard]] bool isLastPacket() const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the packet.
     *
     * @return String describing the packet, including pageID, pagePos, pageLen, srcIP, dstIP, and
     * Timeout.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     *
     * @param os Output stream to write to.
     * @param packet The packet to output.
     * @return Reference to the output stream after writing the packet's string representation.
     */
    friend std::ostream& operator<<(std::ostream& os, const Packet& packet);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality comparison operator.
     *
     * @param other Packet to compare with.
     * @return true if both packets have the same pageID and pagePosition, false otherwise.
     */
    [[nodiscard]] bool operator==(const Packet& other) const noexcept;

    /**
     * @brief Inequality comparison operator.
     *
     * @param other Packet to compare with.
     * @return true if the packets differ in pageID or pagePosition.
     */
    [[nodiscard]] bool operator!=(const Packet& other) const noexcept;
};

// =============== Getters ===============
inline size_t Packet::getPageID() const noexcept {
    return pageID;
}

inline size_t Packet::getPagePos() const noexcept {
    return pagePos;
}

inline size_t Packet::getPageLen() const noexcept {
    return pageLen;
}

inline size_t Packet::getTimeout() const noexcept {
    return timeout;
}

inline IPAddress Packet::getDstIP() const noexcept {
    return dstIP;
}

inline IPAddress Packet::getSrcIP() const noexcept {
    return srcIP;
}

// =============== Query Methods ===============
inline bool Packet::isFirstPacket() const noexcept {
    return pagePos == 0;
}

inline bool Packet::isLastPacket() const noexcept {
    return pagePos == pageLen - 1;
}

// =============== Comparison Operators ===============
inline bool Packet::operator==(const Packet& other) const noexcept {
    return pageID == other.pageID && pagePos == other.pagePos;
}

inline bool Packet::operator!=(const Packet& other) const noexcept {
    return !(*this == other);
}
