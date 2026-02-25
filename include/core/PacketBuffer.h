#pragma once

#include "Packet.h"
#include "structures/list.h"

/**
 * @class PacketBuffer
 * @brief A packet buffer supporting FIFO queueing.
 *
 * This class wraps a List<Packet> and provides a simple queue interface (enqueue at back, dequeue from front)
 *
 * Used for:
 * - Terminal/Router input/output buffers (FIFO)
 */
class PacketBuffer {
    List<Packet> packets; /**< Internal packet storage */
    size_t capacity;      /**< Maximum capacity (0 = unlimited) */
    IPAddress dstIP;      /**< Associated router IP (for output buffers) */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor with optional capacity.
     *
     * @param capacity Maximum buffer capacity (0 = unlimited, default).
     */
    explicit PacketBuffer(size_t capacity = 0);

    /**
     * @brief Constructor with associated router IP.
     *
     * @param dstIP Associated router IP address.
     *
     * @param capacity Maximum buffer capacity (0 = unlimited, default).
     */
    explicit PacketBuffer(IPAddress dstIP, size_t capacity = 0);

    /**
     * @brief Destructor
     */
    ~PacketBuffer() = default;

    /**
     * @brief Default Copy constructor.
     */
    PacketBuffer(const PacketBuffer&) = default;

    /**
     * @brief Default Move constructor.
     */
    PacketBuffer(PacketBuffer&&) noexcept = default;

    /**
     * @brief Default Copy Assignment operator.
     * @return Reference to this PacketBuffer.
     */
    PacketBuffer& operator=(const PacketBuffer&) = default;

    /**
     * @brief Default Move Assignment operator.
     * @return Reference to this PacketBuffer.
     */
    PacketBuffer& operator=(PacketBuffer&&) noexcept = default;

    // =============== Getters ===============
    /**
     * @brief Gets the associated destination IP.
     *
     * @return Destination IP address.
     *
     * @note A 0.0 IP indicates no specific association.
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    /**
     * @brief Gets the maximum capacity.
     * @return Capacity (0 = unlimited).
     */
    [[nodiscard]] size_t getCapacity() const noexcept;

    // =============== Queue Operations ===============
    /**
     * @brief Adds a packet to the buffer.
     *
     * @param packet The packet to add.
     *
     * @return true if a packet was added, false if the buffer is full.
     */
    bool enqueue(const Packet& packet);

    /**
     * @brief Removes and returns the front packet.
     *
     * In both modes, removes from the front of the queue.
     *
     * @return The front packet.
     * @throws std::runtime_error if the buffer is empty.
     */
    Packet dequeue();

    // =============== Query methods ===============
    /**
     * @brief Checks if the buffer is empty.
     * @return true if no packets in the buffer.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Checks if the buffer is full.
     * @return true if at capacity (always false if capacity = 0).
     */
    [[nodiscard]] bool isFull() const noexcept;

    /**
     * @brief Gets the current number of packets.
     * @return Number of packets in the buffer.
     */
    [[nodiscard]] size_t size() const noexcept;

    // =============== Query methods ===============
    /**
     * @brief Gets the number of available slots.
     * @return Number of additional packets that can be added (or INT_MAX if unlimited).
     */
    [[nodiscard]] size_t availableSpace() const noexcept;

    /**
     * @brief Gets the buffer utilization rate.
     * @return Value between 0.0 and 1.0 (0.0 if unlimited capacity).
     */
    [[nodiscard]] double getUtilization() const noexcept;

    /**
     * @brief Checks if the buffer contains a packet with a specific pageID and position.
     * @param pageID Page ID to search for.
     * @param pagePos Packet position to search for.
     * @return true if found.
     */
    [[nodiscard]] bool contains(size_t pageID, size_t pagePos) const;

    // =============== Buffer Management ===============
    /**
     * @brief Clears all packets from the buffer.
     */
    void clear() noexcept;

    /**
     * @brief Sets a new capacity.
     * @param newCapacity New maximum capacity (0 = unlimited).
     * @throws std::invalid_argument if newCapacity < current size.
     */
    void setCapacity(size_t newCapacity);

    /**
     * @brief Sets the associated destination IP.
     * @param dstIP New destination IP address.
     */
    void setDstIP(IPAddress dstIP) noexcept;

    /**
     * @brief Removes a specific packet from the buffer.
     * @param index Position to remove.
     * @throws std::out_of_range if the index is invalid.
     */
    void removeAt(size_t index);

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the buffer.
     * @return String describing buffer state.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     * @param os Output stream.
     * @param buffer Buffer to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const PacketBuffer& buffer);
};

// =============== Getters ===============
[[nodiscard]] inline IPAddress PacketBuffer::getDstIP() const noexcept {
    return dstIP;
}

inline size_t PacketBuffer::getCapacity() const noexcept {
    return capacity;
}

// =============== Query methods ===============
inline bool PacketBuffer::isEmpty() const noexcept {
    return packets.isEmpty();
}

inline void PacketBuffer::setDstIP(IPAddress dstIP) noexcept {
    PacketBuffer::dstIP = dstIP;
}

inline bool PacketBuffer::isFull() const noexcept {
    if (capacity == 0)
        return false;
    return packets.size() >= capacity;
}

inline size_t PacketBuffer::size() const noexcept {
    return packets.size();
}
