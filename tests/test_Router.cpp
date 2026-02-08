#include <gtest/gtest.h>
#include "core/Router.h"
#include "core/Terminal.h"

// =============== RouterConnection tests ===============
TEST(RouterConnectionConstructor, ValidConstructor) {
    Router routerA{IPAddress{5, 0}};
    Router routerB{IPAddress{10, 0}};

    RouterConnection connection{&routerB};

    EXPECT_EQ(connection.router, &routerB);
    EXPECT_EQ(connection.outputBuffer.getDestinationIP(), routerB.getIP());
}

// =============== Constructor tests ===============
TEST(RouterConstructor, ValidConstructor) {
    const IPAddress routerIP{5, 0};  // Terminal ID must be 0

    const Router router{routerIP};

    EXPECT_EQ(router.getIP().getRouterIP(), 5);
    EXPECT_EQ(router.getPacketsReceived(), 0);
    EXPECT_EQ(router.getConnectedTerminals(), 0);
    EXPECT_EQ(router.getConnectedRouters(), 0);
    EXPECT_EQ(router.getInternalBW(), 10);
    EXPECT_EQ(router.getExternalBW(), 5);
}

TEST(RouterConstructor, ConstructorInvalidIP) {
    const IPAddress invalidIP{5, 10};  // Terminal ID != 0

    EXPECT_THROW(Router{invalidIP}, std::invalid_argument);
}

TEST(RouterConstructor, ConstructorWithCustomBW) {
    const IPAddress routerIP{10, 0};

    const Router router{routerIP, 300, 20, 10};

    EXPECT_EQ(router.getInternalBW(), 20);
    EXPECT_EQ(router.getExternalBW(), 10);
}

// =============== Connection Management tests ===============
TEST(RouterConnectionManagement, ConnectTerminalValid) {
    Router router{IPAddress{5, 0}};
    auto terminal = std::make_unique<Terminal>(IPAddress{5, 10}, &router);

    EXPECT_EQ(router.getConnectedTerminals(), 0);
    EXPECT_NO_THROW(router.connectTerminal(std::move(terminal)));
    EXPECT_EQ(router.getConnectedTerminals(), 1);
}

TEST(RouterConnectionManagement, ConnectTerminalNull) {
    Router router{IPAddress{5, 0}};

    EXPECT_THROW(router.connectTerminal(nullptr), std::invalid_argument);
}

TEST(RouterConnectionManagement, ConnectTerminalDuplicate) {
    Router router{IPAddress{5, 0}};
    auto terminal = std::make_unique<Terminal>(IPAddress{5, 10}, &router);

    EXPECT_NO_THROW(router.connectTerminal(std::move(terminal)));
    EXPECT_THROW(router.connectTerminal(std::move(terminal)), std::invalid_argument);
}

TEST(RouterConnectionManagement, ConnectTerminalWrongRouter) {
    Router router{IPAddress{5, 0}};
    Router dummyRouter{IPAddress{10, 0}};
    auto terminal = std::make_unique<Terminal>(IPAddress{10, 5}, &dummyRouter);

    EXPECT_THROW(router.connectTerminal(std::move(terminal)), std::invalid_argument);
}

TEST(RouterConnectionManagement, ConnectMultipleTerminals) {
    Router router{IPAddress{5, 0}};
    auto t1 = std::make_unique<Terminal>(IPAddress{5, 10}, &router);
    auto t2 = std::make_unique<Terminal>(IPAddress{5, 20}, &router);
    auto t3 = std::make_unique<Terminal>(IPAddress{5, 30}, &router);

    router.connectTerminal(std::move(t1));
    router.connectTerminal(std::move(t2));
    router.connectTerminal(std::move(t3));

    EXPECT_EQ(router.getConnectedTerminals(), 3);
}

TEST(RouterConnectionManagement, ConnectRouterValid) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};

    EXPECT_NO_THROW(router1.connectRouter(&router2));
    EXPECT_EQ(router1.getConnectedRouters(), 1);
}

TEST(RouterConnectionManagement, ConnectRouterNull) {
    Router router{IPAddress{5, 0}};

    EXPECT_THROW(router.connectRouter(nullptr), std::invalid_argument);
}

TEST(RouterConnectionManagement, ConnectRouterDuplicate) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};

    router1.connectRouter(&router2);

    EXPECT_THROW(router1.connectRouter(&router2), std::invalid_argument);
}

TEST(RouterConnectionManagement, ConnectMultipleRouters) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};
    Router router3{IPAddress{15, 0}};
    Router router4{IPAddress{20, 0}};

    router1.connectRouter(&router2);
    router1.connectRouter(&router3);
    router1.connectRouter(&router4);

    EXPECT_EQ(router1.getConnectedRouters(), 3);
}

