#include <gtest/gtest.h>
#include "core/Page.h"
#include "core/PageReassembler.h"

// ===============  Constructor tests ===============
TEST(PageReassemblerConstructor, ValidConstructor) {
    const PageReassembler reassembler(100, 10);

    EXPECT_EQ(reassembler.pageID, 100);
    EXPECT_EQ(reassembler.expectedPackets, 10);
    EXPECT_EQ(reassembler.currentPackets, 0);
    EXPECT_FALSE(reassembler.isComplete());
}

// =============== Getters tests ===============
TEST(PageReassemblerGetters, GetCompletionRate) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 10);

    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.0);

    for (int i = 0; i < 5; ++i) {
        reassembler.addPacket(Packet(100, i, 10, 0, dest, origin));
    }

    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.5);

    for (int i = 5; i < 10; ++i) {
        reassembler.addPacket(Packet(100, i, 10, 0, dest, origin));
    }

    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 1.0);
}

TEST(PageReassemblerGetters, GetRemainingPackets) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 10);

    EXPECT_EQ(reassembler.getRemainingPackets(), 10);

    reassembler.addPacket(Packet(100, 0, 10, 0, dest, origin));
    EXPECT_EQ(reassembler.getRemainingPackets(), 9);

    reassembler.addPacket(Packet(100, 5, 10, 0, dest, origin));
    EXPECT_EQ(reassembler.getRemainingPackets(), 8);
}

// =============== Query tests ===============
TEST(PageReassemblerQuery, IsCompleteEmpty) {
    const PageReassembler reassembler(100, 5);
    EXPECT_FALSE(reassembler.isComplete());
}

TEST(PageReassemblerQuery, IsCompletePartial) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    reassembler.addPacket(Packet(100, 0, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 1, 5, 0, dest, origin));

    EXPECT_FALSE(reassembler.isComplete());
}

TEST(PageReassemblerQuery, IsCompleteFull) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    for (int i = 0; i < 3; ++i) {
        reassembler.addPacket(Packet(100, i, 3, 0, dest, origin));
    }

    EXPECT_TRUE(reassembler.isComplete());
}

TEST(PageReassemblerQuery, HasPacketAt) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    EXPECT_FALSE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(2));

    reassembler.addPacket(Packet(100, 0, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 2, 5, 0, dest, origin));

    EXPECT_TRUE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(1));
    EXPECT_TRUE(reassembler.hasPacketAt(2));
    EXPECT_FALSE(reassembler.hasPacketAt(3));
}

TEST(PageReassemblerQuery, HasPacketAtInvalidPosition) {
    const PageReassembler reassembler(100, 5);

    EXPECT_FALSE(reassembler.hasPacketAt(10));
}

// =============== Modifiers tests ===============
TEST(PageReassemblerModifiers, AddPacketInOrder) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    const Packet p0(100, 0, 3, 0, dest, origin);
    const Packet p1(100, 1, 3, 0, dest, origin);
    const Packet p2(100, 2, 3, 0, dest, origin);

    EXPECT_TRUE(reassembler.addPacket(p0));
    EXPECT_EQ(reassembler.currentPackets, 1);

    EXPECT_TRUE(reassembler.addPacket(p1));
    EXPECT_EQ(reassembler.currentPackets, 2);

    EXPECT_TRUE(reassembler.addPacket(p2));
    EXPECT_EQ(reassembler.currentPackets, 3);

    EXPECT_TRUE(reassembler.isComplete());
}

TEST(PageReassemblerModifiers, AddPacketOutOfOrder) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 2, 5, 0, dest, origin)));
    EXPECT_EQ(reassembler.currentPackets, 1);

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 0, 5, 0, dest, origin)));
    EXPECT_EQ(reassembler.currentPackets, 2);

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 4, 5, 0, dest, origin)));
    EXPECT_EQ(reassembler.currentPackets, 3);

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 1, 5, 0, dest, origin)));
    EXPECT_EQ(reassembler.currentPackets, 4);

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 3, 5, 0, dest, origin)));
    EXPECT_EQ(reassembler.currentPackets, 5);

    EXPECT_TRUE(reassembler.isComplete());
}

