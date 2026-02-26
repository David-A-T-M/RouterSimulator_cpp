#include <algorithm>

#include "core/Page.h"
#include "core/Router.h"
#include "core/Terminal.h"

Terminal::Terminal(Router* router, uint8_t terminalID, const Config& cfg)
    : terminalIP(router->getIP().getRouterIP(), terminalID),
      rtrConn(router),
      inBuffer(cfg.inBufferCap),
      inProcCap(cfg.inProcCap),
      outBuffer(cfg.outBufferCap),
      outBW(cfg.outputBW),
      pagesCreated(0),
      pagesSent(0),
      pagesOutDropped(0),
      pagesCompleted(0),
      pagesTimedOut(0),
      packetsGenerated(0),
      packetsSent(0),
      packetsOutDropped(0),
      packetsOutTimedOut(0),
      packetsReceived(0),
      packetsInTimedOut(0),
      packetsInDropped(0),
      packetsSuccProcessed(0),
      nextPageID(0) {
    if (terminalID == 0) {
        throw std::invalid_argument("Terminal ID must be greater than 0");
    }
}

bool Terminal::sendPage(size_t length, IPAddress destIP, size_t expTick) {
    const Page page(nextPageID++, length, terminalIP, destIP);

    List<Packet> packets  = page.toPackets(expTick);
    const auto numPackets = packets.size();
    pagesCreated++;
    packetsGenerated += numPackets;

    if (outBuffer.availableSpace() < numPackets) {
        pagesOutDropped++;
        packetsOutDropped += numPackets;
        return false;
    }

    for (const auto& packet : packets) {
        outBuffer.enqueue(packet);
    }
    pagesSent++;

    return true;
}

bool Terminal::receivePacket(const Packet& packet) {
    packetsReceived++;

    if (isIDQuarantined(packet.getPageID())) {
        packetsInTimedOut++;
        return false;
    }

    if (!inBuffer.enqueue(packet)) {
        packetsInDropped++;
        return false;
    }

    return true;
}

size_t Terminal::processInputBuffer(size_t currentTick) {
    size_t processedCount = 0;

    while (processedCount < inProcCap && !inBuffer.isEmpty()) {
        Packet packet = inBuffer.dequeue();
        processedCount++;

        if (currentTick >= packet.getExpTick()) {
            packetsInTimedOut++;
            continue;
        }

        if (packet.getDstIP() != terminalIP) {
            packetsInDropped++;
            continue;
        }

        PageReassembler* reassembler = findOrCreateReassembler(
            packet.getPageID(), packet.getPageLen(), currentTick + MAX_ASSEMBLER_TTL);

        if (!reassembler) {
            packetsInTimedOut++;
            continue;
        }

        if (!reassembler->addPacket(packet)) {
            packetsInDropped++;
            continue;
        }

        if (reassembler->isComplete()) {
            packetsSuccProcessed += reassembler->getTotalPackets();
            handleCompletedPage(reassembler->getPageID());
        }
    }
    return processedCount;
}

size_t Terminal::processOutputBuffer(size_t currentTick) {
    size_t processedCount = 0;

    while (processedCount < outBW && !outBuffer.isEmpty()) {
        const Packet packet = outBuffer.dequeue();

        if (currentTick >= packet.getExpTick()) {
            packetsOutTimedOut++;
            continue;
        }

        rtrConn->receivePacket(packet);
        packetsSent++;
        processedCount++;
    }

    return processedCount;
}

void Terminal::tick(size_t currentTick) {
    updateQuarantine(currentTick);
    cleanupReassemblers(currentTick);

    processOutputBuffer(currentTick);
    processInputBuffer(currentTick);
}

PageReassembler* Terminal::findOrCreateReassembler(size_t pageID, size_t pageLength,
                                                   size_t expTick) {
    auto it = std::ranges::find(reassemblers, pageID, &PageReassembler::getPageID);

    if (it != reassemblers.end()) {
        return (it->getTotalPackets() == pageLength) ? &(*it) : nullptr;
    }

    return &reassemblers.emplace_back(pageID, pageLength, expTick);
}

void Terminal::handleCompletedPage(size_t pageID) {
    auto it = std::ranges::find(reassemblers, pageID, &PageReassembler::getPageID);

    if (it == reassemblers.end()) {
        return;
    }

    List<Packet> packets = it->package();

    [[maybe_unused]]
    const Page completedPage(std::move(packets));

    pagesCompleted++;
    reassemblers.erase(it);
}

void Terminal::cleanupReassemblers(size_t currentTick) {
    std::erase_if(reassemblers, [this, currentTick](const PageReassembler& ra) {
        if (ra.getExpTick() <= currentTick) {
            this->pagesTimedOut++;
            this->packetsInTimedOut += ra.getReceivedPackets();
            this->quarantine.push_back({ra.getPageID(), currentTick + PACKET_TTL});
            return true;
        }
        return false;
    });
}

void Terminal::updateQuarantine(size_t currentTick) {
    std::erase_if(quarantine,
                  [currentTick](const QuarantinedID& q) { return q.expTick <= currentTick; });
}

std::string Terminal::toString() const {
    std::ostringstream oss;
    oss << "Terminal{IP: " << terminalIP << " | Sent: " << pagesSent
        << ", | Reassembled: " << pagesCompleted << " | Pending: " << reassemblers.size() << "}";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Terminal& terminal) {
    os << terminal.toString();
    return os;
}

bool Terminal::isIDQuarantined(size_t id) const {
    return std::ranges::any_of(
        quarantine, [id](size_t qID) { return qID == id; }, &QuarantinedID::pageID);
}