TEST(RouterConnectionManagement, DisconnectRouter) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};

    router1.connectRouter(&router2);
    EXPECT_EQ(router1.getConnectedRouters(), 1);

    EXPECT_TRUE(router1.disconnectRouter(&router2));
    EXPECT_EQ(router1.getConnectedRouters(), 0);
}

TEST(RouterConnectionManagement, DisconnectRouterNotConnected) {
    Router router1{IPAddress{5, 0}};
    const Router router2{IPAddress{10, 0}};

    EXPECT_FALSE(router1.disconnectRouter(&router2));
}

// =============== Packet Reception tests ===============
TEST(RouterPacketReception, ReceivePacketSuccess) {
    Router router{IPAddress{5, 0}};
    const IPAddress origin{10, 5};
    const IPAddress dest{15, 10};

    const Packet packet{100, 0, 5, 0, dest, origin};

    EXPECT_TRUE(router.receivePacket(packet));
    EXPECT_EQ(router.getPacketsReceived(), 1);
}

TEST(RouterPacketReception, ReceivePacketBufferFull) {
    Router router{IPAddress{5, 0}, 2};  // Capacity 2

    const IPAddress origin{10, 5};
    const IPAddress dest{15, 10};

    EXPECT_TRUE(router.receivePacket(Packet{100, 0, 5, 0, dest, origin}));
    EXPECT_TRUE(router.receivePacket(Packet{100, 1, 5, 0, dest, origin}));
    EXPECT_FALSE(router.receivePacket(Packet{100, 2, 5, 0, dest, origin}));
    EXPECT_EQ(router.getPacketsReceived(), 2);
    EXPECT_EQ(router.getPacketsDropped(), 1);
}

// =============== Processing tests ===============
TEST(RouterProcessing, ProcessInputBufferToTerminal) {
    Router router{IPAddress{5, 0}};
    Router dummyRouter{IPAddress{6, 0}};
    auto terminal = std::make_unique<Terminal>(IPAddress{5, 10}, &router);

    router.connectTerminal(std::move(terminal));
    IPAddress origin{10, 5};
    IPAddress dest{5, 10};
    Packet packet{100, 0, 5, 0, dest, origin};

    router.receivePacket(packet);
    EXPECT_EQ(router.getLocalBufferUsage(), 0);

    router.processInputBuffer();
    EXPECT_EQ(router.getLocalBufferUsage(), 1);
}

TEST(RouterProcessing, ProcessInputBufferToNeighbor) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};

    router1.connectRouter(&router2);
    IPAddress destination{10, 0};
    IPAddress nextHop{10, 0};
    RoutingTable routingTable{};
    routingTable.setNextHopIP(destination, nextHop);
    router1.setRoutingTable(routingTable);

    IPAddress origin{5, 5};
    IPAddress dest{10, 15};
    Packet packet{100, 0, 5, 0, dest, origin};

    router1.receivePacket(packet);
    router1.processInputBuffer();

    EXPECT_EQ(router1.getNeighborBufferUsage(nextHop), 1);
}

TEST(RouterProcessing, ProcessInputBufferNoRoute) {
    Router router{IPAddress{5, 0}};

    const IPAddress origin{5, 5};
    const IPAddress dest{99, 1};  // No route to 99
    const Packet packet{100, 0, 5, 0, dest, origin};

    router.receivePacket(packet);
    router.processInputBuffer();

    EXPECT_EQ(router.getPacketsDropped(), 1);
}

TEST(RouterProcessing, ProcessInputBufferBandwidthLimit) {
    Router router{IPAddress{5, 0}, 100, 2, 5};  // internalBW = 2

    const IPAddress origin{10, 5};
    const IPAddress dest{15, 10};

    for (int i = 0; i < 5; ++i) {
        router.receivePacket(Packet{100, i, 10, 0, dest, origin});
    }

    EXPECT_EQ(router.processInputBuffer(), 2);
    EXPECT_EQ(router.getPacketsDropped(), 2);
}

TEST(RouterProcessing, ProcessOutputBuffersBasic) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};
    router1.connectRouter(&router2);

    RoutingTable routingTable{};
    routingTable.setNextHopIP(router2.getIP(), router2.getIP());
    router1.setRoutingTable(routingTable);

    const IPAddress origin{5, 5};
    const IPAddress dest{10, 15};

    router1.receivePacket(Packet{100, 0, 5, 0, dest, origin});
    router1.processInputBuffer();
    router1.processOutputBuffers();

    EXPECT_EQ(router2.getPacketsReceived(), 1);
}

