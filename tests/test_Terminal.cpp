#include <gtest/gtest.h>

#include "core/Router.h"
#include "core/Terminal.h"

class TerminalTest : public testing::Test {
protected:
    Router rtr{IPAddress{5, 0}};
    IPAddress dst{5, 10};
    IPAddress src{10, 20};
    static constexpr size_t TERMINAL_ID = 10;
    static constexpr size_t TICK        = 100;
    Terminal trm{&rtr, TERMINAL_ID};
};

// =============== Constructor tests ===============
TEST_F(TerminalTest, Constructor_Default) {
    const Terminal terminal(&rtr, TERMINAL_ID);

    EXPECT_EQ(terminal.getTerminalIP(), IPAddress(rtr.getIP().getRouterIP(), TERMINAL_ID));
    EXPECT_EQ(terminal.getOutputBW(), Terminal::DEF_OUTPUT_BW);
    EXPECT_EQ(terminal.getInternalProc(), Terminal::DEF_INPUT_PROC);
    EXPECT_EQ(terminal.getPagesCreated(), 0);
    EXPECT_EQ(terminal.getPagesSent(), 0);
    EXPECT_EQ(terminal.getPagesDropped(), 0);
    EXPECT_EQ(terminal.getPagesCompleted(), 0);
    EXPECT_EQ(terminal.getPagesTimedOut(), 0);
    EXPECT_EQ(terminal.getPacketsGenerated(), 0);
    EXPECT_EQ(terminal.getPacketsSent(), 0);
    EXPECT_EQ(terminal.getPacketsOutDropped(), 0);
    EXPECT_EQ(terminal.getPacketsOutTimedOut(), 0);
    EXPECT_EQ(terminal.getPacketsOutPending(), 0);
    EXPECT_EQ(terminal.getPacketsReceived(), 0);
    EXPECT_EQ(terminal.getPacketsInTimedOut(), 0);
    EXPECT_EQ(terminal.getPacketsInDropped(), 0);
    EXPECT_EQ(terminal.getPacketsSuccProcessed(), 0);
    EXPECT_EQ(terminal.getPacketsInPending(), 0);
}

TEST_F(TerminalTest, Constructor_CustomConfig) {
    const Terminal::Config cfg = {0, 12, 0, 8};
    const Terminal terminal(&rtr, TERMINAL_ID, cfg);

    EXPECT_EQ(terminal.getOutputBW(), cfg.outputBW);
    EXPECT_EQ(terminal.getInternalProc(), cfg.inProcCap);
}

TEST_F(TerminalTest, Constructor_InvalidID) {
    EXPECT_THROW(Terminal(&rtr, 0), std::invalid_argument);
}

// =============== Transmission tests ===============
TEST_F(TerminalTest, SendPageBasic) {
    EXPECT_TRUE(trm.sendPage(5, src, TICK));

    EXPECT_EQ(trm.getPagesCreated(), 1);
    EXPECT_EQ(trm.getPagesSent(), 1);
    EXPECT_EQ(trm.getPagesDropped(), 0);
    EXPECT_EQ(trm.getPagesCompleted(), 0);
    EXPECT_EQ(trm.getPagesTimedOut(), 0);
    EXPECT_EQ(trm.getPacketsGenerated(), 5);
    EXPECT_EQ(trm.getPacketsSent(), 0);
    EXPECT_EQ(trm.getPacketsOutPending(), 5);
}

TEST_F(TerminalTest, SendMultiplePages) {
    trm.sendPage(5, src, TICK);
    trm.sendPage(10, src, TICK);
    trm.sendPage(3, src, TICK);

    EXPECT_EQ(trm.getPagesCreated(), 3);
    EXPECT_EQ(trm.getPagesSent(), 3);
    EXPECT_EQ(trm.getPacketsGenerated(), 18);
    EXPECT_EQ(trm.getPacketsSent(), 0);
    EXPECT_EQ(trm.getPacketsOutPending(), 18);
}

