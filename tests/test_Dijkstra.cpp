#include <gtest/gtest.h>
#include "algorithms/Dijkstra.h"
#include "core/Router.h"

class DijkstraTestFixture : public ::testing::Test {
protected:
    List<std::unique_ptr<Router>> routers;
    List<const Router*> pRouters;

    Router* createRouter(uint8_t routerID) {
        auto r = std::make_unique<Router>(IPAddress(routerID));
        routers.pushBack(std::move(r));

        Router* ptr = routers.getTail().get();
        pRouters.pushBack(ptr);
        return ptr;
    }

    void connectRouters(Router* r1, Router* r2) {
        r1->connectRouter(r2);
        r2->connectRouter(r1);
    }
};

// =============== Basic tests ===============
TEST_F(DijkstraTestFixture, Basic_OneRouter) {
    const Router* r1 = createRouter(1);

    const RoutingTable table1 = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table1.size(), 0);
}

TEST_F(DijkstraTestFixture, Basic_TwoRoutersNotConnected) {
    const Router* r1 = createRouter(1);
    const Router* r2 = createRouter(2);

    const RoutingTable table1 = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());
    const RoutingTable table2 = DijkstraAlgorithm::computeRoutingTable(pRouters, r2->getIP());

    EXPECT_EQ(table1.size(), 0);
    EXPECT_EQ(table2.size(), 0);
}

TEST_F(DijkstraTestFixture, Basic_TwoRoutersConnected) {
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);

    connectRouters(r1, r2);

    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.size(), 1);
    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r2->getIP());
}

// =============== Line topology tests ===============
TEST_F(DijkstraTestFixture, Line_ThreeRouters) {
    // Topology: R1 -- R2 -- R3
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);

    connectRouters(r1, r2);
    connectRouters(r2, r3);

    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.size(), 2);

    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r2->getIP());

    EXPECT_EQ(table.getNextHopIP(r3->getIP()), r2->getIP());
}

TEST_F(DijkstraTestFixture, Line_FourRouters) {
    // Topology: R1 -- R2 -- R3 -- R4
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);
    Router* r4 = createRouter(4);

    connectRouters(r1, r2);
    connectRouters(r2, r3);
    connectRouters(r3, r4);

    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.size(), 3);

    // Every router should route through r2 to reach the others
    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r2->getIP());
    EXPECT_EQ(table.getNextHopIP(r3->getIP()), r2->getIP());
    EXPECT_EQ(table.getNextHopIP(r4->getIP()), r2->getIP());
}

// =============== Triangle topology tests ===============
TEST_F(DijkstraTestFixture, TriangleTopology) {
    // Topology: R1 -- R2
    //            \    /
    //              R3
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);

    connectRouters(r1, r2);
    connectRouters(r2, r3);
    connectRouters(r3, r1);

    // Simulate some traffic in R1->R2
    for (int i = 0; i < 3; i++) {
        r1->receivePacket(Packet(10, i, 4, r1->getIP(), r2->getIP(), 10));
    }
    auto rt = RoutingTable();
    rt.setNextHopIP(r2->getIP(), r2->getIP());  // Force R1 to route to R2 directly
    r1->setRoutingTable(std::move(rt));
    r1->processInputBuffer(1);

    // Calculate routes from R1
    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.size(), 2);

    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r3->getIP());  // R1->R2 should go through R3
    EXPECT_EQ(table.getNextHopIP(r3->getIP()), r3->getIP());
}

// =============== Star topology tests ===============
TEST_F(DijkstraTestFixture, StarTopology) {
    // Topology: R2, R3, R4 connected to R1
    Router* r1 = createRouter(1);  // Center
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);
    Router* r4 = createRouter(4);

    connectRouters(r1, r2);
    connectRouters(r1, r3);
    connectRouters(r1, r4);

    // From R1: direct to all
    RoutingTable table1 = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());
    EXPECT_EQ(table1.size(), 3);
    EXPECT_EQ(table1.getNextHopIP(r2->getIP()), r2->getIP());
    EXPECT_EQ(table1.getNextHopIP(r3->getIP()), r3->getIP());
    EXPECT_EQ(table1.getNextHopIP(r4->getIP()), r4->getIP());

    // From R2: direct to R1, to R3 and R4 through R1
    RoutingTable table2 = DijkstraAlgorithm::computeRoutingTable(pRouters, r2->getIP());
    EXPECT_EQ(table2.size(), 3);
    EXPECT_EQ(table2.getNextHopIP(r1->getIP()), r1->getIP());
    EXPECT_EQ(table2.getNextHopIP(r3->getIP()), r1->getIP());
    EXPECT_EQ(table2.getNextHopIP(r4->getIP()), r1->getIP());
}