TEST(RouterProcessing, ProcessOutputBuffersMultipleConnections) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};
    Router router3{IPAddress{15, 0}};

    router1.connectRouter(&router2);
    router1.connectRouter(&router3);

    RoutingTable routingTable{};
    routingTable.setNextHopIP(router2.getIP(), router2.getIP());
    routingTable.setNextHopIP(router3.getIP(), router3.getIP());
    router1.setRoutingTable(routingTable);

    IPAddress origin{5, 5};

    router1.receivePacket(Packet{100, 0, 5, 0, IPAddress{10, 1}, origin});
    router1.receivePacket(Packet{200, 0, 5, 0, IPAddress{15, 1}, origin});

    router1.processInputBuffer();
    router1.processOutputBuffers();

    EXPECT_EQ(router2.getPacketsReceived(), 1);
    EXPECT_EQ(router3.getPacketsReceived(), 1);
}

TEST(RouterProcessing, ProcessOutputBuffersBandwidthLimit) {
    Router router1{IPAddress{5, 0}, 200, 10, 2};  // externalBW = 2
    Router router2{IPAddress{10, 0}};
    router1.connectRouter(&router2);

    RoutingTable routingTable{};
    routingTable.setNextHopIP(router2.getIP(), router2.getIP());
    router1.setRoutingTable(routingTable);

    IPAddress origin{5, 5};
    IPAddress dest{10, 1};

    for (int i = 0; i < 5; ++i) {
        router1.receivePacket(Packet{100, i, 10, 0, dest, origin});
    }

    router1.processInputBuffer();

    EXPECT_EQ(router1.processOutputBuffers(), 2);
    EXPECT_EQ(router2.getPacketsReceived(), 2);
}

TEST(RouterProcessing, ProcessLocalBufferValid) {
    Router router{IPAddress{5, 0}, 100, 10, 5};
    auto terminal = std::make_unique<Terminal>(IPAddress{5, 10}, &router);
    Terminal* terminalRawPtr = terminal.get();  // Keep raw pointer for later verification
    router.connectTerminal(std::move(terminal));

    const IPAddress origin{10, 5};
    const IPAddress dest{5, 10};

    for (int i = 0; i < 15; ++i) {
        router.receivePacket(Packet{100, i, 15, 0, dest, origin});
    }

    EXPECT_EQ(router.processInputBuffer(), 10);
    EXPECT_EQ(router.processLocalBuffer(), 10);
    EXPECT_EQ(terminalRawPtr->getReceivedPackets(), 10);
}

TEST(RouterProcessing, ProcessLocalBufferInvalid) {
    Router router{IPAddress{5, 0}, 100, 10, 5};

    const IPAddress origin{10, 5};
    const IPAddress dest{5, 10};

    for (int i = 0; i < 15; ++i) {
        router.receivePacket(Packet{100, i, 15, 0, dest, origin});
    }

    EXPECT_EQ(router.processInputBuffer(), 10);
    EXPECT_EQ(router.processLocalBuffer(), 0);
    EXPECT_EQ(router.getPacketsDropped(), 10);
}

TEST(RouterProcessing, TickFullCycle) {
    Router router1{IPAddress{5, 0}};
    Router router2{IPAddress{10, 0}};
    auto terminal = std::make_unique<Terminal>(IPAddress{5, 10}, &router1);
    Terminal* terminalRawPtr = terminal.get();  // Keep raw pointer for later verification

    router1.connectTerminal(std::move(terminal));
    router1.connectRouter(&router2);

    RoutingTable routingTable{};
    routingTable.setNextHopIP(router2.getIP(), router2.getIP());
    router1.setRoutingTable(routingTable);

    IPAddress origin{5, 5};

    router1.receivePacket(Packet{100, 0, 2, 0, IPAddress{5, 10}, origin});
    router1.receivePacket(Packet{200, 0, 2, 0, IPAddress{10, 1}, origin});

    router1.tick();

    EXPECT_EQ(router1.getPacketsReceived(), 2);
    EXPECT_EQ(router2.getPacketsReceived(), 1);
    EXPECT_EQ(terminalRawPtr->getReceivedPackets(), 1);
}

// =============== Configuration tests ===============
TEST(RouterConfiguration, SetInternalBW) {
    Router router{IPAddress{5, 0}};

    router.setInternalBW(25);

    EXPECT_EQ(router.getInternalBW(), 25);
}

TEST(RouterConfiguration, SetExternalBW) {
    Router router{IPAddress{5, 0}};

    router.setExternalBW(15);

    EXPECT_EQ(router.getExternalBW(), 15);
}

// =============== Utilities tests ===============
TEST(RouterUtilities, ToString) {
    const Router router{IPAddress{5, 0}};

    std::string str = router.toString();

    EXPECT_NE(str.find("Router"), std::string::npos);
    EXPECT_NE(str.find("5"), std::string::npos);
}
