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
 * Stores all the information from the page it belongs to, including its position
 * within the page, and a priority assigned by the router. This priority is used to
 * determine the packet's transmission priority. Packets are the fundamental units
 * used by routers for data transmission.
 */
class Packet {
    size_t pageID;   /**< ID of the page that the packet belongs to. */
    size_t pagePos;  /**< Position of the packet in the page. */
    size_t pageLen;  /**< Length of the page. */
    size_t expTick;  /**< The simulation tick at which the packet should expire. */
    IPAddress srcIP; /**< Reference to the origin terminal IP. */
    IPAddress dstIP; /**< Reference to the destination terminal IP. */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a packet.
     *
     * @param pageID ID of the page that the packet belongs to.
     * @param pagePos Position of the packet in the page (0-based, must be < pageLength).
     * @param pageLen Total length of the page (must be > 0).
     * @param srcIP Origin terminal IP address.
     * @param dstIP Destination terminal IP address.
     * @param expTick The simulation tick at which the packet should expire.
     *
     * @throws std::invalid_argument if parameters are invalid.
     */
    Packet(size_t pageID, size_t pagePos, size_t pageLen, IPAddress srcIP, IPAddress dstIP, size_t expTick);

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
     * @return Reference to this packet after assignment.
     */
    Packet& operator=(const Packet&) = default;

    /**
     * @brief Default Move Assignment Operator.
     * @return Reference to this packet after move assignment.
     */
    Packet& operator=(Packet&&) noexcept = default;

    /**
     * @brief Default Destructor.
     */
    ~Packet() = default;

    // =============== Getters ===============
    /**
     * @brief Gets the page ID.
     * @return Page ID.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Gets the position of this packet within its page.
     * @return Page position (0-based).
     */
    [[nodiscard]] size_t getPagePos() const noexcept;

    /**
     * @brief Gets the total length of the page this packet belongs to.
     * @return Page length.
     */
    [[nodiscard]] size_t getPageLen() const noexcept;

    /**
     * @brief Gets the origin IP address.
     * @return Origin terminal IP.
     */
    [[nodiscard]] IPAddress getSrcIP() const noexcept;

    /**
     * @brief Gets the destination IP address.
     * @return Destination terminal IP.
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    /**
     * @brief Gets the expiration tick of the packet.
     * @return Expiration tick.
     */
    [[nodiscard]] size_t getExpTick() const noexcept;

    // =============== Query Methods ===============
    /**
     * @brief Checks if this is the first packet in the page.
     * @return true if pagePosition == 0.
     */
    [[nodiscard]] bool isFirstPacket() const noexcept;

    /**
     * @brief Checks if this is the last packet in the page.
     * @return true if pagePosition == pageLength - 1.
     */
    [[nodiscard]] bool isLastPacket() const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the packet.
     * @return String in format "Dest: RouterIP - ID: PageID-Position" (e.g., "Dest: 5 - ID: 000000123-4").
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const Packet& packet);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality comparison based on pageID and pagePosition.
     * @param other Packet to compare with.
     * @return true if packets have the same pageID and pagePosition.
     */
    [[nodiscard]] bool operator==(const Packet& other) const noexcept;

    /**
     * @brief Inequality comparison.
     * @param other Packet to compare with.
     * @return true if packets differ in pageID or pagePosition.
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

inline size_t Packet::getExpTick() const noexcept {
    return expTick;
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
