#pragma once

#include "Packet.h"
#include "structures/list.h"

/**
 * @class PacketBuffer
 * @brief Represents a buffer that holds packets in a queue-like structure for terminals and
 * routers.
 *
 * The PacketBuffer class provides functionality to enqueue and dequeue packets, check buffer status
 * (empty/full), and manage buffer capacity. It can be used for both terminal output buffers
 * (packets waiting to be sent to the router) and router input/output buffers (packets waiting to be
 * processed or forwarded).
 */
class PacketBuffer {
    List<Packet> packets; /**< List of packets currently in the buffer, maintained in FIFO order */
    size_t capacity;      /**< Maximum number of packets the buffer can hold (0 = unlimited) */
    IPAddress dstIP;      /**< Associated destination IP for this buffer */

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
     * @param dstIP Associated destination IP for this buffer (0.0 indicates no association).
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
     *
     * @return Reference to this PacketBuffer.
     */
    PacketBuffer& operator=(const PacketBuffer&) = default;

    /**
     * @brief Default Move Assignment operator.
     *
     * @return Reference to this PacketBuffer.
     */
    PacketBuffer& operator=(PacketBuffer&&) noexcept = default;

    // =============== Getters ===============
    /**
     * @brief Gets the associated destination IP.
     *
     * @return Destination IP address (0.0 if no association).
     */
    [[nodiscard]] IPAddress getDstIP() const noexcept;

    /**
     * @brief Gets the maximum capacity.
     *
     * @return Maximum number of packets the buffer can hold (0 = unlimited).
     */
    [[nodiscard]] size_t getCapacity() const noexcept;

    // =============== Queue Operations ===============
    /**
     * @brief Adds a packet to the buffer.
     *
     * @param packet The packet to add to the buffer.
     * @return true if the packet was added successfully, false if the buffer is full and the packet
     * was dropped.
     */
    bool enqueue(const Packet& packet);

    /**
     * @brief Removes and returns the front packet from the buffer.
     *
     * @return The packet at the front of the buffer.
     * @throws std::runtime_error if the buffer is empty when attempting to dequeue.
     */
    Packet dequeue();

    // =============== Query methods ===============
    /**
     * @brief Checks if the buffer is empty.
     *
     * @return true if there are no packets in the buffer, false otherwise.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Checks if the buffer is at full capacity.
     *
     * @return true if the buffer has reached its maximum capacity (if capacity > 0), false
     * otherwise (including unlimited capacity).
     */
    [[nodiscard]] bool isFull() const noexcept;

    /**
     * @brief Gets the current number of packets in the buffer.
     *
     * @return Number of packets currently in the buffer.
     */
    [[nodiscard]] size_t size() const noexcept;

    // =============== Query methods ===============
    /**
     * @brief Gets the number of additional packets that can be added to the buffer before reaching
     * capacity.
     *
     * @return Number of additional packets that can be enqueued (0 if full, or
     * std::numeric_limits<int>::max() if unlimited).
     */
    [[nodiscard]] size_t availableSpace() const noexcept;

    /**
     * @brief Gets the current utilization of the buffer as a percentage of its capacity.
     *
     * @return Utilization as a value between 0.0 and 1.0 (0.0 if empty, or unlimited capacity).
     */
    [[nodiscard]] double getUtilization() const noexcept;

    /**
     * @brief Checks if a packet with a specific page ID and page position exists in the buffer.
     *
     * @param pageID Page ID to search for.
     * @param pagePos Page position to search for.
     * @return true if a packet with the specified page ID and position is found in the buffer,
     * false otherwise.
     */
    [[nodiscard]] bool contains(size_t pageID, size_t pagePos) const;

    // =============== Buffer Management ===============
    /**
     * @brief Clears all packets from the buffer.
     */
    void clear() noexcept;

    /**
     * @brief Sets a new maximum capacity for the buffer.
     *
     * @param newCapacity New maximum capacity (0 = unlimited).
     * @throws std::invalid_argument if newCapacity is less than the current number of packets in
     * the buffer.
     */
    void setCapacity(size_t newCapacity);

    /**
     * @brief Sets a new associated destination IP for this buffer.
     *
     * @param newDst New destination IP address to associate with this buffer.
     */
    void setDstIP(IPAddress newDst) noexcept;

    /**
     * @brief Removes a packet at a specific index from the buffer.
     *
     * @param index Index of the packet to remove (0-based).
     * @throws std::out_of_range if index is greater than or equal to the current number of packets
     * in the buffer.
     */
    void removeAt(size_t index);

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the buffer.
     *
     * @return String describing the buffer's destination IP, capacity, current size, and packet
     * details.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     *
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

inline void PacketBuffer::setDstIP(IPAddress newDst) noexcept {
    dstIP = newDst;
}

inline bool PacketBuffer::isFull() const noexcept {
    if (capacity == 0)
        return false;
    return packets.size() >= capacity;
}

inline size_t PacketBuffer::size() const noexcept {
    return packets.size();
}