TEST_F(TerminalTest, SendPageBufferOverflow) {
    const Terminal::Config cfg = {0, 12, 7, 8};
    Terminal terminal(&rtr, TERMINAL_ID, cfg);

    EXPECT_TRUE(terminal.sendPage(5, src, TICK));
    EXPECT_EQ(terminal.getPagesCreated(), 1);
    EXPECT_EQ(terminal.getPagesSent(), 1);
    EXPECT_EQ(terminal.getPacketsGenerated(), 5);
    EXPECT_EQ(terminal.getPacketsSent(), 0);
    EXPECT_EQ(terminal.getPacketsOutPending(), 5);

    EXPECT_FALSE(terminal.sendPage(3, src, TICK));
    EXPECT_EQ(terminal.getPagesCreated(), 2);
    EXPECT_EQ(terminal.getPagesSent(), 1);
    EXPECT_EQ(terminal.getPagesDropped(), 1);
    EXPECT_EQ(terminal.getPacketsGenerated(), 8);
    EXPECT_EQ(terminal.getPacketsOutPending(), 5);
    EXPECT_EQ(terminal.getPacketsOutDropped(), 3);

    EXPECT_TRUE(terminal.sendPage(2, src, TICK));
    EXPECT_EQ(terminal.getPagesCreated(), 3);
    EXPECT_EQ(terminal.getPagesSent(), 2);
    EXPECT_EQ(terminal.getPagesDropped(), 1);
    EXPECT_EQ(terminal.getPacketsGenerated(), 10);
    EXPECT_EQ(terminal.getPacketsOutPending(), 7);
    EXPECT_EQ(terminal.getPacketsOutDropped(), 3);
}

// =============== Reception tests ===============
TEST_F(TerminalTest, ReceivePacketBasic) {
    const Packet packet(10, 0, 5, src, dst, TICK);

    EXPECT_TRUE(trm.receivePacket(packet));
    EXPECT_EQ(trm.getPacketsReceived(), 1);
    EXPECT_EQ(trm.getPacketsInPending(), 1);
}

TEST_F(TerminalTest, ReceivePacketBufferFull) {
    const Terminal::Config cfg = {3, 12, 7, 8};
    Terminal terminal(&rtr, TERMINAL_ID, cfg);

    EXPECT_TRUE(terminal.receivePacket(Packet(100, 0, 5, src, dst, TICK)));
    EXPECT_TRUE(terminal.receivePacket(Packet(100, 1, 5, src, dst, TICK)));
    EXPECT_TRUE(terminal.receivePacket(Packet(100, 2, 5, src, dst, TICK)));
    EXPECT_EQ(terminal.getPacketsReceived(), 3);
    EXPECT_EQ(terminal.getPacketsInPending(), 3);

    EXPECT_FALSE(terminal.receivePacket(Packet(100, 3, 5, src, dst, TICK)));
    EXPECT_EQ(terminal.getPacketsReceived(), 4);
    EXPECT_EQ(terminal.getPacketsInPending(), 3);
}

// =============== Input buffer processing tests ===============
TEST_F(TerminalTest, ProcessInputBufferCompletePage) {
    trm.receivePacket(Packet(100, 0, 3, src, dst, TICK));
    trm.receivePacket(Packet(100, 1, 3, src, dst, TICK));
    trm.receivePacket(Packet(100, 2, 3, src, dst, TICK));

    trm.processInputBuffer(1);

    EXPECT_EQ(trm.getPagesCompleted(), 1);
    EXPECT_EQ(trm.getPacketsSuccProcessed(), 3);
    EXPECT_EQ(trm.getPacketsInPending(), 0);
}

TEST_F(TerminalTest, ProcessInputBufferIncompletePage) {
    trm.receivePacket(Packet(100, 0, 5, src, dst, TICK));
    trm.receivePacket(Packet(100, 1, 5, src, dst, TICK));

    const size_t proc = trm.processInputBuffer(1);

    EXPECT_EQ(proc, 2);
    EXPECT_EQ(trm.getPagesCompleted(), 0);
    EXPECT_EQ(trm.getPacketsInPending(), 2);
}

