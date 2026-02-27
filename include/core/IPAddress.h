#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

/**
 * @class IPAddress
 * @brief Represents a compact 2-byte IP address for routers and terminals in the network
 * simulation.
 *
 * The IP address is structured as follows:
 * - The upper 8 bits (bits 15-8) represent the Router ID, which identifies the network/router.
 * - The lower 8 bits (bits 7-0) represent the Terminal ID, which identifies a specific terminal
 * connected to that router.
 */
class IPAddress {
    uint16_t address{}; /**< Internal 16-bit representation of the IP address, combining router and
                           terminal IDs. */

public:
    // =============== Constructors & Destructor ===============
    /**
     * @brief Default constructor initializes the IP address to 0.0 (invalid).
     */
    constexpr IPAddress() noexcept = default;

    /**
     * @brief Constructor that takes separate router and terminal IDs to create a compact IP
     * address.
     *
     * The routerIP is stored in the upper 8 bits and the terminalIP in the lower 8 bits of the
     * 16-bit address.
     *
     * @param routerIP The 8-bit identifier for the router (must be > 0 for valid routers).
     * @param terminalIP The 8-bit identifier for the terminal (0 for routers, > 0 for terminals).
     * Default is 0, which indicates a router.
     */
    constexpr explicit IPAddress(uint8_t routerIP, uint8_t terminalIP = 0) noexcept;

    /**
     * @brief Constructor that takes a raw 16-bit integer to create an IP address. This allows for
     * direct initialization from a pre-packed address, but requires the caller to ensure the
     * correct format (router in upper 8 bits, terminal in lower 8 bits).
     *
     * @param rawAddress A 16-bit integer where the upper 8 bits represent the router ID and the
     * lower 8 bits represent the terminal ID.
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
     *
     * @return Reference to this IPAddress after copy assignment.
     */
    IPAddress& operator=(const IPAddress&) = default;

    /**
     * @brief Default move assignment operator.
     *
     * @return Reference to this IPAddress after move assignment.
     */
    IPAddress& operator=(IPAddress&&) noexcept = default;

    // =============== Getters ===============
    /**
     * @brief Extracts the 8-bit Router ID from the 16-bit address.
     *
     * @return The upper 8 bits of the address representing the Router ID.
     */
    [[nodiscard]] constexpr uint8_t getRouterIP() const noexcept;

    /**
     * @brief Extracts the 8-bit Terminal ID from the 16-bit address.
     *
     * @return The lower 8 bits of the address representing the Terminal ID.
     */
    [[nodiscard]] constexpr uint8_t getTerminalIP() const noexcept;

    /**
     * @brief Returns the raw 16-bit integer representation of the IP address, combining both the
     * Router ID and Terminal ID.
     *
     * @return The 16-bit integer where the upper 8 bits are the Router ID and the lower 8 bits are
     * the Terminal ID.
     */
    [[nodiscard]] constexpr uint16_t getRawAddress() const noexcept;

    // =============== Query Methods ===============
    /**
     * @brief Determines if this IP address represents a Router or a Terminal based on the Terminal
     * ID.
     *
     * @return true if the Terminal ID is 0 (indicating a Router), false if the Terminal ID is
     * greater than 0 (indicating a Terminal).
     */
    [[nodiscard]] constexpr bool isRouter() const noexcept;

    /**
     * @brief Checks if the IP address is valid.
     *
     * @return true if the address is valid (not 0.0), false if it is invalid (0.0).
     */
    [[nodiscard]] constexpr bool isValid() const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Converts the IP address to a human-readable string format "RRR.TTT", where RRR is the
     * Router ID and TTT is the Terminal ID, both zero-padded to 3 digits.
     *
     * @return A string describing the IP address in a human-readable format.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Overloads the stream insertion operator for IPAddress.
     *
     * @param os The output stream.
     * @param ip The IPAddress object to print.
     * @return Reference to the output stream after inserting the IP address string representation.
     */
    friend std::ostream& operator<<(std::ostream& os, const IPAddress& ip);

    // =============== Comparison Operators ===============
    /**
     * @brief Equality operator to compare two IPAddress objects.
     *
     * @param other The other IPAddress to compare with.
     * @return true if both IP addresses are the same (same router and terminal IDs), false
     * otherwise.
     */
    [[nodiscard]] constexpr bool operator==(const IPAddress& other) const noexcept;

    /**
     * @brief Inequality operator to compare two IPAddress objects.
     *
     * @param other The other IPAddress to compare with.
     * @return true if the IP addresses are different, false if they are the same.
     */
    [[nodiscard]] constexpr bool operator!=(const IPAddress& other) const noexcept;

    /**
     * @brief Less-than operator for ordering IPAddress objects.
     *
     * @param other The other IPAddress to compare with.
     * @return true if this address is less than the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator<(const IPAddress& other) const noexcept;

    /**
     * @brief Less-than-or-equal operator for ordering IPAddress objects.
     *
     * @param other The other IPAddress to compare with.
     * @return true if this address is less than or equal to the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator<=(const IPAddress& other) const noexcept;

    /**
     * @brief Greater-than operator for ordering IPAddress objects.
     *
     * @param other The other IPAddress to compare with.
     * @return true if this address is greater than the other, false otherwise.
     */
    [[nodiscard]] constexpr bool operator>(const IPAddress& other) const noexcept;

    /**
     * @brief Greater-than-or-equal operator for ordering IPAddress objects.
     *
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
    oss << std::setfill('0') << std::setw(3) << +r << "." << std::setfill('0') << std::setw(3)
        << +t;

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
