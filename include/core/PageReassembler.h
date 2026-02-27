#pragma once

#include "Packet.h"
#include "structures/list.h"

constexpr size_t MAX_ASSEMBLER_TTL = 250; /**< Maximum TTL for a PageReassembler */

/**
 * @class PageReassembler
 * @brief Manages the reassembly of packets into a complete page for a specific page ID.
 *
 * The PageReassembler tracks the expected number of packets for a page, the packets received so
 * far, and the expiration tick. It provides methods to add packets, check completion status, and
 * package the received packets into an ordered list. The reassembler can be reset after packaging
 * to allow reuse.
 */
class PageReassembler {
    size_t pageID;    /**< ID of the page being reassembled */
    size_t total;     /**< Total number of packets expected */
    size_t count;     /**< Number of packets received so far */
    size_t expTick;   /**< System tick at which this reassembler should expire if not completed */
    Packet** packets; /**< Array of pointers to received packets, indexed by their position */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for PageReassembler.
     *
     * @param pageID ID of the page being reassembled.
     * @param length Total number of packets expected for the page (must be > 0).
     * @param expTick System tick at which this reassembler should expire if not completed.
     * @throws std::invalid_argument if length is 0.
     */
    PageReassembler(size_t pageID, size_t length, size_t expTick);

    /**
     * @brief Deleted Copy constructor.
     */
    PageReassembler(const PageReassembler&) = delete;

    /**
     * @brief Deleted Copy assignment operator.
     *
     * @return Reference to this object.
     */
    PageReassembler& operator=(const PageReassembler&) = delete;

    /**
     * @brief Deleted Move constructor.
     *
     * @param other The other PageReassembler to move from.
     */
    PageReassembler(PageReassembler&& other) noexcept;

    /**
     * @brief Deleted Move assignment operator.
     *
     * @param other The other PageReassembler to move from.
     * @return Reference to this object.
     */
    PageReassembler& operator=(PageReassembler&& other) noexcept;

    /**
     * @brief Destructor - cleans up all allocated packets.
     */
    ~PageReassembler();

    // =============== Getters ===============
    /**
     * @brief Gets the page ID associated with this reassembler.
     *
     * @return Page ID of the page being reassembled.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Gets the total number of packets expected for this page.
     *
     * @return Total number of packets expected to complete the page.
     */
    [[nodiscard]] size_t getTotalPackets() const noexcept;

    /**
     * @brief Gets the number of packets received so far for this page.
     *
     * @return Number of packets received so far for this page.
     */
    [[nodiscard]] size_t getReceivedPackets() const noexcept;

    /**
     * @brief Gets the expiration tick for this reassembler.
     *
     * @return System tick at which this reassembler should expire if not completed.
     */
    [[nodiscard]] size_t getExpTick() const noexcept;

    /**
     * @brief Gets the completion rate for this page.
     *
     * @return A value between 0.0 and 1.0.
     */
    [[nodiscard]] double getCompletionRate() const;

    /**
     * @brief Gets the number of packets still needed to complete the page.
     *
     * @return Number of packets still needed to complete the page.
     */
    [[nodiscard]] size_t getRemainingPackets() const;

    // =============== Query methods ===============
    /**
     * @brief Checks if all packets have been received.
     *
     * @return true if count == total, false otherwise.
     */
    [[nodiscard]] bool isComplete() const;

    /**
     * @brief Checks if a specific position has been filled.
     *
     * @param position Position to check (0-based index).
     * @return true if a packet has been received for the given position, false otherwise.
     * @throws std::out_of_range if position >= total.
     */
    [[nodiscard]] bool hasPacketAt(size_t position) const;

    // =============== Modifiers ===============
    /**
     * @brief Adds a packet to the reassembler if it matches the page ID and expected total, and if
     * the position is valid and not already filled.
     *
     * @param p Packet to add to the reassembler.
     * @return true if the packet was added successfully, false if the packet is invalid or a packet
     * for that position has already been received.
     */
    bool addPacket(const Packet& p);

    /**
     * @brief Packages the received packets into a List<Packet> in the correct order. This should
     * only be called when isComplete() returns true. After packaging, the reassembler is reset and
     * can be reused for a new page.
     *
     * @return List of packets in the correct order for the completed page.
     * @throws std::runtime_error if the page is not complete or if any packet is missing during
     * packaging.
     */
    List<Packet> package();

    /**
     * @brief Resets the reassembler, deleting all stored packets.
     */
    void reset();

    /**
     * @brief Generates a string representation of the reassembler.
     *
     * @return A string representation of the reassembler, including page ID, total packets,
     * received packets, and expiration tick.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator for PageReassembler.
     *
     * @param os Output stream to write to.
     * @param reassembler The reassembler to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const PageReassembler& reassembler);

    /**
     * @brief Equality operator.
     *
     * @param other The other reassembler to compare to.
     * @return true if both reassemblers have the same pageID, total, count, expTick, and packet
     * presence; false otherwise.
     */
    bool operator==(const PageReassembler& other) const noexcept;

    /**
     * @brief Inequality operator.
     *
     * @param other The other reassembler to compare to.
     * @return true if the reassemblers differ in any of pageID, total, count, expTick, or packet
     * presence; false otherwise.
     */
    bool operator!=(const PageReassembler& other) const noexcept;
};

// =============== Query methods ===============
inline bool PageReassembler::isComplete() const {
    return count == total;
}

// =============== Getters ===============
inline size_t PageReassembler::getPageID() const noexcept {
    return pageID;
}

inline size_t PageReassembler::getTotalPackets() const noexcept {
    return total;
}

inline size_t PageReassembler::getReceivedPackets() const noexcept {
    return count;
}

inline size_t PageReassembler::getExpTick() const noexcept {
    return expTick;
}

inline double PageReassembler::getCompletionRate() const {
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(count) / static_cast<double>(total);
}

inline size_t PageReassembler::getRemainingPackets() const {
    return total - count;
}
