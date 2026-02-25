#pragma once

#include "Packet.h"
#include "structures/list.h"

constexpr size_t MAX_ASSEMBLER_TTL = 250;

/**
 * @class PageReassembler
 * @brief Reassembles packets into a complete page in the correct order.
 *
 * Stores incoming packets (which may arrive out of order) in their correct positions using an array indexed by packet
 * position. Once all packets are received, creates an ordered List<Packet> ready to construct a Page. Will expire if
 * the simulation tick reaches expTick, at which point it should be discarded.
 */
class PageReassembler {
    size_t pageID;    /**< ID of the page being reassembled */
    size_t total;     /**< Total number of packets expected */
    size_t count;     /**< Number of packets received so far */
    size_t expTick;   /**< Tick of expiration */
    Packet** packets; /**< Array of packet pointers indexed by position */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for PageReassembler.
     * @param pageID Page ID to reassemble.
     * @param length Total number of packets on the page (must be > 0).
     * @param expTick Tick at which this reassembler should expire (current system tick + MAX_ASSEMBLER_TTL).
     * @throws std::invalid_argument if parameters are invalid.
     */
    PageReassembler(size_t pageID, size_t length, size_t expTick);

    /**
     * @brief Deleted Copy constructor.
     */
    PageReassembler(const PageReassembler&) = delete;

    /**
     * @brief Deleted Copy assignment operator.
     * @return Reference to this object.
     */
    PageReassembler& operator=(const PageReassembler&) = delete;

    /**
     * @brief Deleted Move constructor.
     * @param other The other PageReassembler to move from.
     */
    PageReassembler(PageReassembler&& other) noexcept;

    /**
     * @brief Deleted Move assignment operator.
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
     * @brief Gets the page ID being reassembled.
     * @return Page ID.
     */
    [[nodiscard]] size_t getPageID() const noexcept;

    /**
     * @brief Gets the total number of packets expected for this page.
     * @return Total number of packets.
     */
    [[nodiscard]] size_t getTotalPackets() const noexcept;

    /**
     * @brief Gets the number of packets received so far.
     * @return Number of packets received.
     */
    [[nodiscard]] size_t getReceivedPackets() const noexcept;

    /**
     * @brief Gets the expiration tick.
     * @return Tick at which this reassembler should expire.
     */
    [[nodiscard]] size_t getExpTick() const noexcept;

    /**
     * @brief Gets the completion percentage.
     * @return Value between 0.0 and 1.0.
     */
    [[nodiscard]] double getCompletionRate() const;

    /**
     * @brief Gets the number of packets still needed.
     * @return Number of packets still needed to complete the page.
     */
    [[nodiscard]] size_t getRemainingPackets() const;

    // =============== Query methods ===============
    /**
     * @brief Checks if all packets have been received.
     * @return true if currentPackets == expectedPackets.
     */
    [[nodiscard]] bool isComplete() const;

    /**
     * @brief Checks if a specific position has been filled.
     * @param position The packet position to check.
     * @return true if a packet at this position has been received.
     * @throws std::out_of_range if position >= total.
     */
    [[nodiscard]] bool hasPacketAt(size_t position) const;

    // =============== Modifiers ===============
    /**
     * @brief Adds a packet to the reassembler.
     *
     * The packet is placed in its correct position based on getPagePosition().
     * Duplicate packets (same position already filled) are ignored.
     *
     * @param p The packet to add.
     * @return true if a packet was added, false if position already occupied or invalid.
     */
    bool addPacket(const Packet& p);

    /**
     * @brief Packages all received packets into an ordered List<Packet>.
     *
     * Creates a List with packets in the correct order (position 0, 1, 2, ...).
     * After calling this, the reassembler is reset and can be reused.
     *
     * @return List<Packet> containing all packets in order.
     * @throws std::runtime_error if not all packets have been received.
     */
    List<Packet> package();

    /**
     * @brief Resets the reassembler, deleting all stored packets.
     */
    void reset();

    /**
     * @brief Gets a string representation of the reassembler's state.
     * @return String describing pageID, total, count, expTick, and packet presence.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator for PageReassembler.
     * @param os Output stream to write to.
     * @param reassembler The reassembler to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const PageReassembler& reassembler);

    /**
     * @brief Equality operator - checks if two reassemblers are in the same state.
     * @param other The other reassembler to compare to.
     * @return true if both reassemblers have the same pageID, total, count, expTick, and packet presence.
     */
    bool operator==(const PageReassembler& other) const noexcept;

    /**
     * @brief Inequality operator - checks if two reassemblers are in different states.
     * @param other The other reassembler to compare to.
     * @return true if any of the pageID, total, count, expTick, or packet presence differ.
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
    if (total == 0)
        return 0.0;
    return static_cast<double>(count) / static_cast<double>(total);
}

inline size_t PageReassembler::getRemainingPackets() const {
    return total - count;
}
