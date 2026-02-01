#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>
#include "core/Page.h"

// =============== Constructors tests ===============
TEST(PageConstructors, ValidConstructor) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(100, 10, origin, dest);

    EXPECT_EQ(page.getPageID(), 100);
    EXPECT_EQ(page.getPageLength(), 10);
    EXPECT_EQ(page.getOriginIP(), origin);
    EXPECT_EQ(page.getDestinationIP(), dest);
}

TEST(PageConstructors, ConstructorInvalidOriginIP) {
    const IPAddress invalidOrigin(0, 0);
    const IPAddress dest(20, 10);

    EXPECT_THROW(Page(100, 10, invalidOrigin, dest), std::invalid_argument);
}

TEST(PageConstructors, ConstructorInvalidDestinationIP) {
    const IPAddress origin(10, 5);
    const IPAddress invalidDest(0, 0);

    EXPECT_THROW(Page(100, 10, origin, invalidDest), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsValid) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    for (int i = 0; i < 5; ++i) {
        packets.pushBack(Packet(100, i, 5, 0, dest, origin));
    }

    const Page page(std::move(packets));

    EXPECT_EQ(page.getPageID(), 100);
    EXPECT_EQ(page.getPageLength(), 5);
    EXPECT_EQ(page.getOriginIP(), origin);
    EXPECT_EQ(page.getDestinationIP(), dest);
}

TEST(PageConstructors, ConstructorFromPacketsEmpty) {
    List<Packet> emptyPackets;

    EXPECT_THROW(Page(std::move(emptyPackets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsWrongCount) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    // Create only 3 packets but pageLength says 5
    for (int i = 0; i < 3; ++i) {
        packets.pushBack(Packet(100, i, 5, 0, dest, origin));
    }

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsInconsistentPageID) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, 0, dest, origin));
    packets.pushBack(Packet(100, 1, 3, 0, dest, origin));
    packets.pushBack(Packet(200, 2, 3, 0, dest, origin));  // Different pageID

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsInconsistentPageLength) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, 0, dest, origin));
    packets.pushBack(Packet(100, 1, 5, 0, dest, origin));  // Different length

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsInconsistentOriginIP) {
    const IPAddress origin1(10, 5);
    const IPAddress origin2(15, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 2, 0, dest, origin1));
    packets.pushBack(Packet(100, 1, 2, 0, dest, origin2));  // Different origin

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsInconsistentDestinationIP) {
    const IPAddress origin(10, 5);
    const IPAddress dest1(20, 10);
    const IPAddress dest2(20, 15);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 2, 0, dest1, origin));
    packets.pushBack(Packet(100, 1, 2, 0, dest2, origin));  // Different destination

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, ConstructorFromPacketsWrongPositions) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, 0, dest, origin));
    packets.pushBack(Packet(100, 2, 3, 0, dest, origin));  // Position 2 at index 1
    packets.pushBack(Packet(100, 2, 3, 0, dest, origin));

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST(PageConstructors, CopyConstructor) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page1(100, 10, origin, dest);
    const Page page2(page1);

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLength(), 10);
    EXPECT_EQ(page2.getOriginIP(), origin);
    EXPECT_EQ(page2.getDestinationIP(), dest);
}

TEST(PageConstructors, MoveConstructor) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    Page page1(100, 10, origin, dest);
    const Page page2(std::move(page1));

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLength(), 10);
}

TEST(PageConstructors, CopyAssignment) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page1(100, 10, origin, dest);
    Page page2(200, 5, dest, origin);

    page2 = page1;

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLength(), 10);
}

TEST(PageConstructors, MoveAssignment) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    Page page1(100, 10, origin, dest);
    Page page2(200, 5, dest, origin);

    page2 = std::move(page1);

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLength(), 10);
}

