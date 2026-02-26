#include <gtest/gtest.h>
#include "core/Router.h"
#include "core/Terminal.h"

class RouterTest : public testing::Test {
protected:
    static constexpr size_t TICK = 100;

    Router rtr1{IPAddress{5, 0}};
    Router rtr2{IPAddress{10, 0}};
    Router rtr3{IPAddress{15, 0}};

    void connectAndRoute() {
        rtr1.connectRouter(&rtr2);
        rtr1.connectRouter(&rtr3);

        RoutingTable rt;
        rt.setNextHopIP(rtr2.getIP(), rtr2.getIP());
        rt.setNextHopIP(rtr3.getIP(), rtr3.getIP());
        rtr1.setRoutingTable(std::move(rt));
    }
};

// =============== Constructor tests ===============
TEST_F(RouterTest, Constructor_Default) {
    EXPECT_EQ(rtr1.getIP().getRouterIP(), 5);
    EXPECT_EQ(rtr1.getTerminalCount(), 0);
    EXPECT_EQ(rtr1.getRouterCount(), 0);
    EXPECT_EQ(rtr1.getInProcCap(), Router::DEF_INPUT_PROC);
    EXPECT_EQ(rtr1.getLocBufferBW(), Router::DEF_LOC_BW);
    EXPECT_EQ(rtr1.getOutBufferBW(), Router::DEF_OUTPUT_BW);
    EXPECT_EQ(rtr1.getPacketsReceived(), 0);
    EXPECT_EQ(rtr1.getPacketsDropped(), 0);
    EXPECT_EQ(rtr1.getPacketsTimedOut(), 0);
    EXPECT_EQ(rtr1.getPacketsForwarded(), 0);
    EXPECT_EQ(rtr1.getPacketsDelivered(), 0);
    EXPECT_EQ(rtr1.getLocalBufferUsage(), 0);
}

TEST_F(RouterTest, Constructor_InvalidIP) {
    const IPAddress invalidIP{5, 10};
    EXPECT_THROW(Router{invalidIP}, std::invalid_argument);
}

TEST_F(RouterTest, Constructor_WithTerminals) {
    const Router router{IPAddress{10, 0}, 5};
    EXPECT_EQ(router.getTerminalCount(), 5);
}

TEST_F(RouterTest, Constructor_CustomConfig) {
    const Router::Config cfg{50, 8, 20, 6, 30, 4};
    const Router router{IPAddress{10, 0}, 0, cfg};

    EXPECT_EQ(router.getInProcCap(), 8);
    EXPECT_EQ(router.getLocBufferBW(), 6);
    EXPECT_EQ(router.getOutBufferBW(), 4);
}

// =============== Connection Management tests ===============
TEST_F(RouterTest, ConnectTerminal_Valid) {
    auto t = std::make_unique<Terminal>(&rtr1, 10);
    EXPECT_EQ(rtr1.getTerminalCount(), 0);
    EXPECT_NO_THROW(rtr1.connectTerminal(std::move(t)));
    EXPECT_EQ(rtr1.getTerminalCount(), 1);
}

TEST_F(RouterTest, ConnectTerminal_Null) {
    EXPECT_THROW(rtr1.connectTerminal(nullptr), std::invalid_argument);
}

TEST_F(RouterTest, ConnectTerminal_Duplicate) {
    auto t1 = std::make_unique<Terminal>(&rtr1, 10);
    auto t2 = std::make_unique<Terminal>(&rtr1, 10);
    rtr1.connectTerminal(std::move(t1));
    EXPECT_THROW(rtr1.connectTerminal(std::move(t2)), std::invalid_argument);
}

TEST_F(RouterTest, ConnectTerminal_WrongRouter) {
    auto t = std::make_unique<Terminal>(&rtr2, 5);
    EXPECT_THROW(rtr1.connectTerminal(std::move(t)), std::invalid_argument);
}

TEST_F(RouterTest, ConnectTerminal_Multiple) {
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 1));
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 2));
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 3));
    EXPECT_EQ(rtr1.getTerminalCount(), 3);
}

TEST_F(RouterTest, ConnectRouter_Valid) {
    EXPECT_TRUE(rtr1.connectRouter(&rtr2));
    EXPECT_EQ(rtr1.getRouterCount(), 1);
}

