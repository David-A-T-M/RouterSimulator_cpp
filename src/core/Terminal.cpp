#include "core/Terminal.h"

Terminal::Terminal(IPAddress ip, Router* router, size_t outputCapacity, size_t inputCapacity, size_t externalBW,
                   size_t internalBW)
    : ip(ip),
      connectedRouter(router),
      outputBuffer(PacketBuffer::Mode::FIFO, outputCapacity),
      inputBuffer(PacketBuffer::Mode::FIFO, inputCapacity),
      externalBW(externalBW),
      internalBW(internalBW),
      sentPages(0),
      receivedPages(0),
      nextPageID(0) {
    if (!router) {
        throw std::invalid_argument("Router cannot be nullptr");
    }
}

bool Terminal::sendPage(size_t length, IPAddress destIP) {
    const Page page(nextPageID++, length, ip, destIP);

    List<Packet> packets = page.fragmentToPackets();

    bool allEnqueued = true;

    for (const auto& packet : packets) {
        if (!outputBuffer.enqueue(packet)) {
            allEnqueued = false;
        }
    }

    sentPages++;

    return allEnqueued;
}

bool Terminal::receivePacket(const Packet& packet) {
    if (inputBuffer.enqueue(packet)) {
        return true;
    }
    return false;
}

void Terminal::tick() {
    processOutputBuffer();
    processInputBuffer();
}

size_t Terminal::processOutputBuffer() {
    size_t packetsSent = 0;

    while (packetsSent < externalBW && !outputBuffer.isEmpty()) {
        Packet packet = outputBuffer.dequeue();

        // TODO: implement router
        // connectedRouter->receivePacket(packet, this);

        packetsSent++;
    }

    return packetsSent;
}

size_t Terminal::processInputBuffer() {
    size_t packetsProcessed = 0;

    while (packetsProcessed < internalBW && !inputBuffer.isEmpty()) {
        Packet packet = inputBuffer.dequeue();

        if (packet.getDestinationIP() != ip) {
            continue;
        }

        PageReassembler* reassembler = findOrCreateReassembler(packet.getPageID(), packet.getPageLength());

        if (!reassembler) {
            continue;
        }

        if (!reassembler->addPacket(packet)) {
            packetsProcessed++;
            continue;
        }

        if (reassembler->isComplete()) {
            handleCompletedPage(reassembler);
        }

        packetsProcessed++;
    }

    return packetsProcessed;
}

PageReassembler* Terminal::findOrCreateReassembler(int pageID, int pageLength) {
    for (auto& reassembler : reassemblers) {
        if (reassembler.pageID == pageID) {
            if (reassembler.expectedPackets != pageLength) {
                return nullptr;
            }
            return &reassembler;
        }
    }

    try {
        reassemblers.pushBack(PageReassembler(pageID, pageLength));
        return &reassemblers.getTail();
    } catch (...) {
        return nullptr;
    }
}

void Terminal::handleCompletedPage(PageReassembler* reassembler) {
    List<Packet> packets = reassembler->package();

    const Page completedPage(std::move(packets));

    std::cout << "Terminal " << ip << " received page "
              << completedPage.getPageID() << std::endl;

    receivedPages++;

    removeReassembler(reassembler);
}

void Terminal::removeReassembler(const PageReassembler* reassembler) {
    for (int i = 0; i < reassemblers.size(); ++i) {
        if (reassemblers[i] == *reassembler) {
            reassemblers.removeAt(i);
            return;
        }
    }
}

std::string Terminal::toString() const {
    std::ostringstream oss;
    oss << "Terminal{IP=" << ip
        << ", Sent=" << sentPages
        << ", Received=" << receivedPages
        << ", Active=" << reassemblers.size() << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Terminal& terminal) {
    os << terminal.toString();
    return os;
}
