#include <gtest/gtest.h>
#include "core/PacketBuffer.h"
#include "core/Page.h"

class PacketBufferTest : public testing::Test {
protected:
    const IPAddress src{20, 15};
    const IPAddress dst{10, 5};
    const IPAddress rtr{15, 0};
    static constexpr size_t TICK = 100;
    PacketBuffer buffer{};
};

// =============== Constructors tests ===============
TEST_F(PacketBufferTest, Constructor_Default) {
    const PacketBuffer buffer{};

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.getCapacity(), 0);  // Unlimited
    EXPECT_FALSE(buffer.getDstIP().isValid());
}

TEST_F(PacketBufferTest, Constructor_WithCapacity) {
    const PacketBuffer buffer{100};

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.getCapacity(), 100);
    EXPECT_FALSE(buffer.getDstIP().isValid());
}

TEST_F(PacketBufferTest, Constructor_WithIP) {
    const PacketBuffer buffer(rtr);

    EXPECT_EQ(buffer.getDstIP(), rtr);
    EXPECT_EQ(buffer.getCapacity(), 0);
    EXPECT_TRUE(buffer.getDstIP().isValid());
}

TEST_F(PacketBufferTest, Constructor_WithIpAndCapacity) {
    const PacketBuffer buffer(rtr, 100);

    EXPECT_EQ(buffer.getDstIP(), rtr);
    EXPECT_EQ(buffer.getCapacity(), 100);
    EXPECT_FALSE(buffer.isFull());
    EXPECT_TRUE(buffer.getDstIP().isValid());
}

// =============== Queue operation tests ===============
TEST_F(PacketBufferTest, Dequeue_Empty) {
    EXPECT_THROW(buffer.dequeue(), std::runtime_error);
}

TEST_F(PacketBufferTest, Queue_Deque) {
    Packet p1(100, 0, 3, src, dst, TICK);
    Packet p2(100, 1, 3, src, dst, TICK);
    Packet p3(100, 2, 3, src, dst, TICK);

    EXPECT_TRUE(buffer.enqueue(p1));
    EXPECT_TRUE(buffer.enqueue(p2));
    EXPECT_TRUE(buffer.enqueue(p3));

    EXPECT_EQ(buffer.size(), 3);

    Packet out1 = buffer.dequeue();
    EXPECT_EQ(out1.getPagePos(), 0);

    Packet out2 = buffer.dequeue();
    EXPECT_EQ(out2.getPagePos(), 1);

    Packet out3 = buffer.dequeue();
    EXPECT_EQ(out3.getPagePos(), 2);

    EXPECT_TRUE(buffer.isEmpty());
}

TEST_F(PacketBufferTest, Queue_CapacityLimit) {
    PacketBuffer buffer(3);

    EXPECT_TRUE(buffer.enqueue(Packet(100, 0, 5, src, dst, TICK)));
    EXPECT_TRUE(buffer.enqueue(Packet(100, 1, 5, src, dst, TICK)));
    EXPECT_TRUE(buffer.enqueue(Packet(100, 2, 5, src, dst, TICK)));

    EXPECT_TRUE(buffer.isFull());

    // Should reject further packets
    EXPECT_FALSE(buffer.enqueue(Packet(100, 3, 5, src, dst, TICK)));

    EXPECT_EQ(buffer.size(), 3);
}

TEST_F(PacketBufferTest, UnlimitedCapacity) {
    // Should never be full
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(buffer.enqueue(Packet(100, i, 100, src, dst, TICK)));
    }

    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(buffer.size(), 100);
}

// =============== Query tests ===============
TEST_F(PacketBufferTest, Query_AvailableSpace) {
    PacketBuffer buffer(10);

    EXPECT_EQ(buffer.availableSpace(), 10);

    buffer.enqueue(Packet(100, 0, 5, src, dst, TICK));
    buffer.enqueue(Packet(100, 1, 5, src, dst, TICK));

    EXPECT_EQ(buffer.availableSpace(), 8);
}

