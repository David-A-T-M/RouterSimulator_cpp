#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>
#include "core/Page.h"

class PageTest : public ::testing::Test {
protected:
    const IPAddress src{20, 15};
    const IPAddress dst{10, 5};
    static constexpr size_t TICK = 10;
};

// =============== Constructors tests ===============
TEST_F(PageTest, Constructor_Valid) {
    const Page page(100, 10, src, dst);

    EXPECT_EQ(page.getPageID(), 100);
    EXPECT_EQ(page.getPageLen(), 10);
    EXPECT_EQ(page.getSrcIP(), src);
    EXPECT_EQ(page.getDstIP(), dst);
}

TEST_F(PageTest, Constructor_InvalidSrcIP) {
    const IPAddress invalidSrc(0, 0);

    EXPECT_THROW(Page(100, 10, invalidSrc, dst), std::invalid_argument);
}

TEST_F(PageTest, Constructor_InvalidDstIP) {
    const IPAddress invalidDst(0, 0);

    EXPECT_THROW(Page(100, 10, src, invalidDst), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsValid) {
    List<Packet> packets;
    for (int i = 0; i < 5; ++i) {
        packets.pushBack(Packet(100, i, 5, src, dst, TICK));
    }

    const Page page(std::move(packets));

    EXPECT_EQ(page.getPageID(), 100);
    EXPECT_EQ(page.getPageLen(), 5);
    EXPECT_EQ(page.getSrcIP(), src);
    EXPECT_EQ(page.getDstIP(), dst);
}

TEST_F(PageTest, Constructor_PacketsEmpty) {
    List<Packet> emptyPackets;

    EXPECT_THROW(Page(std::move(emptyPackets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsWrongCount) {
    List<Packet> packets;
    // Create only 3 packets, but pageLength says 5
    for (int i = 0; i < 3; ++i) {
        packets.pushBack(Packet(100, i, 5, src, dst, TICK));
    }

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsInconsistentPageID) {
    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, src, dst, TICK));
    packets.pushBack(Packet(100, 1, 3, src, dst, TICK));
    packets.pushBack(Packet(200, 2, 3, src, dst, TICK));  // Different pageID

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsInconsistentPageLen) {
    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, src, dst, TICK));
    packets.pushBack(Packet(100, 1, 5, src, dst, TICK));  // Different length

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsInconsistentSrcIP) {
    const IPAddress otherSrcIP(15, 5);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 2, src, dst, TICK));
    packets.pushBack(Packet(100, 1, 2, otherSrcIP, dst, TICK));  // Different origin

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsInconsistentDstIP) {
    const IPAddress otherDstIP(20, 15);

    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 2, src, dst, TICK));
    packets.pushBack(Packet(100, 1, 2, src, otherDstIP, TICK));  // Different destination

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_PacketsWrongPositions) {
    List<Packet> packets;
    packets.pushBack(Packet(100, 0, 3, src, dst, TICK));
    packets.pushBack(Packet(100, 2, 3, src, dst, TICK));  // Position 2 at index 1
    packets.pushBack(Packet(100, 2, 3, src, dst, TICK));

    EXPECT_THROW(Page(std::move(packets)), std::invalid_argument);
}

TEST_F(PageTest, Constructor_Copy) {
    const Page page1(100, 10, src, dst);
    const Page page2(page1);

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLen(), 10);
    EXPECT_EQ(page2.getSrcIP(), src);
    EXPECT_EQ(page2.getDstIP(), dst);
}

TEST_F(PageTest, Constructor_Move) {
    Page page1(100, 10, src, dst);
    const Page page2(std::move(page1));

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLen(), 10);
}

TEST_F(PageTest, Assignment_Copy) {
    const Page page1(100, 10, src, dst);
    Page page2(200, 5, src, dst);

    page2 = page1;

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLen(), 10);
}

TEST_F(PageTest, Assignment_Move) {
    Page page1(100, 10, src, dst);
    Page page2(200, 5, src, dst);

    page2 = std::move(page1);

    EXPECT_EQ(page2.getPageID(), 100);
    EXPECT_EQ(page2.getPageLen(), 10);
}

// =============== Page Operations tests ===============
TEST_F(PageTest, ToPackets_Basic) {
    const Page page(100, 5, src, dst);
    List<Packet> packets = page.toPackets(TICK);

    EXPECT_EQ(packets.size(), 5);

    for (int i = 0; i < packets.size(); ++i) {
        EXPECT_EQ(packets[i].getPageID(), 100);
        EXPECT_EQ(packets[i].getPagePos(), i);
        EXPECT_EQ(packets[i].getPageLen(), 5);
        EXPECT_EQ(packets[i].getSrcIP(), src);
        EXPECT_EQ(packets[i].getDstIP(), dst);
        EXPECT_EQ(packets[i].getExpTick(), TICK);
    }
}

TEST_F(PageTest, ToPackets_SinglePacket) {
    const Page page(100, 1, src, dst);
    List<Packet> packets = page.toPackets(TICK);

    EXPECT_EQ(packets.size(), 1);
    EXPECT_EQ(packets[0].getPagePos(), 0);
    EXPECT_TRUE(packets[0].isFirstPacket());
    EXPECT_TRUE(packets[0].isLastPacket());
}

