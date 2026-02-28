#include <gtest/gtest.h>
#include <algorithm>
#include <queue>
#include <sstream>
#include "core/Packet.h"

class PacketTest : public testing::Test {
protected:
    const IPAddress src{20, 15};
    const IPAddress dst{10, 5};
    static constexpr size_t TICK = 10;
};

// =============== Constructors tests ===============
TEST_F(PacketTest, Constructor_Valid) {
    const Packet packet(100, 5, 10, src, dst, TICK);

    EXPECT_EQ(packet.getPageID(), 100);
    EXPECT_EQ(packet.getPagePos(), 5);
    EXPECT_EQ(packet.getPageLen(), 10);
    EXPECT_EQ(packet.getSrcIP(), src);
    EXPECT_EQ(packet.getDstIP(), dst);
    EXPECT_EQ(packet.getTimeout(), TICK);
}

TEST_F(PacketTest, Constructor_InvalidPagePos) {
    // pagePos >= pageLen
    EXPECT_THROW(Packet(100, 10, 10, src, dst, TICK), std::invalid_argument);
    EXPECT_THROW(Packet(100, 15, 10, src, dst, TICK), std::invalid_argument);
}

TEST_F(PacketTest, Constructor_InvalidDstIP) {
    const IPAddress invalidDst(0, 0);

    EXPECT_THROW(Packet(100, 0, 10, src, invalidDst, TICK), std::invalid_argument);
}

TEST_F(PacketTest, Constructor_InvalidSrcIP) {
    const IPAddress invalidSrc(0, 0);

    EXPECT_THROW(Packet(100, 0, 10, invalidSrc, dst, TICK), std::invalid_argument);
}

TEST_F(PacketTest, Constructor_Copy) {
    const Packet p1(100, 5, 10, src, dst, TICK);
    const Packet p2(p1);

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePos(), 5);
    EXPECT_EQ(p2.getPageLen(), 10);
    EXPECT_EQ(p1, p2);
}

TEST_F(PacketTest, Constructor_Move) {
    Packet p1(100, 5, 10, src, dst, TICK);
    const Packet p2(std::move(p1));

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePos(), 5);
}

TEST_F(PacketTest, Assignment_Copy) {
    const Packet p1(100, 5, 10, src, dst, TICK);
    Packet p2(200, 0, 5, src, dst, TICK);

    p2 = p1;

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePos(), 5);
    EXPECT_EQ(p1, p2);
}

TEST_F(PacketTest, Assignment_Move) {

    Packet p1(100, 5, 10, src, dst, TICK);
    Packet p2(200, 0, 5, src, dst, TICK);

    p2 = std::move(p1);

    EXPECT_EQ(p2.getPageID(), 100);
    EXPECT_EQ(p2.getPagePos(), 5);
}

// =============== Getters tests ===============
TEST_F(PacketTest, Getters_All) {
    const Packet packet(12345, 7, 20, src, dst, TICK);

    EXPECT_EQ(packet.getPageID(), 12345);
    EXPECT_EQ(packet.getPagePos(), 7);
    EXPECT_EQ(packet.getPageLen(), 20);
    EXPECT_EQ(packet.getDstIP(), dst);
    EXPECT_EQ(packet.getSrcIP(), src);
    EXPECT_EQ(packet.getTimeout(), TICK);
}

// =============== Query tests ===============
TEST_F(PacketTest, Query_IsFirstPacket) {
    const Packet first(100, 0, 10, src, dst, TICK);
    const Packet middle(100, 5, 10, src, dst, TICK);
    const Packet last(100, 9, 10, src, dst, TICK);

    EXPECT_TRUE(first.isFirstPacket());
    EXPECT_FALSE(middle.isFirstPacket());
    EXPECT_FALSE(last.isFirstPacket());
}

TEST_F(PacketTest, Query_IsLastPacket) {
    const Packet first(100, 0, 10, src, dst, TICK);
    const Packet middle(100, 5, 10, src, dst, TICK);
    const Packet last(100, 9, 10, src, dst, TICK);

    EXPECT_FALSE(first.isLastPacket());
    EXPECT_FALSE(middle.isLastPacket());
    EXPECT_TRUE(last.isLastPacket());
}