TEST(PageReassemblerModifiers, AddPacketWrongPageID) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    const Packet wrongPacket(200, 0, 3, 0, dest, origin);

    EXPECT_FALSE(reassembler.addPacket(wrongPacket));
    EXPECT_EQ(reassembler.currentPackets, 0);
}

TEST(PageReassemblerModifiers, AddPacketWrongPageLength) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    const Packet wrongPacket(100, 0, 10, 0, dest, origin);

    EXPECT_FALSE(reassembler.addPacket(wrongPacket));
    EXPECT_EQ(reassembler.currentPackets, 0);
}

TEST(PageReassemblerModifiers, AddPacketInvalidPosition) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    const Packet invalidPacket(100, 9, 10, 0, dest, origin);

    EXPECT_FALSE(reassembler.addPacket(invalidPacket));
    EXPECT_EQ(reassembler.currentPackets, 0);
}

TEST(PageReassemblerModifiers, AddPacketDuplicate) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    const Packet p0(100, 0, 3, 0, dest, origin);

    EXPECT_TRUE(reassembler.addPacket(p0));
    EXPECT_EQ(reassembler.currentPackets, 1);

    EXPECT_FALSE(reassembler.addPacket(p0));
    EXPECT_EQ(reassembler.currentPackets, 1);
}

TEST(PageReassemblerModifiers, PackageComplete) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    // Add packets in any order
    for (int i = 4; i >= 0; --i) {
        reassembler.addPacket(Packet(100, i, 5, 0, dest, origin));
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> packetList = reassembler.package();

    // Expect packets in order
    EXPECT_EQ(packetList.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(packetList[i].getPagePosition(), i);
        EXPECT_EQ(packetList[i].getPageID(), 100);
    }

    EXPECT_EQ(reassembler.currentPackets, 0);
}

TEST(PageReassemblerModifiers, PackageIncomplete) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    reassembler.addPacket(Packet(100, 0, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 1, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 2, 5, 0, dest, origin));

    EXPECT_FALSE(reassembler.isComplete());

    EXPECT_THROW(reassembler.package(), std::runtime_error);
}

TEST(PageReassemblerModifiers, PackageSinglePacket) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 1);

    reassembler.addPacket(Packet(100, 0, 1, 0, dest, origin));

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> packetList = reassembler.package();

    EXPECT_EQ(packetList.size(), 1);
    EXPECT_EQ(packetList[0].getPagePosition(), 0);
}

TEST(PageReassemblerModifiers, PackagePreservesPacketData) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(42, 3);

    reassembler.addPacket(Packet(42, 0, 3, 5, dest, origin));
    reassembler.addPacket(Packet(42, 1, 3, 3, dest, origin));
    reassembler.addPacket(Packet(42, 2, 3, 8, dest, origin));

    List<Packet> packetList = reassembler.package();

    EXPECT_EQ(packetList[0].getRouterPriority(), 5);
    EXPECT_EQ(packetList[1].getRouterPriority(), 3);
    EXPECT_EQ(packetList[2].getRouterPriority(), 8);

    EXPECT_EQ(packetList[0].getOriginIP(), origin);
    EXPECT_EQ(packetList[0].getDestinationIP(), dest);
}

TEST(PageReassemblerModifiers, Reset) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 5);

    reassembler.addPacket(Packet(100, 0, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 2, 5, 0, dest, origin));
    reassembler.addPacket(Packet(100, 4, 5, 0, dest, origin));

    EXPECT_EQ(reassembler.currentPackets, 3);

    reassembler.reset();

    EXPECT_EQ(reassembler.currentPackets, 0);
    EXPECT_FALSE(reassembler.hasPacketAt(0));
    EXPECT_FALSE(reassembler.hasPacketAt(2));
    EXPECT_FALSE(reassembler.hasPacketAt(4));
}

