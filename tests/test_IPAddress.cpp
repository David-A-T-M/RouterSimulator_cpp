#include <gtest/gtest.h>
#include <algorithm>
#include "core/IPAddress.h"

// =============== Constructors and assignment tests ===============
TEST(IPAddressConstructors, DefaultConstructor) {
    const IPAddress ip;

    EXPECT_EQ(ip.getRouterIP(), 0);
    EXPECT_EQ(ip.getTerminalIP(), 0);
    EXPECT_EQ(ip.getRawAddress(), 0);
}

TEST(IPAddressConstructors, RouterConstructor) {
    const IPAddress ip(static_cast<uint8_t>(10));

    EXPECT_EQ(ip.getRouterIP(), 10);
    EXPECT_EQ(ip.getTerminalIP(), 0);
    EXPECT_TRUE(ip.isRouter());
}

TEST(IPAddressConstructors, RouterAndTerminalConstructor) {
    const IPAddress ip(192, 168);

    EXPECT_EQ(ip.getRouterIP(), 192);
    EXPECT_EQ(ip.getTerminalIP(), 168);
    EXPECT_FALSE(ip.isRouter());
}

TEST(IPAddressConstructors, RawAddressConstructor) {
    const IPAddress ip(static_cast<uint16_t>(0x0A64));

    EXPECT_EQ(ip.getRouterIP(), 10);
    EXPECT_EQ(ip.getTerminalIP(), 100);
}

TEST(IPAddressConstructors, CopyConstructor) {
    const IPAddress ip1(50, 25);
    const IPAddress ip2(ip1);

    EXPECT_EQ(ip2.getRouterIP(), 50);
    EXPECT_EQ(ip2.getTerminalIP(), 25);
    EXPECT_EQ(ip1, ip2);
}

TEST(IPAddressConstructors, CopyAssignment) {
    const IPAddress ip1(10, 20);

    IPAddress ip2 = ip1;

    EXPECT_EQ(ip2.getRouterIP(), 10);
    EXPECT_EQ(ip2.getTerminalIP(), 20);
}

// =============== Getters tests ===============
TEST(IPAddressGetter, GetRouterIPVariousValues) {
    EXPECT_EQ(IPAddress(0, 0).getRouterIP(), 0);
    EXPECT_EQ(IPAddress(1, 0).getRouterIP(), 1);
    EXPECT_EQ(IPAddress(127, 0).getRouterIP(), 127);
    EXPECT_EQ(IPAddress(255, 0).getRouterIP(), 255);
}

TEST(IPAddressGetter, GetTerminalIPVariousValues) {
    EXPECT_EQ(IPAddress(10, 0).getTerminalIP(), 0);
    EXPECT_EQ(IPAddress(10, 1).getTerminalIP(), 1);
    EXPECT_EQ(IPAddress(10, 127).getTerminalIP(), 127);
    EXPECT_EQ(IPAddress(10, 255).getTerminalIP(), 255);
}

TEST(IPAddressGetter, GetRawAddress) {
    EXPECT_EQ(IPAddress(0, 0).getRawAddress(), 0x0000);
    EXPECT_EQ(IPAddress(1, 2).getRawAddress(), 0x0102);
    EXPECT_EQ(IPAddress(255, 255).getRawAddress(), 0xFFFF);
    EXPECT_EQ(IPAddress(10, 100).getRawAddress(), 0x0A64);
}

// =============== Query Methods tests ===============
TEST(IPAddressQuery, IsRouter) {
    EXPECT_TRUE(IPAddress(0, 0).isRouter());
    EXPECT_TRUE(IPAddress(10, 0).isRouter());
    EXPECT_TRUE(IPAddress(255, 0).isRouter());

    EXPECT_FALSE(IPAddress(10, 1).isRouter());
    EXPECT_FALSE(IPAddress(10, 255).isRouter());
}

TEST(IPAddressQuery, IsValid) {
    EXPECT_FALSE(IPAddress(0, 0).isValid());
    EXPECT_TRUE(IPAddress(1, 0).isValid());
    EXPECT_TRUE(IPAddress(0, 1).isValid());
    EXPECT_TRUE(IPAddress(10, 20).isValid());
}

// =============== Utilities tests ===============
TEST(IPAddressUtilities, ToStringRouter) {
    EXPECT_EQ(IPAddress(0, 0).toString(), "Router(0)");
    EXPECT_EQ(IPAddress(10, 0).toString(), "Router(10)");
    EXPECT_EQ(IPAddress(255, 0).toString(), "Router(255)");
}

TEST(IPAddressUtilities, ToStringTerminal) {
    EXPECT_EQ(IPAddress(10, 1).toString(), "10.1");
    EXPECT_EQ(IPAddress(192, 168).toString(), "192.168");
    EXPECT_EQ(IPAddress(255, 255).toString(), "255.255");
    EXPECT_EQ(IPAddress(0, 1).toString(), "0.1");
}