// =============== Utilities tests ===============
TEST_F(PacketTest, ToString_Basic) {
    const Packet packet(123, 4, 10, src, dst, TICK);

    EXPECT_EQ(packet.toString(), "Src: 020.015 -> Dst: 010.005 | ID: 000123-4/10");
}

TEST_F(PacketTest, ToString_LargePageID) {
    const Packet packet(654321, 99, 100, src, dst, TICK);

    EXPECT_EQ(packet.toString(), "Src: 020.015 -> Dst: 010.005 | ID: 654321-99/100");
}

TEST_F(PacketTest, ToString_SmallPageID) {
    const Packet packet(7, 0, 5, src, dst, TICK);

    EXPECT_EQ(packet.toString(), "Src: 020.015 -> Dst: 010.005 | ID: 000007-0/5");
}

TEST_F(PacketTest, StreamOperator_Basic) {

    const Packet packet(999, 5, 10, src, dst, TICK);

    std::ostringstream oss;
    oss << packet;

    EXPECT_EQ(oss.str(), "Src: 020.015 -> Dst: 010.005 | ID: 000999-5/10");
}

TEST_F(PacketTest, StreamOperator_Multiple) {
    const Packet p1(10, 0, 5, src, dst, TICK);
    const Packet p2(20, 1, 5, src, dst, TICK);

    std::ostringstream oss;
    oss << p1 << " | " << p2;

    EXPECT_EQ(oss.str(),
              "Src: 020.015 -> Dst: 010.005 | ID: 000010-0/5 | Src: 020.015 -> Dst: 010.005 | ID: "
              "000020-1/5");
}

// =============== Comparison tests ===============
TEST_F(PacketTest, Operator_Equality) {
    const Packet p1(100, 5, 10, src, dst, TICK);
    const Packet p2(100, 5, 10, src, dst, TICK);
    const Packet p3(100, 6, 10, src, dst, TICK);  // Different position
    const Packet p4(101, 5, 10, src, dst, TICK);  // Different pageID

    EXPECT_TRUE(p1 == p2);   // Same pageID and position
    EXPECT_FALSE(p1 == p3);  // Different position
    EXPECT_FALSE(p1 == p4);  // Different pageID
}

TEST_F(PacketTest, Operator_Inequality) {
    const Packet p1(100, 5, 10, src, dst, TICK);
    const Packet p2(100, 5, 10, src, dst, TICK);
    const Packet p3(100, 6, 10, src, dst, TICK);

    EXPECT_FALSE(p1 != p2);
    EXPECT_TRUE(p2 != p3);
    EXPECT_TRUE(p1 != p3);
}

// =============== Complex tests ===============
TEST_F(PacketTest, Page_Transmission) {

    // Simulate a page of 5 packets
    std::vector<Packet> page;
    for (int i = 0; i < 5; i++) {
        page.emplace_back(42, i, 5, src, dst, TICK);
    }

    EXPECT_EQ(page.size(), 5);
    EXPECT_TRUE(page[0].isFirstPacket());
    EXPECT_TRUE(page[4].isLastPacket());

    for (const auto& packet : page) {
        EXPECT_EQ(packet.getPageID(), 42);
        EXPECT_EQ(packet.getPageLen(), 5);
    }
}

TEST_F(PacketTest, Page_Multiple) {
    // Different pages
    const Packet page1_packet0(1, 0, 5, src, dst, TICK);
    const Packet page1_packet1(1, 1, 5, src, dst, TICK);
    const Packet page2_packet0(2, 0, 3, src, dst, TICK);

    EXPECT_NE(page1_packet0, page2_packet0);  // Different pages
    EXPECT_NE(page1_packet0, page1_packet1);  // Same page, different position
}

TEST_F(PacketTest, Page_Positions) {
    // First position
    const Packet first(100, 0, 100, src, dst, TICK);
    EXPECT_EQ(first.getPagePos(), 0);
    EXPECT_TRUE(first.isFirstPacket());
    EXPECT_FALSE(first.isLastPacket());

    // Last position
    const Packet last(100, 99, 100, src, dst, TICK);
    EXPECT_EQ(last.getPagePos(), 99);
    EXPECT_FALSE(last.isFirstPacket());
    EXPECT_TRUE(last.isLastPacket());
}