TEST_F(TerminalTest, ProcessInputBufferMultiplePages) {
    trm.receivePacket(Packet(100, 0, 2, src, dst, TICK));
    trm.receivePacket(Packet(100, 1, 2, src, dst, TICK));
    trm.receivePacket(Packet(200, 0, 2, src, dst, TICK));
    trm.receivePacket(Packet(200, 1, 2, src, dst, TICK));

    trm.processInputBuffer(1);

    EXPECT_EQ(trm.getPagesCompleted(), 2);
    EXPECT_EQ(trm.getPacketsSuccProcessed(), 4);
    EXPECT_EQ(trm.getPacketsInPending(), 0);
}

TEST_F(TerminalTest, ProcessInputBufferOutOfOrder) {
    trm.receivePacket(Packet(100, 2, 3, src, dst, TICK));
    trm.receivePacket(Packet(100, 0, 3, src, dst, TICK));
    trm.receivePacket(Packet(100, 1, 3, src, dst, TICK));

    trm.processInputBuffer(1);

    EXPECT_EQ(trm.getPagesCompleted(), 1);
    EXPECT_EQ(trm.getPacketsSuccProcessed(), 3);
}

TEST_F(TerminalTest, ProcessInputBufferDuplicatePacket) {
    trm.receivePacket(Packet(100, 0, 2, src, dst, TICK));
    trm.receivePacket(Packet(100, 0, 2, src, dst, TICK));
    trm.receivePacket(Packet(100, 1, 2, src, dst, TICK));
    trm.processInputBuffer(1);

    EXPECT_EQ(trm.getPagesCompleted(), 1);
    EXPECT_EQ(trm.getPacketsReceived(), 3);
    EXPECT_EQ(trm.getPacketsInPending(), 0);
    EXPECT_EQ(trm.getPacketsInDropped(), 1);

    trm.receivePacket(Packet(100, 0, 2, src, dst, TICK));
    trm.processInputBuffer(1);

    EXPECT_EQ(trm.getPagesCompleted(), 1);
}

TEST_F(TerminalTest, ProcessInputBufferWrongDestination) {
    const IPAddress wrongIP(8, 20);

    trm.receivePacket(Packet(100, 0, 3, src, wrongIP, TICK));

    const size_t processed = trm.processInputBuffer(1);

    EXPECT_EQ(processed, 1);
    EXPECT_EQ(trm.getPagesCompleted(), 0);
    EXPECT_EQ(trm.getPacketsInDropped(), 1);
}

TEST_F(TerminalTest, ProcessInputBufferBandwidthLimit) {
    const Terminal::Config cfg{0, 2, 0, Terminal::DEF_OUTPUT_BW};
    Terminal terminal(&rtr, TERMINAL_ID, cfg);

    for (int i = 0; i < 5; ++i) {
        terminal.receivePacket(Packet(100, i, 10, src, dst, TICK));
    }

    const size_t processed = terminal.processInputBuffer(1);
    EXPECT_EQ(processed, 2);
    EXPECT_EQ(terminal.getPacketsInPending(), 5);
}

TEST_F(TerminalTest, ProcessInputBufferExpiredPacket) {
    trm.receivePacket(Packet(100, 0, 3, src, dst, 5));

    trm.processInputBuffer(10);

    EXPECT_EQ(trm.getPacketsInTimedOut(), 1);
    EXPECT_EQ(trm.getPagesCompleted(), 0);
}

// =============== Output buffer processing tests ===============
TEST_F(TerminalTest, ProcessOutputBufferSendsPackets) {
    trm.sendPage(3, dst, TICK);

    const size_t sent = trm.processOutputBuffer(1);

    EXPECT_EQ(sent, 3);
    EXPECT_EQ(trm.getPacketsSent(), 3);
    EXPECT_EQ(trm.getPacketsOutPending(), 0);
}

