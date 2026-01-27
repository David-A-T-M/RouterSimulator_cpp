#include <gtest/gtest.h>
#include <algorithm>
#include <queue>
#include <set>
#include <sstream>
#include "core/Packet.h"

// =============== Constructors tests ===============
TEST(PacketConstructors, ValidConstructor) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet packet(100, 5, 10, 3, dest, origin);

    EXPECT_EQ(packet.getPageID(), 100);
    EXPECT_EQ(packet.getPagePosition(), 5);
    EXPECT_EQ(packet.getPageLength(), 10);
    EXPECT_EQ(packet.getRouterPriority(), 3);
    EXPECT_EQ(packet.getDestinationIP(), dest);
    EXPECT_EQ(packet.getOriginIP(), origin);
}

TEST(PacketConstructors, ConstructorInvalidPageID) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    EXPECT_THROW(Packet(-1, 0, 10, 0, dest, origin), std::invalid_argument);
}

TEST(PacketConstructors, ConstructorInvalidPageLength) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    EXPECT_THROW(Packet(100, 0, 0, 0, dest, origin), std::invalid_argument);
    EXPECT_THROW(Packet(100, 0, -5, 0, dest, origin), std::invalid_argument);
}

TEST(PacketConstructors, ConstructorInvalidPagePosition) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    // Position < 0
    EXPECT_THROW(Packet(100, -1, 10, 0, dest, origin), std::invalid_argument);

    // Position >= length
    EXPECT_THROW(Packet(100, 10, 10, 0, dest, origin), std::invalid_argument);
    EXPECT_THROW(Packet(100, 15, 10, 0, dest, origin), std::invalid_argument);
}

TEST(PacketConstructors, ConstructorInvalidDestinationIP) {
    const IPAddress invalidDest(0, 0);
    const IPAddress validOrigin(20, 15);

    EXPECT_THROW(Packet(100, 0, 10, 0, invalidDest, validOrigin), std::invalid_argument);
}

TEST(PacketConstructors, ConstructorInvalidOriginIP) {
    const IPAddress validDest(10, 5);
    const IPAddress invalidOrigin(0, 0);

    EXPECT_THROW(Packet(100, 0, 10, 0, validDest, invalidOrigin), std::invalid_argument);
}

TEST(PacketConstructors, CopyConstructor) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 5, 10, 3, dest, origin);
    const Packet p2(p1);

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePosition(), 5);
    EXPECT_EQ(p2.getPageLength(), 10);
    EXPECT_EQ(p2.getRouterPriority(), 3);
    EXPECT_EQ(p1, p2);
}

TEST(PacketConstructors, MoveConstructor) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    Packet p1(100, 5, 10, 3, dest, origin);
    const Packet p2(std::move(p1));

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePosition(), 5);
    EXPECT_EQ(p2.getRouterPriority(), 3);
}

TEST(PacketConstructors, CopyAssignment) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 5, 10, 3, dest, origin);
    Packet p2(200, 0, 5, 1, dest, origin);

    p2 = p1;

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePosition(), 5);
    EXPECT_EQ(p1, p2);
}

TEST(PacketConstructors, MoveAssignment) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    Packet p1(100, 5, 10, 3, dest, origin);
    Packet p2(200, 0, 5, 1, dest, origin);

    p2 = std::move(p1);

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePosition(), 5);
}

// =============== Setters tests ===============
TEST(PacketSetters, SetRouterPriority) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    Packet packet(100, 0, 10, 0, dest, origin);

    EXPECT_EQ(packet.getRouterPriority(), 0);

    packet.setRouterPriority(5);
    EXPECT_EQ(packet.getRouterPriority(), 5);

    packet.setRouterPriority(-1);
    EXPECT_EQ(packet.getRouterPriority(), -1);
}

// =============== Getters tests ===============
TEST(PacketGetters, GettersBasic) {
    const IPAddress dest(192, 168);
    const IPAddress origin(10, 1);

    const Packet packet(12345, 7, 20, 8, dest, origin);

    EXPECT_EQ(packet.getPageID(), 12345);
    EXPECT_EQ(packet.getPagePosition(), 7);
    EXPECT_EQ(packet.getPageLength(), 20);
    EXPECT_EQ(packet.getRouterPriority(), 8);
    EXPECT_EQ(packet.getDestinationIP(), dest);
    EXPECT_EQ(packet.getOriginIP(), origin);
}