TEST_F(RouterTest, ConnectRouter_Null) {
    EXPECT_THROW(rtr1.connectRouter(nullptr), std::invalid_argument);
}

TEST_F(RouterTest, ConnectRouter_Duplicate) {
    EXPECT_TRUE(rtr1.connectRouter(&rtr2));
    EXPECT_FALSE(rtr1.connectRouter(&rtr2));
    EXPECT_EQ(rtr1.getRouterCount(), 1);
}

TEST_F(RouterTest, ConnectRouter_Self) {
    EXPECT_FALSE(rtr1.connectRouter(&rtr1));
    EXPECT_EQ(rtr1.getRouterCount(), 0);
}

TEST_F(RouterTest, ConnectRouter_Multiple) {
    EXPECT_TRUE(rtr1.connectRouter(&rtr2));
    EXPECT_TRUE(rtr1.connectRouter(&rtr3));
    EXPECT_EQ(rtr1.getRouterCount(), 2);
}

// =============== Packet Reception tests ===============
TEST_F(RouterTest, ReceivePacket_Success) {
    const Packet packet{100, 0, 5, IPAddress{10, 1}, IPAddress{5, 1}, TICK};
    EXPECT_TRUE(rtr1.receivePacket(packet));
    EXPECT_EQ(rtr1.getPacketsReceived(), 1);
    EXPECT_EQ(rtr1.getPacketsInPending(), 1);
    EXPECT_EQ(rtr1.getPacketsOutPending(), 0);
    EXPECT_EQ(rtr1.getPacketsLocPending(), 0);
}

TEST_F(RouterTest, ReceivePacket_BufferFull) {
    const Router::Config cfg{2, Router::DEF_INPUT_PROC, 0, Router::DEF_LOC_BW,
                             0, Router::DEF_OUTPUT_BW};
    Router router{IPAddress{5, 0}, 0, cfg};

    EXPECT_TRUE(router.receivePacket(Packet{100, 0, 5, IPAddress{10, 1}, IPAddress{5, 1}, TICK}));
    EXPECT_TRUE(router.receivePacket(Packet{100, 1, 5, IPAddress{10, 1}, IPAddress{5, 1}, TICK}));
    EXPECT_FALSE(router.receivePacket(Packet{100, 2, 5, IPAddress{10, 1}, IPAddress{5, 1}, TICK}));
    EXPECT_EQ(router.getPacketsReceived(), 3);
    EXPECT_EQ(router.getPacketsDropped(), 1);
    EXPECT_EQ(router.getPacketsInPending(), 2);
}

// =============== Input buffer processing tests ===============
TEST_F(RouterTest, ProcessInputBuffer_ToLocalBuffer) {
    const IPAddress dest{5, 10};
    const IPAddress origin{10, 5};
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 10));

    rtr1.receivePacket(Packet{100, 0, 5, origin, dest, TICK});
    EXPECT_EQ(rtr1.getLocalBufferUsage(), 0);

    rtr1.processInputBuffer(1);
    EXPECT_EQ(rtr1.getLocalBufferUsage(), 1);
}

TEST_F(RouterTest, ProcessInputBuffer_ToNeighborBuffer) {
    connectAndRoute();
    const IPAddress dst{10, 1};
    const IPAddress src{5, 1};

    rtr1.receivePacket(Packet{100, 0, 5, src, dst, TICK});
    rtr1.processInputBuffer(1);

    EXPECT_EQ(rtr1.getNeighborBufferUsage(rtr2.getIP()), 1);
}

TEST_F(RouterTest, ProcessInputBuffer_NoRoute) {
    const IPAddress src{5, 1};
    const IPAddress dst{99, 1};

    rtr1.receivePacket(Packet{100, 0, 5, src, dst, TICK});
    rtr1.processInputBuffer(1);

    EXPECT_EQ(rtr1.getPacketsDropped(), 1);
}

