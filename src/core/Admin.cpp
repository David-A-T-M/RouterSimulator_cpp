#include "core/Admin.h"

void Admin::printReport() const {
    const NetworkStats s = network->getStats();

    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║         NETWORK REPORT               ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "║ Tick:             " << std::setw(6) << s.currentTick << "             ║\n";
    std::cout << "║ Routers:          " << std::setw(6) << s.totalRouters << "             ║\n";
    std::cout << "║ Terminals:        " << std::setw(6) << s.totalTerminals << "             ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║ PACKETS                              ║\n";
    std::cout << "║   Generated:      " << std::setw(6) << s.packetsGenerated << "             ║\n";
    std::cout << "║   Sent:           " << std::setw(6) << s.packetsSent << "             ║\n";
    std::cout << "║   Delivered:      " << std::setw(6) << s.packetsDelivered << "             ║\n";
    std::cout << "║   Dropped:        " << std::setw(6) << s.packetsDropped << "             ║\n";
    std::cout << "║   Timed out:      " << std::setw(6) << s.packetsTimedOut << "             ║\n";
    std::cout << "║   In flight:      " << std::setw(6) << s.packetsInFlight << "             ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║ PAGES                                ║\n";
    std::cout << "║   Created:        " << std::setw(6) << s.pagesCreated << "             ║\n";
    std::cout << "║   Completed:      " << std::setw(6) << s.pagesCompleted << "             ║\n";
    std::cout << "║   Dropped:        " << std::setw(6) << s.pagesDropped << "             ║\n";
    std::cout << "║   Timed out:      " << std::setw(6) << s.pagesTimedOut << "             ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║ RATES                                ║\n";
    std::cout << "║   Delivery rate:  " << std::setw(5) << s.deliveryRate() * 100
              << "%             ║\n";
    std::cout << "║   Success rate:   " << std::setw(5) << s.successRate() * 100
              << "%             ║\n";
    std::cout << "║   Drop rate:      " << std::setw(5) << s.dropRate() * 100
              << "%             ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
}

void Admin::runFor(size_t ticks, size_t reportInterval) const {
    for (size_t i = 1; i <= ticks; ++i) {
        network->simulate(1);

        if (reportInterval > 0 && i % reportInterval == 0) {
            std::cout << "── Tick " << std::setw(4) << i << " ──────────────────────────\n";
            printReport();
        }
    }
}