// =============== Query tests ===============
TEST(PacketQuery, IsFirstPacket) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet first(100, 0, 10, 0, dest, origin);
    const Packet middle(100, 5, 10, 0, dest, origin);
    const Packet last(100, 9, 10, 0, dest, origin);

    EXPECT_TRUE(first.isFirstPacket());
    EXPECT_FALSE(middle.isFirstPacket());
    EXPECT_FALSE(last.isFirstPacket());
}

TEST(PacketQuery, IsLastPacket) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet first(100, 0, 10, 0, dest, origin);
    const Packet middle(100, 5, 10, 0, dest, origin);
    const Packet last(100, 9, 10, 0, dest, origin);

    EXPECT_FALSE(first.isLastPacket());
    EXPECT_FALSE(middle.isLastPacket());
    EXPECT_TRUE(last.isLastPacket());
}

// =============== Utilities tests ===============
TEST(PacketUtilities, ToString) {
    const IPAddress dest(5, 100);
    const IPAddress origin(10, 50);

    const Packet packet(123, 4, 10, 0, dest, origin);

    // Format: Dest: RouterIP - ID: PageID-Position
    EXPECT_EQ(packet.toString(), "Dest: 5.100 - ID: 000000123-4");
}

TEST(PacketUtilities, ToStringLargePageID) {
    const IPAddress dest(255, 1);
    const IPAddress origin(1, 1);

    const Packet packet(987654321, 99, 100, 0, dest, origin);

    EXPECT_EQ(packet.toString(), "Dest: 255.1 - ID: 987654321-99");
}

TEST(PacketUtilities, ToStringSmallPageID) {
    const IPAddress dest(1, 1);
    const IPAddress origin(2, 2);

    const Packet packet(7, 0, 5, 0, dest, origin);

    EXPECT_EQ(packet.toString(), "Dest: 1.1 - ID: 000000007-0");
}

TEST(PacketUtilities, StreamOperator) {
    const IPAddress dest(42, 10);
    const IPAddress origin(1, 1);

    const Packet packet(999, 5, 10, 0, dest, origin);

    std::ostringstream oss;
    oss << packet;

    EXPECT_EQ(oss.str(), "Dest: 42.10 - ID: 000000999-5");
}

TEST(PacketUtilities, StreamOperatorMultiple) {
    const IPAddress dest(1, 1);
    const IPAddress origin(2, 2);

    const Packet p1(10, 0, 5, 0, dest, origin);
    const Packet p2(20, 1, 5, 0, dest, origin);

    std::ostringstream oss;
    oss << p1 << " | " << p2;

    EXPECT_EQ(oss.str(), "Dest: 1.1 - ID: 000000010-0 | Dest: 1.1 - ID: 000000020-1");
}

// =============== Comparison tests ===============
TEST(PacketComparison, EqualityOperator) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 5, 10, 3, dest, origin);
    const Packet p2(100, 5, 10, 7, dest, origin);  // Different priority
    const Packet p3(100, 6, 10, 3, dest, origin);  // Different position
    const Packet p4(101, 5, 10, 3, dest, origin);  // Different pageID

    EXPECT_TRUE(p1 == p2);   // Same pageID and position
    EXPECT_FALSE(p1 == p3);  // Different position
    EXPECT_FALSE(p1 == p4);  // Different pageID
}

TEST(PacketComparison, InequalityOperator) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 5, 10, 0, dest, origin);
    const Packet p2(100, 5, 10, 0, dest, origin);
    const Packet p3(100, 6, 10, 0, dest, origin);

    EXPECT_FALSE(p1 != p2);
    EXPECT_TRUE(p2 != p3);
    EXPECT_TRUE(p1 != p3);
}

TEST(PacketComparison, LessThanByPriority) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet highPri(100, 0, 10, 10, dest, origin);
    const Packet lowPri(100, 0, 10, 5, dest, origin);

    // Lower priority comes FIRST (so lowPri < highPri is TRUE)
    EXPECT_TRUE(lowPri < highPri);
    EXPECT_FALSE(highPri < lowPri);
}

TEST(PacketComparison, LessThanByPageID) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 0, 10, 5, dest, origin);
    const Packet p2(200, 0, 10, 5, dest, origin);

    EXPECT_TRUE(p1 < p2);
    EXPECT_FALSE(p2 < p1);
}

TEST(PacketComparison, LessThanByPosition) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 3, 10, 5, dest, origin);
    const Packet p2(100, 7, 10, 5, dest, origin);

    EXPECT_TRUE(p1 < p2);
    EXPECT_FALSE(p2 < p1);
}

TEST(PacketComparison, LessThanCombined) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    // Priority is most important
    const Packet highPri(200, 9, 10, 10, dest, origin);
    const Packet lowPri(100, 0, 10, 5, dest, origin);

    EXPECT_TRUE(lowPri < highPri);
}