TEST_F(PageTest, ToPackets_LargePage) {
    const Page page(999, 100, src, dst);
    List<Packet> packets = page.toPackets(TICK);

    EXPECT_EQ(packets.size(), 100);
    EXPECT_TRUE(packets[0].isFirstPacket());
    EXPECT_TRUE(packets[99].isLastPacket());
}

// =============== Utilities tests ===============
TEST_F(PageTest, ToString_Basic) {
    const Page page(42, 7, src, dst);
    std::string str = page.toString();

    // Should contain key information
    EXPECT_NE(str.find("42"), std::string::npos);  // pageID
    EXPECT_NE(str.find('7'), std::string::npos);   // pageLength
}

TEST_F(PageTest, Operator_Stream) {
    const Page page(123, 15, src, dst);

    std::ostringstream oss;
    oss << page;

    std::string output = oss.str();
    EXPECT_NE(output.find("123"), std::string::npos);
    EXPECT_NE(output.find("15"), std::string::npos);
}

// =============== Comparison tests ===============
TEST_F(PageTest, Operator_Equality) {
    const Page page1(100, 10, src, dst);
    const Page page2(100, 10, src, dst);
    const Page page3(101, 10, src, dst);

    EXPECT_TRUE(page1 == page2);
    EXPECT_FALSE(page2 == page3);
    EXPECT_FALSE(page1 == page3);
}

TEST_F(PageTest, Operator_Inequality) {
    const Page page1(100, 10, src, dst);
    const Page page2(100, 10, src, dst);
    const Page page3(101, 10, src, dst);

    EXPECT_FALSE(page1 != page2);
    EXPECT_TRUE(page2 != page3);
    EXPECT_TRUE(page1 != page3);
}

TEST_F(PageTest, Operator_LessThan) {
    const Page page1(100, 10, src, dst);
    const Page page2(101, 10, src, dst);

    EXPECT_TRUE(page1 < page2);
    EXPECT_FALSE(page2 < page1);
}

// =============== Round-trip tests ===============
TEST_F(PageTest, RoundTrip_FragmentAndReassemble) {
    // Create original page
    const Page originalPage(42, 7, src, dst);

    // Fragment into packets
    List<Packet> packets = originalPage.toPackets(TICK);

    // Reassemble from packets
    const Page reassembledPage{std::move(packets)};

    // Verify they're identical
    EXPECT_EQ(reassembledPage.getPageID(), originalPage.getPageID());
    EXPECT_EQ(reassembledPage.getPageLen(), originalPage.getPageLen());
    EXPECT_EQ(reassembledPage.getSrcIP(), originalPage.getSrcIP());
    EXPECT_EQ(reassembledPage.getDstIP(), originalPage.getDstIP());
    EXPECT_EQ(reassembledPage, originalPage);
}

TEST_F(PageTest, RoundTrip_MultiplePages) {
    const IPAddress otherSrcIP(15, 8);
    const IPAddress otherDstIP(25, 12);

    // Create multiple pages
    const Page page1(100, 5, src, dst);
    const Page page2(200, 10, otherSrcIP, otherDstIP);

    // Fragment
    List<Packet> packets1 = page1.toPackets(TICK);
    List<Packet> packets2 = page2.toPackets(TICK);

    // Reassemble
    const Page rebuilt1(std::move(packets1));
    const Page rebuilt2(std::move(packets2));

    // Verify
    EXPECT_EQ(rebuilt1, page1);
    EXPECT_EQ(rebuilt2, page2);
    EXPECT_NE(rebuilt1, rebuilt2);
}

// =============== Complex tests ===============
TEST_F(PageTest, Complex_TransmissionScenario) {
    // Terminal A creates a page to send to Terminal B
    const Page pageToSend(999, 20, src, dst);

    EXPECT_EQ(pageToSend.getPageLen(), 20);

    // Fragment to packets for transmission
    List<Packet> outgoingPackets = pageToSend.toPackets(TICK);

    EXPECT_EQ(outgoingPackets.size(), 20);

    // Simulate transmission (packets arrive at Terminal B)
    // Terminal B reassembles the page
    const Page receivedPage(std::move(outgoingPackets));

    EXPECT_EQ(receivedPage.getPageID(), 999);
}

TEST_F(PageTest, Complex_OrderPreservation) {
    const Page page(100, 10, src, dst);
    List<Packet> packets = page.toPackets(TICK);

    for (int i = 0; i < packets.size(); ++i) {
        EXPECT_EQ(packets[i].getPagePos(), i);
    }
}

TEST_F(PageTest, Complex_STLContainers) {
    std::vector<Page> pages;
    pages.emplace_back(100, 5, src, dst);
    pages.emplace_back(200, 10, src, dst);
    pages.emplace_back(150, 7, src, dst);

    std::sort(pages.begin(), pages.end());

    EXPECT_EQ(pages[0].getPageID(), 100);
    EXPECT_EQ(pages[1].getPageID(), 150);
    EXPECT_EQ(pages[2].getPageID(), 200);
}
