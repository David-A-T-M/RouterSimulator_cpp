#pragma once

#include "Network.h"

/**
 * @class Admin
 * @brief Provides administrative functions for monitoring and controlling the network simulation.
 *
 * The Admin class serves as an interface for generating reports on the network's performance and
 * running the simulation for a specified number of ticks with periodic reporting. It holds a
 * reference to the Network object and uses its methods to retrieve statistics and advance the
 * simulation.
 */
class Admin {
    /** Pointer to the Network object that this Admin instance will manage and report on. */
    Network* network;

public:
    /**
     * @brief Constructor that initializes the Admin with a reference to the Network object.
     *
     * @param net Reference to the Network object that this Admin will manage.
     */
    explicit Admin(Network& net) : network(&net) {}

    /**
     * @brief Destructor for Admin. Defaulted to allow automatic cleanup of resources.
     */
    ~Admin() = default;

    /**
     * @brief Deleted copy constructor to prevent copying of the Admin object.
     */
    Admin(const Admin&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the Admin object.
     *
     * @return Reference to this Admin object.
     */
    Admin& operator=(const Admin&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the Admin object.
     */
    void printReport() const;

    /**
     * @brief Deleted move assignment operator to prevent moving of the Admin object.
     *
     * @param ticks The number of ticks to run the simulation for.
     * @param reportInterval The interval (in ticks) at which to print the network report.
     */
    void runFor(size_t ticks, size_t reportInterval = 10) const;
};
