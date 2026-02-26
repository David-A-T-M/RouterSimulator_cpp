#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

/**
 * @class IPAddress
 * @brief Represents a simplified IP address with Router.Terminal format.
 *
 * This class efficiently stores a 2-byte IP address where:
 * - The high byte represents the Router ID (0-255)
 * - The low byte represents the Terminal ID (0-255)
 * - If the Terminal ID is 0, it represents only a Router
 */
class IPAddress {
    uint16_t address{};

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Default constructor, initializes the IP address to 0.0.
     */
    constexpr IPAddress() noexcept = default;

    /**
     * @brief Constructs a 16-bit IP address from 8-bit components.
     *
     * Uses bitwise shifting to pack the router and terminal IDs into a single
     * 16-bit integer for memory efficiency and fast comparisons.
     *
     * @param routerIP The 8-bit identifier for the network/router.
     * @param terminalIP The 8-bit identifier for the specific terminal (default is 0 for routers).
     */
    constexpr explicit IPAddress(uint8_t routerIP, uint8_t terminalIP = 0) noexcept;

    /**
     * @brief Constructs an IPAddress directly from a raw 16-bit value.
     * @param rawAddress The pre-packed 16-bit representation.
     */
    constexpr explicit IPAddress(uint16_t rawAddress) noexcept;

    /**
     * @brief Default destructor.
     */
    ~IPAddress() = default;

    /**
     * @brief Default copy constructor.
     */
    IPAddress(const IPAddress&) = default;

    /**
     * @brief Default move constructor.
     */
    IPAddress(IPAddress&&) noexcept = default;

    /**
     * @brief Default copy assignment operator.
     * @return Reference to this IPAddress after assignment.
     */
    IPAddress& operator=(const IPAddress&) = default;

    /**
     * @brief Default move assignment operator.
     * @return Reference to this IPAddress after move assignment.
     */
    IPAddress& operator=(IPAddress&&) noexcept = default;

    // =============== Getters ===============
    /**
     * @brief Extracts the 8-bit Router ID from the 16-bit address.
     * @return The upper 8 bits of the address.
     */
    [[nodiscard]] constexpr uint8_t getRouterIP() const noexcept;

    /**
     * @brief Extracts the 8-bit Terminal ID from the 16-bit address.
     * @return The lower 8 bits of the address.
     */
    [[nodiscard]] constexpr uint8_t getTerminalIP() const noexcept;

    /**
     * @brief Returns the raw 16-bit integer representation of the IP.
     * @return uint16_t containing both Router and Terminal segments.
     */
    [[nodiscard]] constexpr uint16_t getRawAddress() const noexcept;

    // =============== Query Methods ===============
    /**
     * @brief Checks if the address represents a Router node.
     *
     * In this simulation, a Router is defined by having a
     * Terminal ID equal to zero.
     *
     * @return true if it is a Router, false if it is a Terminal.
     */
    [[nodiscard]] constexpr bool isRouter() const noexcept;

    /**
     * @brief Validates if the IP address is not null (0.0).
     * @return true if the address is non-zero, false otherwise.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Returns a human-readable string representation of the IP.
     *
     * Formatting:
     * - Routers: "Router(X)"
     * - Terminals: "X.Y"
     *
     * @return std::string formatted address.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Overloads the stream insertion operator for IPAddress.
     * @param os The output stream.
     * @param ip The IPAddress object to print.
     * @return std::ostream& The modified output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const IPAddress& ip);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality operator to compare two IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if both addresses are equal, false otherwise.
     */
    [[nodiscard]] constexpr bool operator==(const IPAddress& other) const noexcept;

    /**
     * @brief Inequality operator to compare two IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if the addresses are not equal, false otherwise.
     */
    [[nodiscard]] constexpr bool operator!=(const IPAddress& other) const noexcept;

    /**
     * @brief Less-than operator for ordering IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if this address is less than the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator<(const IPAddress& other) const noexcept;

    /**
     * @brief Less-than-or-equal operator for ordering IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if this address is less than or equal to the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator<=(const IPAddress& other) const noexcept;

    /**
     * @brief Greater-than operator for ordering IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if this address is greater than the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator>(const IPAddress& other) const noexcept;

    /**
     * @brief Greater-than-or-equal operator for ordering IPAddress objects.
     * @param other The other IPAddress to compare with.
     * @return true if this address is greater than or equal to the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator>=(const IPAddress& other) const noexcept;
};

// =============== Constructors & Destructor ===============
constexpr IPAddress::IPAddress(uint8_t routerIP, uint8_t terminalIP) noexcept
    : address(static_cast<uint16_t>(routerIP) << 8 | terminalIP) {}

constexpr IPAddress::IPAddress(uint16_t rawAddress) noexcept : address(rawAddress) {}

// =============== Getters ===============
constexpr uint8_t IPAddress::getRouterIP() const noexcept {
    return static_cast<uint8_t>(address >> 8);
}

constexpr uint8_t IPAddress::getTerminalIP() const noexcept {
    return static_cast<uint8_t>(address & 0xFF);
}

constexpr uint16_t IPAddress::getRawAddress() const noexcept {
    return address;
}

// =============== Query Methods ===============
constexpr bool IPAddress::isRouter() const noexcept {
    return (address & 0xFF) == 0;
}

constexpr bool IPAddress::isValid() const noexcept {
    return address != 0;
}

// =============== Utilities ===============
inline std::string IPAddress::toString() const {
    const uint8_t r = getRouterIP();
    const uint8_t t = getTerminalIP();

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(3) << +r << "." << std::setfill('0') << std::setw(3) << +t;

    return oss.str();
}

inline std::ostream& operator<<(std::ostream& os, const IPAddress& ip) {
    os << ip.toString();
    return os;
}

// =============== Comparison Operators ===============
constexpr bool IPAddress::operator==(const IPAddress& other) const noexcept {
    return address == other.address;
}

constexpr bool IPAddress::operator!=(const IPAddress& other) const noexcept {
    return address != other.address;
}

constexpr bool IPAddress::operator<(const IPAddress& other) const noexcept {
    return address < other.address;
}

constexpr bool IPAddress::operator<=(const IPAddress& other) const noexcept {
    return address <= other.address;
}

constexpr bool IPAddress::operator>(const IPAddress& other) const noexcept {
    return address > other.address;
}

constexpr bool IPAddress::operator>=(const IPAddress& other) const noexcept {
    return address >= other.address;
}

/** @cond */
template <>
struct std::hash<IPAddress> {
    size_t operator()(const IPAddress& ip) const noexcept {
        return std::hash<uint16_t>{}(ip.getRawAddress());
    }
};
/** @endcond */