TEST_F(RouterTest, ProcessInputBuffer_BandwidthLimit) {
    const Router::Config cfg{0, 2, 0, Router::DEF_LOC_BW, 0, Router::DEF_OUTPUT_BW};
    Router router{IPAddress{5, 0}, 0, cfg};

    const IPAddress src{99, 1};
    const IPAddress dst{5, 1};

    for (size_t i = 0; i < 5; ++i) {
        router.receivePacket(Packet{100, i, 10, src, dst, TICK});
    }

    EXPECT_EQ(router.processInputBuffer(1), 2);
}

TEST_F(RouterTest, ProcessInputBuffer_ExpiredPacket) {
    const IPAddress dst{5, 10};
    const IPAddress src{10, 5};

    rtr1.receivePacket(Packet{100, 0, 5, src, dst, 5});
    rtr1.processInputBuffer(10);

    EXPECT_EQ(rtr1.getPacketsTimedOut(), 1);
    EXPECT_EQ(rtr1.getLocalBufferUsage(), 0);
}

// =============== Output buffer processing tests ===============
TEST_F(RouterTest, ProcessOutputBuffers_Basic) {
    connectAndRoute();

    rtr1.receivePacket(Packet{100, 0, 5, IPAddress{5, 1}, IPAddress{10, 1}, TICK});
    rtr1.processInputBuffer(1);
    rtr1.processOutputBuffers(1);

    EXPECT_EQ(rtr2.getPacketsReceived(), 1);
    EXPECT_EQ(rtr1.getPacketsForwarded(), 1);
}

TEST_F(RouterTest, ProcessOutputBuffers_MultipleConnections) {
    connectAndRoute();

    rtr1.receivePacket(Packet{100, 0, 5, IPAddress{5, 1}, IPAddress{10, 1}, TICK});
    rtr1.receivePacket(Packet{200, 0, 5, IPAddress{5, 1}, IPAddress{15, 1}, TICK});
    rtr1.processInputBuffer(1);
    rtr1.processOutputBuffers(1);

    EXPECT_EQ(rtr2.getPacketsReceived(), 1);
    EXPECT_EQ(rtr3.getPacketsReceived(), 1);
    EXPECT_EQ(rtr1.getPacketsForwarded(), 2);
}

TEST_F(RouterTest, ProcessOutputBuffers_BandwidthLimit) {
    const Router::Config cfg{0, Router::DEF_INPUT_PROC, 0, Router::DEF_LOC_BW, 0, 2};
    Router router{IPAddress{5, 0}, 0, cfg};
    router.connectRouter(&rtr2);
    RoutingTable rt;
    rt.setNextHopIP(rtr2.getIP(), rtr2.getIP());
    router.setRoutingTable(std::move(rt));

    for (size_t i = 0; i < 5; ++i) {
        router.receivePacket(Packet{100, i, 10, IPAddress{5, 1}, IPAddress{10, 1}, TICK});
    }

    router.processInputBuffer(1);
    const size_t sent = router.processOutputBuffers(1);

    EXPECT_EQ(sent, 2);
    EXPECT_EQ(rtr2.getPacketsReceived(), 2);
    EXPECT_EQ(router.getNeighborBufferUsage(rtr2.getIP()), 3);
}

TEST_F(RouterTest, ProcessOutputBuffers_ExpiredPacket) {
    connectAndRoute();

    rtr1.receivePacket(Packet{100, 0, 5, IPAddress{5, 1}, IPAddress{10, 1}, 5});
    rtr1.processInputBuffer(1);
    const size_t sent = rtr1.processOutputBuffers(10);

    EXPECT_EQ(sent, 0);
    EXPECT_EQ(rtr1.getPacketsTimedOut(), 1);
    EXPECT_EQ(rtr2.getPacketsReceived(), 0);
}

// =============== Local buffer processing tests ===============
TEST_F(RouterTest, ProcessLocalBuffer_Valid) {
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 10));
    const IPAddress dst{5, 10};
    const IPAddress src{10, 5};

    for (size_t i = 0; i < 5; ++i) {
        rtr1.receivePacket(Packet{100, i, 5, src, dst, TICK});
    }

    rtr1.processInputBuffer(1);
    rtr1.processLocalBuffer(1);

    EXPECT_EQ(rtr1.getPacketsDelivered(), 5);
    EXPECT_EQ(rtr1.getTerminal(dst)->getPacketsReceived(), 5);
}

