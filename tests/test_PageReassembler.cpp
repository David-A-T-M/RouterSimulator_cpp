#include <gtest/gtest.h>
#include "core/Page.h"
#include "core/PageReassembler.h"

class TestPageReassembler : public ::testing::Test {
protected:
    const IPAddress src{20, 15};
    const IPAddress dst{10, 5};
    static constexpr size_t TICK = 100;
    PageReassembler reassembler{100, 10, TICK};
};

// ===============  Constructor tests ===============
TEST_F(TestPageReassembler, Constructor_Valid) {
    EXPECT_EQ(reassembler.pageID, 100);
    EXPECT_EQ(reassembler.total, 10);
    EXPECT_EQ(reassembler.count, 0);
    EXPECT_FALSE(reassembler.isComplete());
}

TEST_F(TestPageReassembler, Constructor_Move) {
    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 1, 10, src, dst, TICK));

    const PageReassembler moved(std::move(reassembler));

    EXPECT_EQ(moved.pageID, 100);
    EXPECT_EQ(moved.total, 10);
    EXPECT_EQ(moved.count, 2);
    EXPECT_NE(moved.packets, nullptr);
    EXPECT_TRUE(moved.hasPacketAt(0));
    EXPECT_TRUE(moved.hasPacketAt(1));
    EXPECT_FALSE(moved.isComplete());

    EXPECT_EQ(reassembler.packets, nullptr);
}

TEST_F(TestPageReassembler, Assignment_Move) {
    PageReassembler otherPR(2, 10, TICK);

    reassembler = std::move(otherPR);

    EXPECT_EQ(reassembler.pageID, 2);
    EXPECT_EQ(reassembler.total, 10);

    EXPECT_EQ(otherPR.packets, nullptr);
}

TEST_F(TestPageReassembler, Move_SelfAssignment) {
    PageReassembler& ref = reassembler;
    EXPECT_NO_THROW(reassembler = std::move(ref));

    EXPECT_EQ(reassembler.pageID, 100);
}

// =============== Getters tests ===============
TEST_F(TestPageReassembler, Getter_CompletionRate) {
    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.0);

    for (int i = 0; i < 5; ++i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.5);

    for (int i = 5; i < 10; ++i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 1.0);
}

TEST_F(TestPageReassembler, Getter_RemainingPackets) {
    EXPECT_EQ(reassembler.getRemainingPackets(), 10);

    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    EXPECT_EQ(reassembler.getRemainingPackets(), 9);

    reassembler.addPacket(Packet(100, 5, 10, src, dst, TICK));
    EXPECT_EQ(reassembler.getRemainingPackets(), 8);
}

// =============== Query tests ===============
TEST_F(TestPageReassembler, IsComplete_Empty) {
    EXPECT_FALSE(reassembler.isComplete());
}

TEST_F(TestPageReassembler, IsComplete_Partial) {
    reassembler.addPacket(Packet(100, 0, 5, src, dst, TICK));
    reassembler.addPacket(Packet(100, 1, 5, src, dst, TICK));

    EXPECT_FALSE(reassembler.isComplete());
}

TEST_F(TestPageReassembler, IsComplete_Full) {
    for (int i = 0; i < 10; ++i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_TRUE(reassembler.isComplete());
}

TEST_F(TestPageReassembler, HasPacketAt_Valid) {
    EXPECT_FALSE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(2));

    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 2, 10, src, dst, TICK));

    EXPECT_TRUE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(1));
    EXPECT_TRUE(reassembler.hasPacketAt(2));
    EXPECT_THROW((void)reassembler.hasPacketAt(10), std::out_of_range);
}

TEST_F(TestPageReassembler, HasPacket_InvalidPos) {
    EXPECT_THROW((void)reassembler.hasPacketAt(10), std::out_of_range);
}

// =============== Modifiers tests ===============
TEST_F(TestPageReassembler, AddPacket_Ordered) {
    PageReassembler otherPR(100, 3, TICK);
    const Packet p0(100, 0, 3, src, dst, TICK);
    const Packet p1(100, 1, 3, src, dst, TICK);
    const Packet p2(100, 2, 3, src, dst, TICK);

    EXPECT_TRUE(otherPR.addPacket(p0));
    EXPECT_EQ(otherPR.count, 1);

    EXPECT_TRUE(otherPR.addPacket(p1));
    EXPECT_EQ(otherPR.count, 2);
    EXPECT_TRUE(otherPR.addPacket(p2));
    EXPECT_EQ(otherPR.count, 3);

    EXPECT_TRUE(otherPR.isComplete());
}

TEST_F(TestPageReassembler, AddPacket_Unordered) {
    const int arrivalOrder[] = {3, 7, 1, 9, 0, 5, 2, 8, 4, 6};

    for (int i = 0; i < 10; ++i) {
        const int pos = arrivalOrder[i];
        Packet p(100, pos, 10, src, dst, TICK);

        EXPECT_TRUE(reassembler.addPacket(p));
        EXPECT_EQ(reassembler.count, i + 1);
    }

    EXPECT_TRUE(reassembler.isComplete());
}

TEST_F(TestPageReassembler, AddPacket_WrongPageID) {
    const Packet wrongPacket(200, 0, 10, src, dst, TICK);

    EXPECT_FALSE(reassembler.addPacket(wrongPacket));
    EXPECT_EQ(reassembler.count, 0);
}

