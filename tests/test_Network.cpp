#include <gtest/gtest.h>

#include "core/Network.h"
#include "core/Terminal.h"

// =============== Fixture ===============
class NetworkTest : public testing::Test {
protected:
    static constexpr size_t ROUTER_COUNT  = Network::DEF_ROUTERS_COUNT;
    static constexpr size_t MAX_TERMINALS = Network::DEF_MAX_TERMINALS;
    static constexpr size_t COMPLEXITY    = Network::DEF_COMPLEXITY;
    static constexpr float PROBABILITY    = Network::DEF_PROBABILITY;
    static constexpr size_t PAGE_LEN      = Network::DEF_MAX_PAGE_LEN;

    Network net{};
};

// =============== Constructor tests ===============
TEST_F(NetworkTest, Constructor_InstantiatesObjects) {
    size_t totalTerminals = 0;
    for (const auto& routers = net.getRouters(); const auto* rtr : routers) {
        totalTerminals += rtr->getTerminalCount();
    }
    EXPECT_EQ(net.getRouters().size(), ROUTER_COUNT);
    EXPECT_EQ(totalTerminals, ROUTER_COUNT * MAX_TERMINALS);
}

TEST_F(NetworkTest, Constructor_DefaultParameters) {
    const Network::Config c{10, 3, 5, 0.5, 3};
    const Network n{c};
    for (const auto* rtr : n.getRouters()) {
        for (const auto trm : rtr->getTerminals()) {
            EXPECT_EQ(trm->getTrafficProbability(), 0.5f);
            EXPECT_EQ(trm->getMaxPageLength(), 3);
        }
        EXPECT_GE(rtr->getRouterCount(), 1);
    }
}

TEST_F(NetworkTest, Constructor_MinimalConnectivity) {
    const Network::Config c{10, 3, 0, 0.5, 3};
    const Network n{c};
    for (const auto* rtr : net.getRouters()) {
        EXPECT_GE(rtr->getRouterCount(), 1);
    }
}

TEST_F(NetworkTest, Constructor_ZeroTerminals) {
    const Network::Config c{4, 0, 0, 0.5f, 5};
    const Network n{c};
    for (const auto* rtr : n.getRouters()) {
        EXPECT_EQ(rtr->getTerminalCount(), 0);
    }
}

TEST_F(NetworkTest, Constructor_SingleRouter) {
    const Network::Config c{1, 2, 0, 0.5f, 5};
    const Network n{c};
    EXPECT_EQ(n.getRouters().size(), 1);
    EXPECT_EQ(n.getRouters()[0]->getRouterCount(), 0);
}

// =============== establishLink tests ===============
TEST(NetworkStaticTest, EstablishLink_Bidirectional) {
    Router rtrA{IPAddress{1, 0}};
    Router rtrB{IPAddress{2, 0}};

    Network::establishLink(&rtrA, &rtrB);

    EXPECT_EQ(rtrA.getRouterCount(), 1);
    EXPECT_EQ(rtrB.getRouterCount(), 1);
}

TEST(NetworkStaticTest, EstablishLink_SameRouter_NoOp) {
    Router rtrA{IPAddress{1, 0}};

    EXPECT_NO_THROW(Network::establishLink(&rtrA, &rtrA));
    EXPECT_EQ(rtrA.getRouterCount(), 0);
}

TEST(NetworkStaticTest, EstablishLink_AlreadyConnected_NoOp) {
    Router rtrA{IPAddress{1, 0}};
    Router rtrB{IPAddress{2, 0}};

    Network::establishLink(&rtrA, &rtrB);
    Network::establishLink(&rtrA, &rtrB);
    EXPECT_EQ(rtrA.getRouterCount(), 1);
    EXPECT_EQ(rtrB.getRouterCount(), 1);
}

// =============== simulate tests ===============
TEST_F(NetworkTest, Simulate_ZeroTicks_NoOp) {
    EXPECT_NO_THROW(net.simulate(0));
}

TEST_F(NetworkTest, Simulate_PacketsReceivedAfterSimulation) {
    const Network::Config c{4, 3, 1, 1.0f, 3};
    Network n{c};
    n.simulate(10);

    size_t totalReceived = 0;
    for (const auto* rtr : n.getRouters()) {
        totalReceived += rtr->getPacketsReceived();
    }
    EXPECT_GT(totalReceived, 0);
}

TEST_F(NetworkTest, Simulate_ZeroProbability_NoTrafficGenerated) {
    const Network::Config c{4, 3, 1, 0.0f, 5};
    Network n{c};
    n.simulate(20);

    size_t totalReceived = 0;
    for (const auto* rtr : n.getRouters()) {
        totalReceived += rtr->getPacketsReceived();
    }
    EXPECT_EQ(totalReceived, 0);
}

TEST_F(NetworkTest, Simulate_RouteRecalculation_EveryFiveTicks) {
    EXPECT_NO_THROW(net.simulate(20));
}

TEST_F(NetworkTest, Simulate_MultipleCallsAreStable) {
    net.simulate(5);
    EXPECT_NO_THROW(net.simulate(5));
    EXPECT_NO_THROW(net.simulate(5));
}

TEST_F(NetworkTest, Simulate_PacketsDeliveredLessThanReceived) {
    const Network::Config c{4, 3, 1, 1.0f, 3};
    Network n{c};
    n.simulate(20);

    for (const auto* rtr : n.getRouters()) {
        EXPECT_LE(rtr->getPacketsDelivered(), rtr->getPacketsReceived());
    }
}

TEST_F(NetworkTest, Simulate_DroppedPlusForwardedPlusDelivered_ConsistentWithReceived) {
    const Network::Config c{4, 3, 1, 1.0f, 3};
    Network n{c};
    n.simulate(20);

    for (const auto* rtr : n.getRouters()) {
        const size_t accounted = rtr->getPacketsDropped() + rtr->getPacketsForwarded() +
                                 rtr->getPacketsDelivered() + rtr->getPacketsTimedOut();
        EXPECT_LE(accounted, rtr->getPacketsReceived());
    }
}

// =============== Stress tests ===============
TEST(NetworkStressTest, LargeNetwork_Simulate) {
    const Network::Config c{20, 10, 5, 0.3f, 10};
    EXPECT_NO_THROW(Network{c}.simulate(30));
}

TEST(NetworkStressTest, MinimalNetwork_TwoRouters) {
    const Network::Config c{2, 2, 0, 0.5f, 5};
    Network n{c};
    EXPECT_NO_THROW(n.simulate(10));
}

TEST(NetworkStressTest, HighLoad_ManyTerminals) {
    const Network::Config c{6, 8, 2, 1.0f, 10};
    Network n{c};
    EXPECT_NO_THROW(n.simulate(100));
}

TEST(NetworkStressTest, LongSimulation_TickCounterStable) {
    const Network::Config c{5, 4, 1, 0.5f, 5};
    Network n{c};
    EXPECT_NO_THROW(n.simulate(200));
}