TEST(IPAddressUtilities, StreamOperatorRouter) {
    const IPAddress ip(42, 0);
    std::ostringstream oss;

    oss << ip;

    EXPECT_EQ(oss.str(), "Router(42)");
}

TEST(IPAddressUtilities, StreamOperatorTerminal) {
    const IPAddress ip(192, 168);
    std::ostringstream oss;

    oss << ip;

    EXPECT_EQ(oss.str(), "192.168");
}

TEST(IPAddressUtilities, StreamOperatorMultiple) {
    const IPAddress ip1(10, 0);
    const IPAddress ip2(20, 30);
    std::ostringstream oss;

    oss << ip1 << " and " << ip2;

    EXPECT_EQ(oss.str(), "Router(10) and 20.30");
}

// =============== Comparison Operators tests ===============
TEST(IPAddressComparison, EqualityOperator) {
    const IPAddress ip1(10, 20);
    const IPAddress ip2(10, 20);
    const IPAddress ip3(10, 21);

    EXPECT_TRUE(ip1 == ip2);
    EXPECT_FALSE(ip1 == ip3);
    EXPECT_TRUE(IPAddress(0, 0) == IPAddress(0, 0));
}

TEST(IPAddressComparison, InequalityOperator) {
    const IPAddress ip1(10, 20);
    const IPAddress ip2(10, 21);
    const IPAddress ip3(10, 20);

    EXPECT_TRUE(ip1 != ip2);
    EXPECT_FALSE(ip1 != ip3);
}

TEST(IPAddressComparison, LessThanOperator) {
    EXPECT_TRUE(IPAddress(10, 20) < IPAddress(10, 21));
    EXPECT_TRUE(IPAddress(10, 20) < IPAddress(11, 0));
    EXPECT_FALSE(IPAddress(10, 20) < IPAddress(10, 20));
    EXPECT_FALSE(IPAddress(10, 21) < IPAddress(10, 20));
}

TEST(IPAddressComparison, LessThanOrEqualOperator) {
    EXPECT_TRUE(IPAddress(10, 20) <= IPAddress(10, 21));
    EXPECT_TRUE(IPAddress(10, 20) <= IPAddress(10, 20));
    EXPECT_FALSE(IPAddress(10, 21) <= IPAddress(10, 20));
}

TEST(IPAddressComparison, GreaterThanOperator) {
    EXPECT_TRUE(IPAddress(10, 21) > IPAddress(10, 20));
    EXPECT_TRUE(IPAddress(11, 0) > IPAddress(10, 255));
    EXPECT_FALSE(IPAddress(10, 20) > IPAddress(10, 20));
    EXPECT_FALSE(IPAddress(10, 20) > IPAddress(10, 21));
}

TEST(IPAddressComparison, GreaterThanOrEqualOperator) {
    EXPECT_TRUE(IPAddress(10, 21) >= IPAddress(10, 20));
    EXPECT_TRUE(IPAddress(10, 20) >= IPAddress(10, 20));
    EXPECT_FALSE(IPAddress(10, 20) >= IPAddress(10, 21));
}

TEST(IPAddressComparison, ComparisonConsistency) {
    const IPAddress ip1(5, 10);
    const IPAddress ip2(5, 20);
    const IPAddress ip3(5, 10);

    // Reflexivity
    EXPECT_TRUE(ip1 == ip1);
    EXPECT_FALSE(ip1 != ip1);
    EXPECT_FALSE(ip1 < ip1);
    EXPECT_TRUE(ip1 <= ip1);
    EXPECT_FALSE(ip1 > ip1);
    EXPECT_TRUE(ip1 >= ip1);

    // Symmetry
    EXPECT_EQ(ip1 == ip3, ip3 == ip1);
    EXPECT_EQ(ip1 != ip2, ip2 != ip1);

    // Transitivity
    if (const IPAddress ip4(5, 15); ip1 < ip4 && ip4 < ip2) {
        EXPECT_TRUE(ip1 < ip2);
    }
}

// =============== Edge tests ===============
TEST(IPAddressEdge, BoundaryValues) {
    const IPAddress ipMin(0, 0);
    EXPECT_EQ(ipMin.getRawAddress(), 0);

    const IPAddress ipMax(255, 255);
    EXPECT_EQ(ipMax.getRawAddress(), 0xFFFF);
    EXPECT_EQ(ipMax.getRouterIP(), 255);
    EXPECT_EQ(ipMax.getTerminalIP(), 255);
}

TEST(IPAddressEdge, EdgeCaseRouterIDs) {
    EXPECT_EQ(IPAddress(0, 100).getRouterIP(), 0);
    EXPECT_EQ(IPAddress(1, 100).getRouterIP(), 1);
    EXPECT_EQ(IPAddress(254, 100).getRouterIP(), 254);
    EXPECT_EQ(IPAddress(255, 100).getRouterIP(), 255);
}