TEST_F(TerminalTest, ProcessOutputBufferBandwidthLimit) {
    const Terminal::Config cfg{0, Terminal::DEF_INPUT_PROC, 0, 3};
    Terminal terminal(&rtr, TERMINAL_ID, cfg);

    terminal.sendPage(7, dst, TICK);

    const size_t sent = terminal.processOutputBuffer(1);
    EXPECT_EQ(sent, 3);
    EXPECT_EQ(terminal.getPacketsSent(), 3);
    EXPECT_EQ(terminal.getPacketsOutPending(), 4);
}

TEST_F(TerminalTest, ProcessOutputBufferExpiredPackets) {
    trm.sendPage(3, dst, 5);

    const size_t sent = trm.processOutputBuffer(10);

    EXPECT_EQ(sent, 0);
    EXPECT_EQ(trm.getPacketsSent(), 0);
    EXPECT_EQ(trm.getPacketsOutTimedOut(), 3);
}

// =============== Setter tests ===============
TEST_F(TerminalTest, SetExternalBW) {
    trm.setExternalBW(10);
    EXPECT_EQ(trm.getOutputBW(), 10);
}

TEST_F(TerminalTest, SetInternalProc) {
    trm.setInternalProc(20);
    EXPECT_EQ(trm.getInternalProc(), 20);
}

// =============== Utilities tests ===============
TEST_F(TerminalTest, ToString) {
    const std::string str = trm.toString();
    EXPECT_NE(str.find("Terminal"), std::string::npos);
    EXPECT_NE(str.find("5"), std::string::npos);
}

// =============== Tick / Quarantine tests ===============
TEST_F(TerminalTest, TickProcessesBothBuffers) {
    trm.sendPage(2, dst, TICK);
    trm.receivePacket(Packet(200, 0, 2, src, dst, TICK));
    trm.receivePacket(Packet(200, 1, 2, src, dst, TICK));

    trm.tick(1);

    EXPECT_EQ(trm.getPacketsSent(), 2);
    EXPECT_EQ(trm.getPagesCompleted(), 1);
}

TEST_F(TerminalTest, ReassemblerTimeout_SetsQuarantine) {
    trm.receivePacket(Packet(500, 0, 10, src, dst, TICK));
    trm.processInputBuffer(1);

    trm.tick(TICK + MAX_ASSEMBLER_TTL + 1);

    EXPECT_EQ(trm.getPagesTimedOut(), 1);
    EXPECT_EQ(trm.getPacketsInTimedOut(), 1);

    const bool accepted = trm.receivePacket(Packet(500, 1, 10, src, dst, TICK));
    EXPECT_FALSE(accepted);
    EXPECT_EQ(trm.getPacketsInTimedOut(), 2);
}

TEST_F(TerminalTest, QuarantineExpires_AcceptsPacketAgain) {
    trm.receivePacket(Packet(500, 0, 10, src, dst, TICK));
    trm.processInputBuffer(1);

    trm.tick(TICK + MAX_ASSEMBLER_TTL + 1);

    trm.tick(2 * TICK + MAX_ASSEMBLER_TTL + 1);

    const bool accepted = trm.receivePacket(Packet(500, 1, 10, src, dst, TICK));
    EXPECT_TRUE(accepted);
}

// =============== Complex scenario ===============
TEST_F(TerminalTest, ComplexScenario_SendAndReceive) {
    trm.sendPage(5, dst, TICK);
    trm.sendPage(5, dst, TICK);
    trm.sendPage(5, dst, TICK);

    EXPECT_EQ(trm.getPagesCreated(), 3);
    EXPECT_EQ(trm.getPacketsOutPending(), 15);

    for (int i = 0; i < 3; ++i)
        trm.receivePacket(Packet(100, i, 3, src, dst, TICK));
    for (int i = 0; i < 3; ++i)
        trm.receivePacket(Packet(200, i, 3, src, dst, TICK));

    trm.tick(1);

    EXPECT_EQ(trm.getPagesCompleted(), 2);
    EXPECT_EQ(trm.getPacketsSent(), 5);
}
