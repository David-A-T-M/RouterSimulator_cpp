#include <gtest/gtest.h>
#include "core/PacketBuffer.h"
#include "core/Page.h"

// =============== Constructors tests ===============
TEST(PacketBufferConstructors, ConstructorFIFO) {
    const PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    EXPECT_EQ(buffer.getMode(), PacketBuffer::Mode::FIFO);
    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.getCapacity(), 0);  // Unlimited
}

TEST(PacketBufferConstructors, ConstructorPriority) {
    const PacketBuffer buffer(PacketBuffer::Mode::PRIORITY, 100);

    EXPECT_EQ(buffer.getMode(), PacketBuffer::Mode::PRIORITY);
    EXPECT_EQ(buffer.getCapacity(), 100);
    EXPECT_FALSE(buffer.isFull());
}

TEST(PacketBufferConstructors, ConstructorInvalidCapacity) {
    EXPECT_THROW(PacketBuffer(PacketBuffer::Mode::FIFO, -5), std::invalid_argument);
}

// =============== Getters ===============
TEST(PacketBufferGetters, GetMaxPriority) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::PRIORITY);

    EXPECT_EQ(buffer.getMaxPriority(), -1);  // Empty

    buffer.enqueue(Packet(100, 0, 3, 5, dest, origin));
    buffer.enqueue(Packet(100, 1, 3, 10, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 3, dest, origin));

    EXPECT_EQ(buffer.getMaxPriority(), 10);
}

TEST(PacketBufferGetters, GetMinPriority) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::PRIORITY);

    EXPECT_EQ(buffer.getMinPriority(), -1);  // Empty

    buffer.enqueue(Packet(100, 0, 3, 5, dest, origin));
    buffer.enqueue(Packet(100, 1, 3, 10, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 3, dest, origin));

    EXPECT_EQ(buffer.getMinPriority(), 3);
}

// =============== Queue operation tests ===============
TEST(PacketBufferQueue, DequeueEmpty) {
    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    EXPECT_THROW(buffer.dequeue(), std::runtime_error);
}

// =============== FIFO Mode ===============
TEST(PacketBufferQueue, FIFOEnqueueDequeue) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    Packet p1(100, 0, 3, 0, dest, origin);
    Packet p2(100, 1, 3, 0, dest, origin);
    Packet p3(100, 2, 3, 0, dest, origin);

    EXPECT_TRUE(buffer.enqueue(p1));
    EXPECT_TRUE(buffer.enqueue(p2));
    EXPECT_TRUE(buffer.enqueue(p3));

    EXPECT_EQ(buffer.size(), 3);

    Packet out1 = buffer.dequeue();
    EXPECT_EQ(out1.getPagePosition(), 0);

    Packet out2 = buffer.dequeue();
    EXPECT_EQ(out2.getPagePosition(), 1);

    Packet out3 = buffer.dequeue();
    EXPECT_EQ(out3.getPagePosition(), 2);

    EXPECT_TRUE(buffer.isEmpty());
}

// =============== Priority Mode ===============
TEST(PacketBufferQueue, PriorityOrdering) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::PRIORITY);

    // Add with different priorities (lower = higher urgency)
    buffer.enqueue(Packet(100, 0, 5, 5, dest, origin));  // Priority 5
    buffer.enqueue(Packet(100, 1, 5, 2, dest, origin));  // Priority 2 (higher urgency)
    buffer.enqueue(Packet(100, 2, 5, 8, dest, origin));  // Priority 8 (lower urgency)
    buffer.enqueue(Packet(100, 3, 5, 2, dest, origin));  // Priority 2 (same as second)

    EXPECT_EQ(buffer.size(), 4);

    // Should dequeue in priority order: 2, 2, 5, 8
    const Packet p1 = buffer.dequeue();
    EXPECT_EQ(p1.getRouterPriority(), 2);

    const Packet p2 = buffer.dequeue();
    EXPECT_EQ(p2.getRouterPriority(), 2);

    const Packet p3 = buffer.dequeue();
    EXPECT_EQ(p3.getRouterPriority(), 5);

    const Packet p4 = buffer.dequeue();
    EXPECT_EQ(p4.getRouterPriority(), 8);
}

TEST(PacketBufferQueue, PriorityInsertionOrder) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::PRIORITY);

    buffer.enqueue(Packet(100, 0, 3, 10, dest, origin));
    buffer.enqueue(Packet(100, 1, 3, 5, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 1, dest, origin));

    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 1);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 5);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 10);
}

// =============== Query tests ===============
TEST(PacketBufferQuery, AvailableSpace) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO, 10);

    EXPECT_EQ(buffer.availableSpace(), 10);

    buffer.enqueue(Packet(100, 0, 5, 0, dest, origin));
    buffer.enqueue(Packet(100, 1, 5, 0, dest, origin));

    EXPECT_EQ(buffer.availableSpace(), 8);
}

TEST(PacketBufferQuery, Utilization) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO, 10);

    EXPECT_DOUBLE_EQ(buffer.getUtilization(), 0.0);

    for (int i = 0; i < 5; ++i) {
        buffer.enqueue(Packet(100, i, 10, 0, dest, origin));
    }

    EXPECT_DOUBLE_EQ(buffer.getUtilization(), 0.5);
}

TEST(PacketBufferQuery, Contains) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    buffer.enqueue(Packet(100, 0, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 0, dest, origin));

    EXPECT_TRUE(buffer.contains(100, 0));
    EXPECT_TRUE(buffer.contains(100, 2));
    EXPECT_FALSE(buffer.contains(100, 1));
    EXPECT_FALSE(buffer.contains(200, 0));
}