// =============== Page Operations tests ===============
TEST(PageOperations, FragmentToPacketsBasic) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(100, 5, origin, dest);
    List<Packet> packets = page.fragmentToPackets();

    EXPECT_EQ(packets.size(), 5);

    for (int i = 0; i < packets.size(); ++i) {
        EXPECT_EQ(packets[i].getPageID(), 100);
        EXPECT_EQ(packets[i].getPagePosition(), i);
        EXPECT_EQ(packets[i].getPageLength(), 5);
        EXPECT_EQ(packets[i].getOriginIP(), origin);
        EXPECT_EQ(packets[i].getDestinationIP(), dest);
        EXPECT_EQ(packets[i].getRouterPriority(), 0);
    }
}

TEST(PageOperations, FragmentToPacketsWithPriority) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(100, 3, origin, dest);
    List<Packet> packets = page.fragmentToPackets(7);

    EXPECT_EQ(packets.size(), 3);

    for (const auto& packet : packets) {
        EXPECT_EQ(packet.getRouterPriority(), 7);
    }
}

TEST(PageOperations, FragmentToPacketsSinglePacket) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(100, 1, origin, dest);
    List<Packet> packets = page.fragmentToPackets();

    EXPECT_EQ(packets.size(), 1);
    EXPECT_EQ(packets[0].getPagePosition(), 0);
    EXPECT_TRUE(packets[0].isFirstPacket());
    EXPECT_TRUE(packets[0].isLastPacket());
}

TEST(PageOperations, FragmentToPacketsLargePage) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(999, 100, origin, dest);
    List<Packet> packets = page.fragmentToPackets();

    EXPECT_EQ(packets.size(), 100);
    EXPECT_TRUE(packets[0].isFirstPacket());
    EXPECT_TRUE(packets[99].isLastPacket());
}

// =============== Query tests ===============
TEST(PageQuery, IsAddressedTo) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);
    const IPAddress other(30, 15);

    const Page page(100, 10, origin, dest);

    EXPECT_TRUE(page.isAddressedTo(dest));
    EXPECT_FALSE(page.isAddressedTo(origin));
    EXPECT_FALSE(page.isAddressedTo(other));
}

TEST(PageQuery, IsFrom) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);
    const IPAddress other(30, 15);

    const Page page(100, 10, origin, dest);

    EXPECT_TRUE(page.isFrom(origin));
    EXPECT_FALSE(page.isFrom(dest));
    EXPECT_FALSE(page.isFrom(other));
}

// =============== Utilities tests ===============
TEST(PageTest, ToString) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(42, 7, origin, dest);
    std::string str = page.toString();

    // Should contain key information
    EXPECT_NE(str.find("42"), std::string::npos);  // pageID
    EXPECT_NE(str.find('7'), std::string::npos);   // pageLength
}

TEST(PageTest, StreamOperator) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(123, 15, origin, dest);

    std::ostringstream oss;
    oss << page;

    std::string output = oss.str();
    EXPECT_NE(output.find("123"), std::string::npos);
    EXPECT_NE(output.find("15"), std::string::npos);
}

// =============== Comparison tests ===============
TEST(PageTest, EqualityOperator) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page1(100, 10, origin, dest);
    const Page page2(100, 10, origin, dest);
    const Page page3(101, 10, origin, dest);

    EXPECT_TRUE(page1 == page2);
    EXPECT_FALSE(page2 == page3);
    EXPECT_FALSE(page1 == page3);
}

TEST(PageTest, InequalityOperator) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page1(100, 10, origin, dest);
    const Page page2(100, 10, origin, dest);
    const Page page3(101, 10, origin, dest);

    EXPECT_FALSE(page1 != page2);
    EXPECT_TRUE(page2 != page3);
    EXPECT_TRUE(page1 != page3);
}

TEST(PageTest, LessThanOperator) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page1(100, 10, origin, dest);
    const Page page2(101, 10, origin, dest);

    EXPECT_TRUE(page1 < page2);
    EXPECT_FALSE(page2 < page1);
}

