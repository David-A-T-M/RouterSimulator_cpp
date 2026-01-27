#pragma once

#include <iomanip>
#include <sstream>
#include "core/IPAddress.h"

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
    int pageID;              /**< ID of the page that the packet belongs to. */
    int pagePosition;        /**< Position of the packet in the page. */
    int pageLength;          /**< Length of the page. */
    int routerPriority;      /**< Priority of the packet in the router. The lower the value, the higher the priority. */
    IPAddress destinationIP; /**< Reference to the destination terminal IP. */
    IPAddress originIP;      /**< Reference to the origin terminal IP. */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for creating a packet.
     * @param pageID ID of the page that the packet belongs to (must be >= 0).
     * @param pagePosition Position of the packet in the page (0-based, must be < pageLength).
     * @param pageLength Total length of the page (must be > 0).
     * @param routerPriority Initial router priority.
     * @param destinationIP Destination terminal IP address.
     * @param originIP Origin terminal IP address.
     * @throws std::invalid_argument if parameters are invalid.
     */
    Packet(int pageID, int pagePosition, int pageLength, int routerPriority, IPAddress destinationIP,
           IPAddress originIP);

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
     */
    Packet& operator=(const Packet&) = default;

    /**
     * @brief Default Move Assignment Operator.
     */
    Packet& operator=(Packet&&) noexcept = default;

    /**
     * @brief Default Destructor.
     */
    ~Packet() = default;

    // =============== Setters ===============
    /**
     * @brief Sets the router priority.
     * @param priority New router priority value. The lower the value, the higher the priority.
     */
    void setRouterPriority(int priority) noexcept;

    // =============== Getters ===============
    /**
     * @brief Gets the page ID.
     * @return Page ID.
     */
    [[nodiscard]] int getPageID() const noexcept;

    /**
     * @brief Gets the position of this packet within its page.
     * @return Page position (0-based).
     */
    [[nodiscard]] int getPagePosition() const noexcept;

    /**
     * @brief Gets the total length of the page this packet belongs to.
     * @return Page length.
     */
    [[nodiscard]] int getPageLength() const noexcept;

    /**
     * @brief Gets the router priority assigned to this packet.
     * @return Router priority. The lower the value, the higher the priority.
     */
    [[nodiscard]] int getRouterPriority() const noexcept;

    /**
     * @brief Gets the destination IP address.
     * @return Destination terminal IP.
     */
    [[nodiscard]] IPAddress getDestinationIP() const noexcept;

    /**
     * @brief Gets the origin IP address.
     * @return Origin terminal IP.
     */
    [[nodiscard]] IPAddress getOriginIP() const noexcept;

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

    /**
     * @brief Less-than comparison for ordering packets.
     * Orders by: 1) routerPriority (ascending), 2) pageID, 3) pagePosition.
     * @param other Packet to compare with.
     * @return true if this packet should come before other.
     */
    [[nodiscard]] bool operator<(const Packet& other) const noexcept;

    /**
     * @brief Less-than-or-equal comparison.
     */
    [[nodiscard]] bool operator<=(const Packet& other) const noexcept;

    /**
     * @brief Greater-than comparison.
     */
    [[nodiscard]] bool operator>(const Packet& other) const noexcept;

    /**
     * @brief Greater-than-or-equal comparison.
     */
    [[nodiscard]] bool operator>=(const Packet& other) const noexcept;
};

// =============== Setters ===============
inline void Packet::setRouterPriority(int priority) noexcept {
    routerPriority = priority;
}

// =============== Getters ===============
inline int Packet::getPageID() const noexcept {
    return pageID;
}

inline int Packet::getPagePosition() const noexcept {
    return pagePosition;
}

inline int Packet::getPageLength() const noexcept {
    return pageLength;
}

inline int Packet::getRouterPriority() const noexcept {
    return routerPriority;
}

inline IPAddress Packet::getDestinationIP() const noexcept {
    return destinationIP;
}

inline IPAddress Packet::getOriginIP() const noexcept {
    return originIP;
}

// =============== Query Methods ===============
inline bool Packet::isFirstPacket() const noexcept {
    return pagePosition == 0;
}

inline bool Packet::isLastPacket() const noexcept {
    return pagePosition == pageLength - 1;
}

// =============== Comparison Operators ===============
inline bool Packet::operator==(const Packet& other) const noexcept {
    return pageID == other.pageID && pagePosition == other.pagePosition;
}

inline bool Packet::operator!=(const Packet& other) const noexcept {
    return !(*this == other);
}

inline bool Packet::operator<(const Packet& other) const noexcept {
    if (routerPriority != other.routerPriority) {
        return routerPriority < other.routerPriority;
    }

    if (pageID != other.pageID) {
        return pageID < other.pageID;
    }

    return pagePosition < other.pagePosition;
}

inline bool Packet::operator<=(const Packet& other) const noexcept {
    return !(other < *this);
}

inline bool Packet::operator>(const Packet& other) const noexcept {
    return other < *this;
}

inline bool Packet::operator>=(const Packet& other) const noexcept {
    return !(*this < other);
}