TEST_F(RouterTest, ProcessLocalBuffer_NoTerminal) {
    const IPAddress dst{5, 99};
    const IPAddress src{10, 5};

    rtr1.receivePacket(Packet{100, 0, 5, src, dst, TICK});
    rtr1.processInputBuffer(1);
    rtr1.processLocalBuffer(1);

    EXPECT_EQ(rtr1.getPacketsDropped(), 1);
    EXPECT_EQ(rtr1.getPacketsDelivered(), 0);
}

TEST_F(RouterTest, ProcessLocalBuffer_BandwidthLimit) {
    const Router::Config cfg{0, Router::DEF_INPUT_PROC, 0, 3, 0, Router::DEF_OUTPUT_BW};
    Router router{IPAddress{5, 0}, 0, cfg};
    auto t    = std::make_unique<Terminal>(&router, 10);
    auto* raw = t.get();
    router.connectTerminal(std::move(t));

    const IPAddress dst{5, 10};
    const IPAddress src{10, 5};

    for (size_t i = 0; i < 6; ++i) {
        router.receivePacket(Packet{100, i, 6, src, dst, TICK});
    }

    router.processInputBuffer(1);
    const size_t delivered = router.processLocalBuffer(1);

    EXPECT_EQ(delivered, 3);
    EXPECT_EQ(raw->getPacketsReceived(), 3);
}

TEST_F(RouterTest, ProcessLocalBuffer_ExpiredPacket) {
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 10));
    const IPAddress dst{5, 10};
    const IPAddress src{10, 5};

    rtr1.receivePacket(Packet{100, 0, 5, src, dst, 5});
    rtr1.processInputBuffer(1);
    rtr1.processLocalBuffer(10);

    EXPECT_EQ(rtr1.getPacketsTimedOut(), 1);
    EXPECT_EQ(rtr1.getPacketsDelivered(), 0);
}

// =============== Tick tests ===============
TEST_F(RouterTest, Tick_FullCycle) {
    rtr1.connectTerminal(std::make_unique<Terminal>(&rtr1, 10));
    connectAndRoute();

    rtr1.receivePacket(Packet{100, 0, 2, IPAddress{5, 1}, IPAddress{5, 10}, TICK});
    rtr1.receivePacket(Packet{200, 0, 2, IPAddress{5, 1}, IPAddress{10, 1}, TICK});

    rtr1.tick(1);
    rtr1.tick(1);

    EXPECT_EQ(rtr2.getPacketsReceived(), 1);
    EXPECT_EQ(rtr1.getTerminal(IPAddress{5, 10})->getPacketsReceived(), 1);
}

TEST_F(RouterTest, Tick_TerminalsAreTicked) {
    connectAndRoute();
    auto t = std::make_unique<Terminal>(&rtr1, 10);
    Terminal* t1 = t.get();
    rtr1.connectTerminal(std::move(t));

    t1->sendPage(2, IPAddress{10, 1}, TICK);

    rtr1.tick(1);

    EXPECT_GT(rtr1.getPacketsReceived(), 0);
}

// =============== Configuration tests ===============
TEST_F(RouterTest, SetInProcCap) {
    rtr1.setInProcCap(25);
    EXPECT_EQ(rtr1.getInProcCap(), 25);
}

TEST_F(RouterTest, SetLocBufferBW) {
    rtr1.setLocBufferBW(15);
    EXPECT_EQ(rtr1.getLocBufferBW(), 15);
}

TEST_F(RouterTest, SetOutBufferBW) {
    rtr1.setOutBufferBW(8);
    EXPECT_EQ(rtr1.getOutBufferBW(), 8);
}

// =============== Utilities tests ===============
TEST_F(RouterTest, ToString) {
    const std::string str = rtr1.toString();
    EXPECT_NE(str.find("Router"), std::string::npos);
    EXPECT_NE(str.find('5'), std::string::npos);
}

TEST_F(RouterTest, StreamOperator) {
    std::ostringstream oss;
    oss << rtr1;
    EXPECT_NE(oss.str().find("Router"), std::string::npos);
}

TEST_F(RouterTest, GetNeighborBufferUsage_NotConnected) {
    EXPECT_EQ(rtr1.getNeighborBufferUsage(IPAddress{99, 0}), 0);
}