// =============== Round-trip tests ===============
TEST(PageRoundTrip, RoundTripFragmentAndReassemble) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    // Create original page
    const Page originalPage(42, 7, origin, dest);

    // Fragment into packets
    List<Packet> packets = originalPage.fragmentToPackets();

    // Reassemble from packets
    const Page reassembledPage{std::move(packets)};

    // Verify they're identical
    EXPECT_EQ(reassembledPage.getPageID(), originalPage.getPageID());
    EXPECT_EQ(reassembledPage.getPageLength(), originalPage.getPageLength());
    EXPECT_EQ(reassembledPage.getOriginIP(), originalPage.getOriginIP());
    EXPECT_EQ(reassembledPage.getDestinationIP(), originalPage.getDestinationIP());
    EXPECT_EQ(reassembledPage, originalPage);
}

TEST(PageRoundTrip, RoundTripMultiplePages) {
    const IPAddress origin1(10, 5);
    const IPAddress dest1(20, 10);
    const IPAddress origin2(15, 8);
    const IPAddress dest2(25, 12);

    // Create multiple pages
    const Page page1(100, 5, origin1, dest1);
    const Page page2(200, 10, origin2, dest2);

    // Fragment
    List<Packet> packets1 = page1.fragmentToPackets();
    List<Packet> packets2 = page2.fragmentToPackets();

    // Reassemble
    const Page rebuilt1(std::move(packets1));
    const Page rebuilt2(std::move(packets2));

    // Verify
    EXPECT_EQ(rebuilt1, page1);
    EXPECT_EQ(rebuilt2, page2);
    EXPECT_NE(rebuilt1, rebuilt2);
}

// =============== Complex tests ===============
TEST(PageComplex, TransmissionScenario) {
    const IPAddress terminalA(5, 10);
    const IPAddress terminalB(8, 20);

    // Terminal A creates a page to send to Terminal B
    const Page pageToSend(999, 20, terminalA, terminalB);

    EXPECT_EQ(pageToSend.getPageLength(), 20);
    EXPECT_TRUE(pageToSend.isFrom(terminalA));
    EXPECT_TRUE(pageToSend.isAddressedTo(terminalB));

    // Fragment to packets for transmission
    List<Packet> outgoingPackets = pageToSend.fragmentToPackets();

    EXPECT_EQ(outgoingPackets.size(), 20);

    // Simulate transmission (packets arrive at Terminal B)
    // Terminal B reassembles the page
    const Page receivedPage(std::move(outgoingPackets));

    EXPECT_EQ(receivedPage.getPageID(), 999);
    EXPECT_TRUE(receivedPage.isFrom(terminalA));
    EXPECT_TRUE(receivedPage.isAddressedTo(terminalB));
}

TEST(PageComplex, RouterPriorityScenario) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    // High priority page
    const Page urgentPage(100, 5, origin, dest);
    List<Packet> urgentPackets = urgentPage.fragmentToPackets(1);

    // Low priority page
    const Page normalPage(200, 5, origin, dest);
    List<Packet> normalPackets = normalPage.fragmentToPackets(10);

    // Verify priorities
    EXPECT_EQ(urgentPackets[0].getRouterPriority(), 1);
    EXPECT_EQ(normalPackets[0].getRouterPriority(), 10);
}

TEST(PageComplex, PacketOrderPreservation) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    const Page page(100, 10, origin, dest);
    List<Packet> packets = page.fragmentToPackets();

    for (int i = 0; i < packets.size(); ++i) {
        EXPECT_EQ(packets[i].getPagePosition(), i);
    }
}

TEST(PageComplex, UseInSTLContainers) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    std::vector<Page> pages;
    pages.emplace_back(100, 5, origin, dest);
    pages.emplace_back(200, 10, origin, dest);
    pages.emplace_back(150, 7, origin, dest);

    std::sort(pages.begin(), pages.end());

    EXPECT_EQ(pages[0].getPageID(), 100);
    EXPECT_EQ(pages[1].getPageID(), 150);
    EXPECT_EQ(pages[2].getPageID(), 200);
}
