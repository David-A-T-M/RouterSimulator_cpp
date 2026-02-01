#pragma once

#include "Packet.h"
#include "structures/list.h"

/**
 * @struct PageReassembler
 * @brief Reassembles packets into a complete page in the correct order.
 *
 * Stores incoming packets (which may arrive out of order) in their correct
 * positions using an array indexed by packet position. Once all packets are
 * received, creates an ordered List<Packet> ready to construct a Page.
 */
struct PageReassembler {
    size_t pageID;           /**< ID of the page being reassembled */
    size_t expectedPackets;  /**< Total number of packets expected */
    size_t currentPackets;   /**< Number of packets received so far */
    Packet** packetArray; /**< Array of packet pointers indexed by position */

    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor for PageReassembler.
     * @param pageID Page ID to reassemble.
     * @param length Total number of packets in the page (must be > 0).
     * @throws std::invalid_argument if parameters are invalid.
     */
    PageReassembler(size_t pageID, size_t length);

    /**
     * @brief Deleted Copy constructor.
     */
    PageReassembler(const PageReassembler&) = delete;

    /**
     * @brief Deleted Copy assignment operator.
     */
    PageReassembler& operator=(const PageReassembler&) = delete;

    /**
     * @brief Deleted Move constructor.
     */
    PageReassembler(PageReassembler&& other) noexcept;

    /**
     * @brief Deleted Move assignment operator.
     */
    PageReassembler& operator=(PageReassembler&& other) noexcept;

    /**
     * @brief Destructor - cleans up all allocated packets.
     */
    ~PageReassembler();

    // =============== Getters ===============
    /**
     * @brief Gets the completion percentage.
     * @return Value between 0.0 and 1.0.
     */
    [[nodiscard]] double getCompletionRate() const;

    /**
     * @brief Gets the number of packets still needed.
     * @return expectedPackets - currentPackets.
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
     * @return true if packet at this position has been received.
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
     * @return true if packet was added, false if position already occupied or invalid.
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

    bool operator==(const PageReassembler& other) const noexcept;

    bool operator!=(const PageReassembler& other) const noexcept;
};

// =============== Query methods ===============
inline bool PageReassembler::isComplete() const {
    return currentPackets == expectedPackets;
}

// =============== Getters ===============
inline double PageReassembler::getCompletionRate() const {
    if (expectedPackets == 0)
        return 0.0;
    return static_cast<double>(currentPackets) / expectedPackets;
}

inline size_t PageReassembler::getRemainingPackets() const {
    return expectedPackets - currentPackets;
}