TEST(PacketBufferQuery, CountPacketsFromPage) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    buffer.enqueue(Packet(100, 0, 5, 0, dest, origin));
    buffer.enqueue(Packet(100, 1, 5, 0, dest, origin));
    buffer.enqueue(Packet(200, 0, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 2, 5, 0, dest, origin));

    EXPECT_EQ(buffer.countPacketsFromPage(100), 3);
    EXPECT_EQ(buffer.countPacketsFromPage(200), 1);
    EXPECT_EQ(buffer.countPacketsFromPage(300), 0);
}

// =============== Buffer Management tests ===============
TEST(PacketBufferManagement, SetCapacity) {
    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    buffer.setCapacity(10);
    EXPECT_EQ(buffer.getCapacity(), 10);
}

TEST(PacketBufferManagement, SetCapacityBelowSize) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    for (int i = 0; i < 5; ++i) {
        buffer.enqueue(Packet(100, i, 10, 0, dest, origin));
    }

    EXPECT_THROW(buffer.setCapacity(3), std::invalid_argument);
}

TEST(PacketBufferManagement, CapacityLimit) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO, 3);

    EXPECT_TRUE(buffer.enqueue(Packet(100, 0, 5, 0, dest, origin)));
    EXPECT_TRUE(buffer.enqueue(Packet(100, 1, 5, 0, dest, origin)));
    EXPECT_TRUE(buffer.enqueue(Packet(100, 2, 5, 0, dest, origin)));

    EXPECT_TRUE(buffer.isFull());

    // Should reject further packets
    EXPECT_FALSE(buffer.enqueue(Packet(100, 3, 5, 0, dest, origin)));

    EXPECT_EQ(buffer.size(), 3);
}

TEST(PacketBufferManagement, UnlimitedCapacity) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO, 0);  // Unlimited

    // Should never be full
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(buffer.enqueue(Packet(100, i, 100, 0, dest, origin)));
    }

    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(buffer.size(), 100);
}

TEST(PacketBufferManagement, Clear) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    buffer.enqueue(Packet(100, 0, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 1, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 0, dest, origin));

    EXPECT_EQ(buffer.size(), 3);

    buffer.clear();

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_EQ(buffer.size(), 0);
}

TEST(PacketBufferManagement, RemoveAt) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO);

    buffer.enqueue(Packet(100, 0, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 1, 3, 0, dest, origin));
    buffer.enqueue(Packet(100, 2, 3, 0, dest, origin));

    buffer.removeAt(1);  // Remove middle packet

    EXPECT_EQ(buffer.size(), 2);
}

// =============== Utilities tests ===============
TEST(PacketBufferUtilities, ToString) {
    const PacketBuffer buffer(PacketBuffer::Mode::FIFO, 100);

    std::string str = buffer.toString();

    EXPECT_NE(str.find("FIFO"), std::string::npos);
    EXPECT_NE(str.find("100"), std::string::npos);
}

// =============== Complex tests ===============
TEST(PacketBufferComplex, TerminalOutputBuffer) {
    const IPAddress terminalIP(5, 10);
    const IPAddress destIP(8, 20);

    PacketBuffer outputBuffer(PacketBuffer::Mode::PRIORITY, 50);

    // Fragment a page
    const Page page(100, 10, terminalIP, destIP);
    List<Packet> packets = page.fragmentToPackets();

    for (const auto& packet : packets) {
        EXPECT_TRUE(outputBuffer.enqueue(packet));
    }

    EXPECT_EQ(outputBuffer.size(), 10);
}

TEST(PacketBufferComplex, RouterInputBuffer) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    // Router input buffer (FIFO)
    PacketBuffer inputBuffer(PacketBuffer::Mode::FIFO, 200);

    // Packets arrive in random order
    inputBuffer.enqueue(Packet(100, 3, 10, 0, dest, origin));
    inputBuffer.enqueue(Packet(100, 1, 10, 0, dest, origin));
    inputBuffer.enqueue(Packet(100, 5, 10, 0, dest, origin));

    // Should dequeue in arrival order (FIFO)
    EXPECT_EQ(inputBuffer.dequeue().getPagePosition(), 3);
    EXPECT_EQ(inputBuffer.dequeue().getPagePosition(), 1);
    EXPECT_EQ(inputBuffer.dequeue().getPagePosition(), 5);
}

TEST(PacketBufferComplex, BufferOverflow) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::FIFO, 6);

    int packetsAccepted = 0;
    int packetsRejected = 0;

    for (int i = 0; i < 10; ++i) {
        if (buffer.enqueue(Packet(100, i, 10, 0, dest, origin))) {
            packetsAccepted++;
        } else {
            packetsRejected++;
        }
    }

    EXPECT_EQ(packetsAccepted, 6);
    EXPECT_EQ(packetsRejected, 4);
}

TEST(PacketBufferComplex, PriorityQueueMixedPriorities) {
    const IPAddress origin(10, 5);
    const IPAddress dest(20, 10);

    PacketBuffer buffer(PacketBuffer::Mode::PRIORITY);

    // Add packets with mixed priorities
    buffer.enqueue(Packet(100, 0, 10, 5, dest, origin));
    buffer.enqueue(Packet(100, 1, 10, 1, dest, origin));  // Highest priority
    buffer.enqueue(Packet(100, 2, 10, 3, dest, origin));
    buffer.enqueue(Packet(100, 3, 10, 5, dest, origin));
    buffer.enqueue(Packet(100, 4, 10, 2, dest, origin));

    // Should dequeue in priority order: 1, 2, 3, 5, 5
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 1);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 2);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 3);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 5);
    EXPECT_EQ(buffer.dequeue().getRouterPriority(), 5);
}