TEST_F(PacketBufferTest, Query_Utilization) {
    PacketBuffer buffer(10);

    EXPECT_DOUBLE_EQ(buffer.getUtilization(), 0.0);

    for (int i = 0; i < 5; ++i) {
        buffer.enqueue(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_DOUBLE_EQ(buffer.getUtilization(), 0.5);
}

TEST_F(PacketBufferTest, Query_Contains) {
    buffer.enqueue(Packet(100, 0, 3, src, dst, TICK));
    buffer.enqueue(Packet(100, 2, 3, src, dst, TICK));

    EXPECT_TRUE(buffer.contains(100, 0));
    EXPECT_TRUE(buffer.contains(100, 2));
    EXPECT_FALSE(buffer.contains(100, 1));
    EXPECT_FALSE(buffer.contains(200, 0));
}

// =============== Buffer Management tests ===============
TEST_F(PacketBufferTest, Setter_Capacity) {
    buffer.setCapacity(10);
    EXPECT_EQ(buffer.getCapacity(), 10);
}

TEST_F(PacketBufferTest, Setter_DstIP) {
    buffer.setDstIP(rtr);
    EXPECT_EQ(buffer.getDstIP(), rtr);
}

TEST_F(PacketBufferTest, Setter_CapacityBelowSize) {
    for (int i = 0; i < 5; ++i) {
        buffer.enqueue(Packet(100, i, 10, src, dst, TICK));
    }

    EXPECT_THROW(buffer.setCapacity(3), std::invalid_argument);
}

TEST_F(PacketBufferTest, Clear) {
    buffer.enqueue(Packet(100, 0, 3, src, dst, TICK));
    buffer.enqueue(Packet(100, 1, 3, src, dst, TICK));
    buffer.enqueue(Packet(100, 2, 3, src, dst, TICK));

    EXPECT_EQ(buffer.size(), 3);

    buffer.clear();

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.size(), 0);
}

TEST_F(PacketBufferTest, RemoveAt) {
    buffer.enqueue(Packet(100, 0, 3, src, dst, TICK));
    buffer.enqueue(Packet(100, 1, 3, src, dst, TICK));
    buffer.enqueue(Packet(100, 2, 3, src, dst, TICK));

    buffer.removeAt(1);  // Remove middle packet

    EXPECT_EQ(buffer.size(), 2);
}

// =============== Utilities tests ===============
TEST_F(PacketBufferTest, Util_ToString) {
    const PacketBuffer buffer(100);

    std::string str = buffer.toString();

    EXPECT_NE(str.find("100"), std::string::npos);
}

// =============== Complex tests ===============
TEST_F(PacketBufferTest, TerminalOutputBuffer) {
    const IPAddress terminalIP(5, 10);
    const IPAddress destIP(8, 20);

    PacketBuffer outputBuffer(50);

    // Fragment a page
    const Page page(100, 10, terminalIP, destIP);
    List<Packet> packets = page.toPackets(TICK);

    for (const auto& packet : packets) {
        EXPECT_TRUE(outputBuffer.enqueue(packet));
    }

    EXPECT_EQ(outputBuffer.size(), 10);
}

TEST_F(PacketBufferTest, RouterInputBuffer) {
    // Router input buffer
    PacketBuffer inputBuffer(200);

    // Packets arrive in random order
    inputBuffer.enqueue(Packet(100, 3, 10, src, dst, TICK));
    inputBuffer.enqueue(Packet(100, 1, 10, src, dst, TICK));
    inputBuffer.enqueue(Packet(100, 5, 10, src, dst, TICK));

    // Should dequeue in arrival order
    EXPECT_EQ(inputBuffer.dequeue().getPagePos(), 3);
    EXPECT_EQ(inputBuffer.dequeue().getPagePos(), 1);
    EXPECT_EQ(inputBuffer.dequeue().getPagePos(), 5);
}

TEST_F(PacketBufferTest, BufferOverflow) {
    PacketBuffer buffer(6);

    int packetsAccepted = 0;
    int packetsRejected = 0;

    for (size_t i = 0; i < 10; ++i) {
        if (buffer.enqueue(Packet(100, i, 10, src, dst, TICK))) {
            packetsAccepted++;
        } else {
            packetsRejected++;
        }
    }

    EXPECT_EQ(packetsAccepted, 6);
    EXPECT_EQ(packetsRejected, 4);
}