TEST(PacketComparison, OtherComparisonOperators) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    const Packet p1(100, 3, 10, 5, dest, origin);
    const Packet p2(100, 7, 10, 5, dest, origin);

    EXPECT_TRUE(p1 <= p2);
    EXPECT_FALSE(p1 > p2);
    EXPECT_FALSE(p1 >= p2);
    EXPECT_TRUE(p2 > p1);
    EXPECT_TRUE(p2 >= p1);
}

// =============== STL containers usage tests ===============
TEST(PacketSTL, UseInSet) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    std::set<Packet> packets;

    packets.insert(Packet(100, 0, 10, 5, dest, origin));
    packets.insert(Packet(100, 1, 10, 5, dest, origin));
    packets.insert(Packet(100, 0, 10, 5, dest, origin));  // Duplicate

    EXPECT_EQ(packets.size(), 2);
}

TEST(PacketSTL, UseInPriorityQueue) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    std::priority_queue<Packet> pq;

    pq.emplace(100, 0, 10, 3, dest, origin);
    pq.emplace(100, 1, 10, 8, dest, origin);  // Higher priority
    pq.emplace(100, 2, 10, 1, dest, origin);

    // Should get highest priority first
    EXPECT_EQ(pq.top().getRouterPriority(), 8);
    pq.pop();
    EXPECT_EQ(pq.top().getRouterPriority(), 3);
    pq.pop();
    EXPECT_EQ(pq.top().getRouterPriority(), 1);
}

TEST(PacketSTL, Sorting) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    std::vector<Packet> packets;
    packets.emplace_back(100, 0, 10, 3, dest, origin);
    packets.emplace_back(100, 1, 10, 8, dest, origin);
    packets.emplace_back(100, 2, 10, 1, dest, origin);
    packets.emplace_back(100, 3, 10, 8, dest, origin);  // Same pri as [1]

    std::sort(packets.begin(), packets.end());

    // Should be sorted by priority (asc), then position
    EXPECT_EQ(packets[0].getRouterPriority(), 1);
    EXPECT_EQ(packets[1].getRouterPriority(), 3);
    EXPECT_EQ(packets[2].getRouterPriority(), 8);
    EXPECT_EQ(packets[2].getPagePosition(), 1);
    EXPECT_EQ(packets[3].getRouterPriority(), 8);
    EXPECT_EQ(packets[3].getPagePosition(), 3);
}

// =============== Complex tests ===============
TEST(PacketComplex, PageTransmission) {
    const IPAddress dest(5, 100);
    const IPAddress origin(10, 50);

    // Simulate a page of 5 packets
    std::vector<Packet> page;
    for (int i = 0; i < 5; i++) {
        page.emplace_back(42, i, 5, 0, dest, origin);
    }

    EXPECT_EQ(page.size(), 5);
    EXPECT_TRUE(page[0].isFirstPacket());
    EXPECT_TRUE(page[4].isLastPacket());

    for (const auto& packet : page) {
        EXPECT_EQ(packet.getPageID(), 42);
        EXPECT_EQ(packet.getPageLength(), 5);
    }
}

TEST(PacketComplex, RouterPriorityUpdate) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    Packet packet(100, 0, 10, 0, dest, origin);

    // Simulate router assigning priority
    packet.setRouterPriority(3);
    EXPECT_EQ(packet.getRouterPriority(), 3);

    // Another router changes priority
    packet.setRouterPriority(7);
    EXPECT_EQ(packet.getRouterPriority(), 7);
}

TEST(PacketComplex, MultiplePages) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    // Different pages
    const Packet page1_packet0(1, 0, 5, 0, dest, origin);
    const Packet page1_packet1(1, 1, 5, 0, dest, origin);
    const Packet page2_packet0(2, 0, 3, 0, dest, origin);

    EXPECT_NE(page1_packet0, page2_packet0);  // Different pages
    EXPECT_NE(page1_packet0, page1_packet1);  // Same page, different position
}

TEST(PacketComplex, BoundaryPagePositions) {
    const IPAddress dest(10, 5);
    const IPAddress origin(20, 15);

    // First position
    const Packet first(100, 0, 100, 0, dest, origin);
    EXPECT_EQ(first.getPagePosition(), 0);
    EXPECT_TRUE(first.isFirstPacket());
    EXPECT_FALSE(first.isLastPacket());

    // Last position
    const Packet last(100, 99, 100, 0, dest, origin);
    EXPECT_EQ(last.getPagePosition(), 99);
    EXPECT_FALSE(last.isFirstPacket());
    EXPECT_TRUE(last.isLastPacket());
}
