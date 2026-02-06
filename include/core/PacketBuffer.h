#pragma once

#include "Packet.h"
#include "structures/list.h"

/**
 * @class PacketBuffer
 * @brief A flexible packet buffer supporting FIFO and priority-based queueing.
 *
 * This class wraps a List<Packet> and provides two operation modes:
 * - FIFO mode: Simple queue (enqueue at back, dequeue from front)
 * - Priority mode: Packets inserted based on router priority (lower value = higher priority)
 *
 * Used for:
 * - Terminal/Router input buffers (FIFO)
 * - Terminal/Router output buffers (Priority)
 */
class PacketBuffer {
public:
    /**
     * @enum Mode
     * @brief Defines the buffer operation mode.
     */
    enum class Mode {
        FIFO,    /**< First-In-First-Out (simple queue) */
        PRIORITY /**< Priority-based insertion (sorted by router priority) */
    };

private:
    List<Packet> packets;    /**< Internal packet storage */
    Mode mode;               /**< Operation mode */
    size_t capacity;         /**< Maximum capacity (0 = unlimited) */
    IPAddress destinationIP; /**< Associated router IP (for output buffers) */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Constructor with mode specification.
     * @param mode Operation mode (FIFO or PRIORITY).
     * @param capacity Maximum buffer capacity (0 = unlimited, default).
     */
    explicit PacketBuffer(Mode mode = Mode::FIFO, size_t capacity = 0);

    /**
     * @brief Constructor with associated router IP.
     * @param ip Associated router IP address.
     * @param mode Operation mode (FIFO or PRIORITY, default PRIORITY).
     * @param capacity Maximum buffer capacity (0 = unlimited, default).
     */
    explicit PacketBuffer(IPAddress ip, Mode mode = Mode::PRIORITY, size_t capacity = 0);

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
     */
    PacketBuffer& operator=(const PacketBuffer&) = default;

    /**
     * @brief Default Move Assignment operator.
     */
    PacketBuffer& operator=(PacketBuffer&&) noexcept = default;

    // =============== Getters ===============
    /**
     * @brief Gets the associated destination IP.
     * @return Destination IP address.
     * @note A 0.0 IP indicates no specific association.
     */
    [[nodiscard]] IPAddress getDestinationIP() const noexcept;

    /**
     * @brief Gets the maximum capacity.
     * @return Capacity (0 = unlimited).
     */
    [[nodiscard]] size_t getCapacity() const noexcept;

    /**
     * @brief Gets the current operation mode.
     * @return Current mode (FIFO or PRIORITY).
     */
    [[nodiscard]] Mode getMode() const noexcept;

    /**
     * @brief Gets the highest priority value in the buffer.
     * @return Highest priority (lowest urgency), or -1 if empty.
     */
    [[nodiscard]] int getMaxPriority() const;

    /**
     * @brief Gets the lowest priority value in the buffer.
     * @return Lowest priority (highest urgency), or -1 if empty.
     */
    [[nodiscard]] int getMinPriority() const;

    // =============== Queue Operations ===============
    /**
     * @brief Adds a packet to the buffer.
     *
     * In FIFO mode: Adds to the back of the queue.
     * In PRIORITY mode: Inserts based on router priority (lower priority value = higher urgency).
     *
     * @param packet The packet to add.
     * @return true if packet was added, false if buffer is full.
     */
    bool enqueue(const Packet& packet);

    /**
     * @brief Removes and returns the front packet.
     *
     * In both modes, removes from the front of the queue.
     *
     * @return The front packet.
     * @throws std::runtime_error if buffer is empty.
     */
    Packet dequeue();

    // =============== Query methods ===============
    /**
     * @brief Checks if the buffer is empty.
     * @return true if no packets in buffer.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Checks if the buffer is full.
     * @return true if at capacity (always false if capacity = 0).
     */
    [[nodiscard]] bool isFull() const noexcept;

    /**
     * @brief Gets the current number of packets.
     * @return Number of packets in buffer.
     */
    [[nodiscard]] size_t size() const noexcept;

    /**
     * @brief Gets the number of available slots.
     * @return capacity - size() (or INT_MAX if unlimited).
     */
    [[nodiscard]] size_t availableSpace() const noexcept;

    /**
     * @brief Gets the buffer utilization rate.
     * @return Value between 0.0 and 1.0 (0.0 if unlimited capacity).
     */
    [[nodiscard]] double getUtilization() const noexcept;

    /**
     * @brief Checks if buffer contains a packet with specific pageID and position.
     * @param pageID Page ID to search for.
     * @param pagePosition Packet position to search for.
     * @return true if found.
     */
    [[nodiscard]] bool contains(int pageID, int pagePosition) const;

    /**
     * @brief Counts packets belonging to a specific page.
     * @param pageID Page ID to count.
     * @return Number of packets from this page.
     */
    [[nodiscard]] size_t countPacketsFromPage(int pageID) const;

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
    void setCapacity(int newCapacity);

    /**
     * @brief Removes a specific packet from the buffer.
     * @param index Position to remove.
     * @throws std::out_of_range if index is invalid.
     */
    void removeAt(int index);

    /**
     * @brief Removes and returns all packets destined to a specific router.
     *
     * Useful for rerouting packets when routing tables change.
     *
     * @param routerIP Router IP to extract packets for.
     * @return List of packets that were removed.
     */
    List<Packet> extractPacketsByDestinationRouter(uint8_t routerIP);

    /**
     * @brief Transfers packets to another buffer based on destination.
     *
     * Removes packets destined to specific router and adds them to target buffer.
     * Used during route recalculation.
     *
     * @param routerIP Destination router IP to transfer.
     * @param targetBuffer Buffer to transfer packets to.
     * @return Number of packets transferred.
     * @note If target buffer is full, remaining packets stay in this buffer.
     */
    int transferPacketsByDestination(uint8_t routerIP, PacketBuffer& targetBuffer);

    /**
     * @brief Gets all packets that should be rerouted based on a predicate function.
     *
     * @param shouldReroute Function that returns true if packet needs rerouting.
     * @return List of packets that need rerouting (removed from buffer).
     */
    template <typename Predicate>
    List<Packet> extractPacketsIf(Predicate shouldReroute) {
        List<Packet> extracted;

        for (size_t i = packets.size(); i > 0; --i) {
            if (shouldReroute(packets[i - 1])) {
                extracted.pushFront((packets[i]));
                packets.removeAt(i);
            }
        }

        return extracted;
    }

    // =============== Utilities ===============
    /**
     * @brief Gets a string representation of the buffer.
     * @return String describing buffer state.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Stream output operator.
     */
    friend std::ostream& operator<<(std::ostream& os, const PacketBuffer& buffer);

private:
    /**
     * @brief Finds the correct insertion position for priority mode.
     * @param packet The packet to insert.
     * @return Index where packet should be inserted.
     */
    [[nodiscard]] size_t findPriorityPosition(const Packet& packet) const;
};

// =============== Getters ===============
[[nodiscard]] inline IPAddress PacketBuffer::getDestinationIP() const noexcept {
    return destinationIP;
}

inline size_t PacketBuffer::getCapacity() const noexcept {
    return capacity;
}

inline PacketBuffer::Mode PacketBuffer::getMode() const noexcept {
    return mode;
}

// =============== Query methods ===============
inline bool PacketBuffer::isEmpty() const noexcept {
    return packets.isEmpty();
}

inline bool PacketBuffer::isFull() const noexcept {
    if (capacity == 0)
        return false;
    return packets.size() >= capacity;
}

inline size_t PacketBuffer::size() const noexcept {
    return packets.size();
}
