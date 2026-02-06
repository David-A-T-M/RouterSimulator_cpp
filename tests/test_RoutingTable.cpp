#include <gtest/gtest.h>
#include "core/RoutingTable.h"

TEST(RoutingTableTest, SetEmptyTable) {
    RoutingTable rt;
    EXPECT_EQ(rt.size(), 0);

    rt.setNextHopIP(IPAddress{1, 0}, IPAddress{2, 0});
    EXPECT_EQ(rt.size(), 1);
}

TEST(RoutingTableTest, SetMultipleEntries) {
    RoutingTable rt;
    rt.setNextHopIP(IPAddress{1, 0}, IPAddress{2, 0});
    rt.setNextHopIP(IPAddress{3, 0}, IPAddress{4, 0});
    EXPECT_EQ(rt.size(), 2);
}

TEST(RoutingTableTest, UpdateExistingEntry) {
    RoutingTable rt;
    rt.setNextHopIP(IPAddress{1, 0}, IPAddress{2, 0});
    EXPECT_EQ(rt.size(), 1);

    rt.setNextHopIP(IPAddress{1, 0}, IPAddress{3, 0});
    EXPECT_EQ(rt.size(), 1);
}