TEST(IPAddressEdge, EdgeCaseTerminalIDs) {
    EXPECT_EQ(IPAddress(100, 0).getTerminalIP(), 0);
    EXPECT_EQ(IPAddress(100, 1).getTerminalIP(), 1);
    EXPECT_EQ(IPAddress(100, 254).getTerminalIP(), 254);
    EXPECT_EQ(IPAddress(100, 255).getTerminalIP(), 255);
}

// =============== Constexpr tests ===============
TEST(IPAddressConstexpr, ConstexprConstructor) {
    constexpr IPAddress ip(10, 20);

    static_assert(ip.getRouterIP() == 10, "constexpr getRouterIP failed");
    static_assert(ip.getTerminalIP() == 20, "constexpr getTerminalIP failed");
}

TEST(IPAddressConstexpr, ConstexprOperations) {
    constexpr IPAddress ip1(10, 20);
    constexpr IPAddress ip2(10, 20);
    constexpr IPAddress ip3(10, 21);

    static_assert(ip1 == ip2, "constexpr equality failed");
    static_assert(ip1 != ip3, "constexpr inequality failed");
    static_assert(ip1 < ip3, "constexpr less than failed");
}

TEST(IPAddressConstexpr, ConstexprIsRouter) {
    constexpr IPAddress router(10, 0);
    constexpr IPAddress terminal(10, 1);

    static_assert(router.isRouter(), "constexpr isRouter failed");
    static_assert(!terminal.isRouter(), "constexpr isRouter failed");
}

// =============== Complex tests ===============
TEST(IPAddressComplex, NetworkScenario) {
    const IPAddress router1(1, 0);
    const IPAddress router2(2, 0);
    const IPAddress terminal1(1, 10);
    const IPAddress terminal2(1, 20);
    const IPAddress terminal3(2, 5);

    EXPECT_EQ(terminal1.getRouterIP(), router1.getRouterIP());
    EXPECT_EQ(terminal2.getRouterIP(), router1.getRouterIP());
    EXPECT_EQ(terminal3.getRouterIP(), router2.getRouterIP());

    EXPECT_TRUE(router1 < router2);
    EXPECT_TRUE(terminal1 < terminal2);
    EXPECT_TRUE(terminal2 < terminal3);
}

TEST(IPAddressComplex, SortingAddresses) {
    std::vector addresses = {IPAddress(5, 100), IPAddress(3, 50), IPAddress(5, 10), IPAddress(3, 200),
                             IPAddress(10, 0)};

    std::sort(addresses.begin(), addresses.end());

    EXPECT_EQ(addresses[0], IPAddress(3, 50));
    EXPECT_EQ(addresses[1], IPAddress(3, 200));
    EXPECT_EQ(addresses[2], IPAddress(5, 10));
    EXPECT_EQ(addresses[3], IPAddress(5, 100));
    EXPECT_EQ(addresses[4], IPAddress(10, 0));
}

TEST(IPAddressComplex, UseInMap) {
    std::map<IPAddress, std::string> routingTable;

    routingTable[IPAddress(1, 10)] = "Device A";
    routingTable[IPAddress(1, 20)] = "Device B";
    routingTable[IPAddress(2, 0)]  = "Router 2";

    EXPECT_EQ(routingTable[IPAddress(1, 10)], "Device A");
    EXPECT_EQ(routingTable[IPAddress(2, 0)], "Router 2");
    EXPECT_EQ(routingTable.size(), 3);
}

TEST(IPAddressComplex, UseInSet) {
    std::set<IPAddress> uniqueAddresses;

    uniqueAddresses.insert(IPAddress(10, 20));
    uniqueAddresses.insert(IPAddress(10, 20));
    uniqueAddresses.insert(IPAddress(10, 21));

    EXPECT_EQ(uniqueAddresses.size(), 2);
    EXPECT_TRUE(uniqueAddresses.count(IPAddress(10, 20)) == 1);
}

// =============== Bitwise tests ===============
TEST(IPAddressBitwise, BitwiseRepresentation) {
    const IPAddress ip(0xAB, 0xCD);

    EXPECT_EQ(ip.getRawAddress(), 0xABCD);
    EXPECT_EQ(ip.getRouterIP(), 0xAB);
    EXPECT_EQ(ip.getTerminalIP(), 0xCD);
}

TEST(IPAddressBitwise, AllBitsSet) {
    const IPAddress ip(0xFF, 0xFF);

    EXPECT_EQ(ip.getRawAddress(), 0xFFFF);
    EXPECT_EQ(ip.getRouterIP(), 255);
    EXPECT_EQ(ip.getTerminalIP(), 255);
}

TEST(IPAddressBitwise, AlternatingBits) {
    const IPAddress ip(0xAA, 0x55);

    EXPECT_EQ(ip.getRouterIP(), 170);
    EXPECT_EQ(ip.getTerminalIP(), 85);
    EXPECT_EQ(ip.getRawAddress(), 0xAA55);
}