TEST(PageReassemblerModifiers, ResetAndReuse) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    reassembler.addPacket(Packet(100, 0, 3, 0, dest, origin));
    reassembler.addPacket(Packet(100, 1, 3, 0, dest, origin));

    reassembler.reset();

    EXPECT_TRUE(reassembler.addPacket(Packet(100, 0, 3, 0, dest, origin)));
    EXPECT_TRUE(reassembler.addPacket(Packet(100, 1, 3, 0, dest, origin)));
    EXPECT_TRUE(reassembler.addPacket(Packet(100, 2, 3, 0, dest, origin)));

    EXPECT_TRUE(reassembler.isComplete());
}

// =============== Complex tests ===============
TEST(PageReassemblerComplex, SimulateNetworkTransmission) {
    const IPAddress terminalA(5, 10);
    const IPAddress terminalB(8, 20);

    PageReassembler reassembler(999, 10);

    const int arrivalOrder[] = {3, 7, 1, 9, 0, 5, 2, 8, 4, 6};

    for (int i = 0; i < 10; ++i) {
        const int pos = arrivalOrder[i];
        Packet p(999, pos, 10, i, terminalB, terminalA);

        EXPECT_TRUE(reassembler.addPacket(p));
        EXPECT_EQ(reassembler.currentPackets, i + 1);
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> orderedPackets = reassembler.package();

    EXPECT_EQ(orderedPackets.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(orderedPackets[i].getPagePosition(), i);
    }
}

TEST(PageReassemblerComplex, SimulatePacketLoss) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 10);

    reassembler.addPacket(Packet(100, 0, 10, 0, dest, origin));
    reassembler.addPacket(Packet(100, 2, 10, 0, dest, origin));
    reassembler.addPacket(Packet(100, 5, 10, 0, dest, origin));
    reassembler.addPacket(Packet(100, 8, 10, 0, dest, origin));

    EXPECT_EQ(reassembler.currentPackets, 4);
    EXPECT_FALSE(reassembler.isComplete());
    EXPECT_DOUBLE_EQ(reassembler.getCompletionRate(), 0.4);

    EXPECT_THROW(reassembler.package(), std::runtime_error);
}

TEST(PageReassemblerComplex, SimulateRetransmission) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(100, 3);

    reassembler.addPacket(Packet(100, 0, 3, 0, dest, origin));

    EXPECT_FALSE(reassembler.addPacket(Packet(100, 0, 3, 0, dest, origin)));

    EXPECT_EQ(reassembler.currentPackets, 1);
}

TEST(PageReassemblerComplex, MultiplePages) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler page1(100, 5);
    PageReassembler page2(200, 3);
    PageReassembler page3(300, 7);

    page1.addPacket(Packet(100, 0, 5, 0, dest, origin));
    page2.addPacket(Packet(200, 0, 3, 0, dest, origin));
    page3.addPacket(Packet(300, 0, 7, 0, dest, origin));

    EXPECT_EQ(page1.currentPackets, 1);
    EXPECT_EQ(page2.currentPackets, 1);
    EXPECT_EQ(page3.currentPackets, 1);

    EXPECT_FALSE(page1.addPacket(Packet(200, 1, 5, 0, dest, origin)));
}

TEST(PageReassemblerComplex, IntegrationWithPage) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PageReassembler reassembler(42, 5);

    for (int i = 0; i < 5; ++i) {
        reassembler.addPacket(Packet(42, i, 5, 0, dest, origin));
    }

    EXPECT_TRUE(reassembler.isComplete());

    List<Packet> packets = reassembler.package();

    const Page reconstructedPage(std::move(packets));

    EXPECT_EQ(reconstructedPage.getPageID(), 42);
    EXPECT_EQ(reconstructedPage.getPageLength(), 5);
    EXPECT_EQ(reconstructedPage.getOriginIP(), origin);
    EXPECT_EQ(reconstructedPage.getDestinationIP(), dest);
}
