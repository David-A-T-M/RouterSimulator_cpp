#include <gtest/gtest.h>
#include "core/Terminal.h"

class MockRouter {
public:
    List<Packet> receivedPackets;

    void receivePacket(const Packet& packet) { receivedPackets.pushBack(packet); }
};

// =============== Constructor tests ===============
TEST(TerminalConstructor, ValidConstructor) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);

    const Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    EXPECT_EQ(terminal.getTerminalIP(), terminalIP);
    EXPECT_EQ(terminal.getSentPages(), 0);
    EXPECT_EQ(terminal.getReceivedPages(), 0);
    EXPECT_EQ(terminal.getExternalBW(), 4);
    EXPECT_EQ(terminal.getInternalBW(), 8);
}

TEST(TerminalConstructor, ConstructorWithCustomBW) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);

    const Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router), 100, 200, 10, 20);

    EXPECT_EQ(terminal.getExternalBW(), 10);
    EXPECT_EQ(terminal.getInternalBW(), 20);
}

TEST(TerminalConstructor, ConstructorNullRouter) {
    const IPAddress terminalIP(5, 10);

    EXPECT_THROW(Terminal(terminalIP, nullptr), std::invalid_argument);
}

// =============== Transmission tests ===============
TEST(TerminalTransmission, SendPageBasic) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress destIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    EXPECT_TRUE(terminal.sendPage(5, destIP));
    EXPECT_EQ(terminal.getSentPages(), 1);
}

TEST(TerminalTransmission, SendMultiplePages) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress destIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.sendPage(5, destIP);
    terminal.sendPage(10, destIP);
    terminal.sendPage(3, destIP);

    EXPECT_EQ(terminal.getSentPages(), 3);
}

TEST(TerminalTransmission, SendPageBufferOverflow) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress destIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router), 5, 100);

    EXPECT_FALSE(terminal.sendPage(10, destIP));
}

// =============== Reception tests ===============
TEST(TerminalReception, ReceivePacketBasic) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    const Packet packet(100, 0, 5, 0, terminalIP, originIP);

    EXPECT_TRUE(terminal.receivePacket(packet));
}

TEST(TerminalReception, ReceivePacketBufferFull) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router), 50, 3);

    EXPECT_TRUE(terminal.receivePacket(Packet(100, 0, 5, 0, terminalIP, originIP)));
    EXPECT_TRUE(terminal.receivePacket(Packet(100, 1, 5, 0, terminalIP, originIP)));
    EXPECT_TRUE(terminal.receivePacket(Packet(100, 2, 5, 0, terminalIP, originIP)));

    EXPECT_FALSE(terminal.receivePacket(Packet(100, 3, 5, 0, terminalIP, originIP)));
}

// =============== InputBuffer tests ===============
TEST(TerminalInputBuffer, ProcessInputBufferCompletePage) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 0, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 2, 3, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 1);
}

TEST(TerminalInputBuffer, ProcessInputBufferIncompletePage) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 0, 5, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 5, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 0);
}

TEST(TerminalInputBuffer, ProcessInputBufferMultiplePages) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 0, 2, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 2, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(200, 0, 2, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(200, 1, 2, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 2);
}

TEST(TerminalInputBuffer, ProcessInputBufferOutOfOrder) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 2, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 0, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 3, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 1);
}

TEST(TerminalInputBuffer, ProcessInputBufferDuplicatePacket) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 0, 2, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 2, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 1);

    terminal.receivePacket(Packet(100, 0, 2, 0, terminalIP, originIP));

    terminal.processInputBuffer();

    EXPECT_EQ(terminal.getReceivedPages(), 1);
}

TEST(TerminalInputBuffer, ProcessInputBufferWrongDestination) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress wrongIP(8, 20);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.receivePacket(Packet(100, 0, 3, 0, wrongIP, originIP));

    EXPECT_EQ(terminal.processInputBuffer(), 0);
}

TEST(TerminalInputBuffer, ProcessInputBufferBandwidthLimit) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router), 50, 100, 4, 2);

    for (int i = 0; i < 5; ++i) {
        terminal.receivePacket(Packet(100, i, 10, 0, terminalIP, originIP));
    }

    const size_t processed = terminal.processInputBuffer();
    EXPECT_EQ(processed, 2);
}

// =============== BW Configuration tests ===============
TEST(TerminalBW, SetExternalBW) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.setExternalBW(10);

    EXPECT_EQ(terminal.getExternalBW(), 10);
}

TEST(TerminalBW, SetInternalBW) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    terminal.setInternalBW(20);

    EXPECT_EQ(terminal.getInternalBW(), 20);
}

// =============== Utilities tests ===============
TEST(TerminalUtilities, ToString) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);

    const Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    std::string str = terminal.toString();

    EXPECT_NE(str.find("5.10"), std::string::npos);
    EXPECT_NE(str.find("Terminal"), std::string::npos);
}

// =============== Complex tests ===============
TEST(TerminalComplex, ComplexScenario) {
    MockRouter router;
    const IPAddress terminalIP(5, 10);
    const IPAddress originIP(10, 20);

    Terminal terminal(terminalIP, reinterpret_cast<Router*>(&router));

    for (int i = 0; i < 5; ++i) {
        terminal.sendPage(10, IPAddress(10, 20));
    }

    terminal.receivePacket(Packet(100, 0, 5, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(100, 1, 5, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(200, 0, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(200, 1, 3, 0, terminalIP, originIP));
    terminal.receivePacket(Packet(200, 2, 3, 0, terminalIP, originIP));

    for (int i = 0; i < 10; ++i) {
        terminal.tick();
    }

    EXPECT_EQ(terminal.getSentPages(), 5);
    EXPECT_EQ(terminal.getReceivedPages(), 1);
}