// =============== Square topology tests ===============
TEST_F(DijkstraTestFixture, SquareTopology) {
    // Topology:  R1 -- R2
    //            |     |
    //            R4 -- R3
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);
    Router* r4 = createRouter(4);

    connectRouters(r1, r2);
    connectRouters(r2, r3);
    connectRouters(r3, r4);
    connectRouters(r4, r1);

    // From R1
    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.size(), 3);

    // To R2 and R4: direct
    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r2->getIP());
    EXPECT_EQ(table.getNextHopIP(r4->getIP()), r4->getIP());

    // To R3: can go through R2 or R4
    const IPAddress nextHop = table.getNextHopIP(r3->getIP());
    EXPECT_TRUE(nextHop == r2->getIP() || nextHop == r4->getIP());
}

TEST_F(DijkstraTestFixture, ComplexDiamondTopology) {
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);
    Router* r4 = createRouter(4);

    connectRouters(r1, r2);  // C1-2
    connectRouters(r1, r3);  // C1-3
    connectRouters(r1, r4);  // C1-4
    connectRouters(r2, r4);  // C2-4
    connectRouters(r3, r4);  // C3-4

    // Simulate congestion on R1->R4
    RoutingTable rt1;
    rt1.setNextHopIP(r4->getIP(), r4->getIP());
    r1->setRoutingTable(std::move(rt1));
    for (int i = 0; i < 20; i++) {
        r1->receivePacket(Packet(10, i, 20, r1->getIP(), r4->getIP(), 10));
    }
    r1->processInputBuffer(1);

    // Simulate moderate congestion on R2->R4
    RoutingTable rt2;
    rt2.setNextHopIP(r4->getIP(), r4->getIP());
    r2->setRoutingTable(std::move(rt2));
    for (int i = 0; i < 5; i++) {
        r2->receivePacket(Packet(10, i, 5, r2->getIP(), r4->getIP(), 10));
    }
    r2->processInputBuffer(1);

    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());
    EXPECT_EQ(table.getNextHopIP(r4->getIP()), r3->getIP());
    EXPECT_EQ(table.getNextHopIP(r2->getIP()), r2->getIP());

    table = DijkstraAlgorithm::computeRoutingTable(pRouters, r2->getIP());
    EXPECT_EQ(table.getNextHopIP(r3->getIP()), r1->getIP());
    EXPECT_EQ(table.getNextHopIP(r4->getIP()), r1->getIP());
}

TEST_F(DijkstraTestFixture, UnreachableNode) {
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    connectRouters(r1, r2);

    Router* rIsolated = createRouter(99);

    RoutingTable table = DijkstraAlgorithm::computeRoutingTable(pRouters, r1->getIP());

    EXPECT_EQ(table.getNextHopIP(rIsolated->getIP()), IPAddress());
}

TEST_F(DijkstraTestFixture, ComputeAllTables_LinearTopology) {
    // 1Topology: R1 -- R2 -- R3
    Router* r1 = createRouter(1);
    Router* r2 = createRouter(2);
    Router* r3 = createRouter(3);

    connectRouters(r1, r2);
    connectRouters(r2, r3);

    List<RoutingTable> allTables;

    DijkstraAlgorithm::computeAllRoutingTables(pRouters, allTables);

    ASSERT_EQ(allTables.size(), 3);

    // Table for R1 (Index 0)
    EXPECT_EQ(allTables[0].getNextHopIP(r3->getIP()), r2->getIP());

    // Table for R2 (Index 1)
    EXPECT_EQ(allTables[1].getNextHopIP(r1->getIP()), r1->getIP());
    EXPECT_EQ(allTables[1].getNextHopIP(r3->getIP()), r3->getIP());

    // Table for R3 (Index 2)
    EXPECT_EQ(allTables[2].getNextHopIP(r1->getIP()), r2->getIP());
}
