#include "core/Admin.h"

int main() {
    Network network{};
    const Admin admin{network};

    admin.runFor(100, 10);

    std::cout << "\n=== FINAL REPORT ===\n";
    admin.printReport();

    return 0;
}