TEST_F(TestPageReassembler, AddPacket_WrongPageLen) {
    const Packet wrongPacket(100, 0, 5, src, dst, TICK);

    EXPECT_FALSE(reassembler.addPacket(wrongPacket));
    EXPECT_EQ(reassembler.count, 0);
}

TEST_F(TestPageReassembler, AddPacket_Duplicate) {
    const Packet p0(100, 0, 10, src, dst, TICK);

    EXPECT_TRUE(reassembler.addPacket(p0));
    EXPECT_EQ(reassembler.count, 1);

    EXPECT_FALSE(reassembler.addPacket(p0));
    EXPECT_EQ(reassembler.count, 1);
}

TEST_F(TestPageReassembler, Package_Complete) {
    // Add packets in any order
    for (int i = 9; i >= 0; --i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> packetList = reassembler.package();

    // Expect packets in order
    EXPECT_EQ(packetList.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(packetList[i].getPagePos(), i);
        EXPECT_EQ(packetList[i].getPageID(), 100);
    }

    EXPECT_EQ(reassembler.count, 0);
}

TEST_F(TestPageReassembler, Package_Incomplete) {
    reassembler.addPacket(Packet(100, 0, 5, src, dst, TICK));
    reassembler.addPacket(Packet(100, 1, 5, src, dst, TICK));
    reassembler.addPacket(Packet(100, 2, 5, src, dst, TICK));

    EXPECT_FALSE(reassembler.isComplete());

    EXPECT_THROW(reassembler.package(), std::runtime_error);
}

TEST_F(TestPageReassembler, Package_SinglePacket) {
    PageReassembler otherPR(100, 1, TICK);
    otherPR.addPacket(Packet(100, 0, 1, src, dst, TICK));

    EXPECT_TRUE(otherPR.isComplete());

    List<Packet> packetList = otherPR.package();

    EXPECT_EQ(packetList.size(), 1);
    EXPECT_EQ(packetList[0].getPagePos(), 0);
}

TEST_F(TestPageReassembler, Package_PreservesPacketData) {
    for (int i = 0; i < 10; ++i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    List<Packet> packetList = reassembler.package();

    EXPECT_EQ(packetList[0].getSrcIP(), src);
    EXPECT_EQ(packetList[0].getDstIP(), dst);
}

TEST_F(TestPageReassembler, Reset_) {
    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 2, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 4, 10, src, dst, TICK));

    EXPECT_EQ(reassembler.count, 3);

    reassembler.reset();

    EXPECT_EQ(reassembler.count, 0);
    EXPECT_FALSE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(2));
    EXPECT_FALSE(reassembler.hasPacketAt(4));
}

TEST_F(TestPageReassembler, Reset_Reuse) {
    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 1, 10, src, dst, TICK));

    reassembler.reset();

    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(reassembler.addPacket(Packet(100, i, 10, src, dst, TICK)));
    }

    EXPECT_TRUE(reassembler.isComplete());
}

// =============== Complex tests ===============
TEST_F(TestPageReassembler, SimulateNetworkTransmission) {
    const int arrivalOrder[] = {3, 7, 1, 9, 0, 5, 2, 8, 4, 6};

    for (int i = 0; i < 10; ++i) {
        const int pos = arrivalOrder[i];
        Packet p(100, pos, 10, src, dst, TICK);

        EXPECT_TRUE(reassembler.addPacket(p));
        EXPECT_EQ(reassembler.count, i + 1);
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> orderedPackets = reassembler.package();

    EXPECT_EQ(orderedPackets.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(orderedPackets[i].getPagePos(), i);
    }
}

TEST_F(TestPageReassembler, SimulatePacketLoss) {
    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 2, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 5, 10, src, dst, TICK));
    reassembler.addPacket(Packet(100, 8, 10, src, dst, TICK));

    EXPECT_EQ(reassembler.count, 4);
    EXPECT_FALSE(reassembler.isComplete());
    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.4);

    EXPECT_THROW(reassembler.package(), std::runtime_error);
}

TEST_F(TestPageReassembler, SimulateRetransmission) {
    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));

    EXPECT_FALSE(reassembler.addPacket(Packet(100, 0, 3, src, dst, TICK)));

    EXPECT_EQ(reassembler.count, 1);
}

TEST_F(TestPageReassembler, MultiplePages) {
    PageReassembler reassembler2(200, 3, TICK);
    PageReassembler reassembler3(300, 7, TICK);

    reassembler.addPacket(Packet(100, 0, 10, src, dst, TICK));
    reassembler2.addPacket(Packet(200, 0, 3, src, dst, TICK));
    reassembler3.addPacket(Packet(300, 0, 7, src, dst, TICK));

    EXPECT_EQ(reassembler.count, 1);
    EXPECT_EQ(reassembler2.count, 1);
    EXPECT_EQ(reassembler3.count, 1);

    EXPECT_FALSE(reassembler.addPacket(Packet(200, 1, 5, src, dst, TICK)));
}

TEST_F(TestPageReassembler, Integration_Page) {
    for (int i = 0; i < 10; ++i) {
        reassembler.addPacket(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> packets = reassembler.package();

    const Page reconstructedPage(std::move(packets));

    EXPECT_EQ(reconstructedPage.getPageID(), 100);
    EXPECT_EQ(reconstructedPage.getPageLen(), 10);
    EXPECT_EQ(reconstructedPage.getSrcIP(), src);
    EXPECT_EQ(reconstructedPage.getDstIP(), dst);
}
