# Router Topology Simulator (C++) 🌐

RouterSimulator_cpp is a C++20 discrete-event network simulator that models packet-switched networking. It simulates a
multi-router topology in which terminals generate traffic, routers forward packets hop-by-hop, and destination terminals
reassemble the original data.

The simulation advances in discrete time units called ticks. On each tick, every router and every terminal executes a
fixed pipeline of operations: draining output buffers, delivering locally-destined packets, ticking attached terminals,
and processing newly arrived packets from the input buffer. Routing tables are recomputed periodically using Dijkstra's
algorithm, with neighbor buffer occupancy used as edge cost.
Includes automated testing, static analysis, and documentation generation.

## What It Models

| Concept                       | Model                                                                     |
|:------------------------------|:--------------------------------------------------------------------------|
| Data unit sent by a terminal  | `Page` — a logical message                                                |
| Transmission unit on the wire | `Packet` — a fragment of a `Page`                                         |
| Addressing                    | 16-bit `IPAddress` (upper 8 bits = router ID, lower 8 bits = terminal ID) |
| Node types                    | `Router` (forwarding) and `Terminal` (end-host)                           |
| Buffering                     | `PacketBuffer` — FIFO queue, optionally capacity-bounded                  |
| Path selection                | `DijkstraAlgorithm` → `RoutingTable`                                      |
| Reassembly at destination     | `PageReassembler` — collects fragments, detects completion                |
| Topology                      | `Network` — randomly generated, configurable                              |
| Simulation control            | `Admin` — runs ticks, collects `NetworkStats`                             |

## 🚀 Key Features

- **CMake-based build system** with library/executable separation and GoogleTest integration
- **Automated CI pipeline** (GitHub Actions) with style checks, static analysis, builds, tests, and documentation
  validation
- **Static analysis** via `cppcheck` with exhaustive checks and HTML report generation
- **Code style enforcement** using `clang-format` with a Google-based profile
- **Doxygen documentation** with strict warning policies

## 🛠️ Technical Overview

The project follows a solid Object-Oriented Programming (OOP) approach to ensure data integrity:

* **`Router` Class:** Represents an individual network node. It manages its own identification and a local routing table
  containing its neighbors. Each router maintains separate buffers for input, output, and local delivery with configurable
  capacities and bandwidth constraints.

* **`Admin` Logic:** The core orchestrator that handles the global graph structure. It ensures that when a router is
  deleted, all incoming and outgoing links from other routers are also wiped to prevent data inconsistency. Admin also
  controls simulation timing and generates periodic reports.

* **`Network` Class:** Central coordinator managing the entire topology, traffic generation, and packet flow simulation.
  It maintains network statistics, recalculates routing tables periodically using Dijkstra's algorithm, and orchestrates
  the tick-by-tick simulation.

* **`Terminal` Class:** End-host devices that generate traffic, fragment pages into packets, and reassemble received
  packets. Terminals are the source and destination of all network traffic.

* **`Dijkstra Algorithm`:** Implements shortest path computation with a dynamic cost metric based on buffer occupancy,
  enabling load-aware routing that naturally avoids congested routers.
  
## Component Overview

The simulator is organized around five primary classes with supporting types and utilities.

| Component           | Header                           | Role                                                                                             |
|:--------------------|:---------------------------------|:-------------------------------------------------------------------------------------------------|
| `Network`           | `include/core/Network.h`         | Top-level orchestrator; owns all routers, runs the simulation loop, triggers route recalculation |
| `Router`            | `include/core/Router.h`          | Packet forwarding node; manages terminals, neighbor connections, and per-direction buffers       |
| `Terminal`          | `include/core/Terminal.h`        | End device; generates traffic, queues outgoing packets, reassembles incoming pages               |
| `PacketBuffer`      | `include/core/PacketBuffer.h`    | FIFO queue used by both `Router` and `Terminal` for all packet buffering                         |
| `PageReassembler`   | `include/core/PageReassembler.h` | Collects packet fragments at a `Terminal` and detects page completion                            |
| `DijkstraAlgorithm` | `include/algorithms/Dijkstra.h`  | Computes shortest-path routing tables for all routers using neighbor buffer usage as edge cost   |
| `Admin`             | `include/core/Admin.h`           | Monitoring interface; calls `Network::simulate()` and prints `NetworkStats` reports              |

Supporting value types — `IPAddress`, `Packet`, `Page`, and `RoutingTable`

## Sequence Diagram
  <img width="1291" height="636" alt="Screenshot From 2026-02-28 09-38-00" src="https://github.com/user-attachments/assets/e96146f9-dba2-4710-a4b0-4b10de838d4f" />

## 📐 Architecture Overview

### Layered Architecture

```
   ┌─────────────────────────────────────────────────────────┐
   │                    APPLICATION LAYER                    │
   │  ┌───────────────────────────────────────────────────┐  │
   │  │                      main.cpp                     │  │
   │  │             Admin (Simulation Control)            │  │
   │  └───────────────────────────────────────────────────┘  │
   └────────────────────────────┬────────────────────────────┘
                                │
        ┌───────────────────────▼───────────────────────┐
        │              ORCHESTRATION LAYER              │
        │  ┌─────────────────────────────────────────┐  │
        │  │        Network (Topology Manager)       │  │
        │  │        • Initializes topology           │  │
        │  │        • Coordinates tick execution     │  │
        │  │        • Collects statistics            │  │
        │  └─────────────────────────────────────────┘  │
        └───────────────────────┬───────────────────────┘
                                │
           ┌────────────────────┼────────────────────┐
           │                    │                    │
    ┌──────▼───────┐  ┌─────────▼─────────┐  ┌───────▼───────┐
    │   ROUTERS    │  │     TERMINALS     │  │  ALGORITHMS   │
    │              │  │                   │  │               │
    │  ┌────────┐  │  │  ┌─────────────┐  │  │ ┌───────────┐ │
    │  │ Input  │  │  │  │ Page Buffer │  │  │ │ Dijkstra  │ │
    │  │ Buffer │  │  │  │             │  │  │ │           │ │
    │  ├────────┤  │  │  ├─────────────┤  │  │ └───────────┘ │
    │  │Routing │  │  │  │  Generates  │  │  │               │
    │  │ Table  │  │  │  │  Traffic    │  │  │ ┌───────────┐ │
    │  ├────────┤  │  │  └─────────────┘  │  │ │Reassembler│ │
    │  │ Output │  │  │                   │  │ │           │ │
    │  │ Buffer │  │  │  ┌─────────────┐  │  │ └───────────┘ │
    │  ├────────┤  │  │  │Reassembler  │  │  │ ┌───────────┐ │
    │  │ Local  │  │  │  │ (per page)  │  │  │ │ Routing   │ │
    │  │ Buffer │  │  │  └─────────────┘  │  │ │ Tables    │ │
    │  └────────┘  │  │                   │  │ └───────────┘ │
    └──────────────┘  └───────────────────┘  └───────────────┘

                            CORE LAYER

   ┌─────────────────────────────────────────────────────────────┐
   │                    DATA STRUCTURE LAYER                     │
   │                                                             │
   │  ┌────────────┐  ┌──────────────┐  ┌─────────────────────┐  │
   │  │ List<T>    │  │ Packet       │  │ IPAddress           │  │
   │  │ (Generic   │  │              │  │ (16-bit address)    │  │
   │  │  linked    │  │ Transmission │  │ Router ID | Term ID │  │
   │  │  list)     │  │ unit         │  │                     │  │
   │  └────────────┘  └──────────────┘  └─────────────────────┘  │
   │                                                             │
   │  ┌───────────────┐  ┌──────────────┐                        │
   │  │ Page          │  │ RoutingTable │                        │
   │  │ Logical       │  │ Destination→ │                        │
   │  │ message       │  │ NextHop Map  │                        │
   │  └───────────────┘  └──────────────┘                        │
   │                                                             │
   └─────────────────────────────────────────────────────────────┘
```

## 🚧 Planned Enhancements

* **Dynamic Topology Management:** Add or remove routers and establish connections (links) in real-time.
* **File Persistence:** Load pre-configured network topologies and export simulation results.
* **Network Analysis:** Analyze connectivity, path costs, and bottlenecks.
* **Advanced Routing:** Multiple routing strategies and multicast support.
* **Performance Optimizations:** Improved algorithms and parallel processing.

## 📋 Prerequisites

- CMake 3.10+
- C++20 compiler
- GoogleTest (fetched automatically by CMake)
- Doxygen (optional, for documentation target)
- clang-format, cppcheck (optional, for local checks)

## Quick Start

### Build

```bash
cmake -B build
cmake --build build
```

### Run

```bash
./build/RouterSimulator_cpp
```

### Run Tests

```bash
ctest --test-dir build
```

### Static Analysis

```bash
cmake --build build --target check
```

### Documentation

```bash
cmake --build build --target doc
```

## Project Structure

```
RouterSimulator_cpp/
├── include/
│   ├── core/
│   │   ├── IPAddress.h          # IPAddress type
│   │   ├── Packet.h             # Packet type
│   │   ├── Page.h               # Page type
│   │   ├── PacketBuffer.h       # FIFO packet queue
│   │   ├── RoutingTable.h       # Next-hop table
│   │   ├── Router.h             # Router node
│   │   ├── Terminal.h           # End-host node
│   │   ├── Network.h            # Topology + simulation loop
│   │   └── Admin.h              # Simulation runner + reporter
│   ├── algorithms/
│   │   └── Dijkstra.h           # Routing table computation
│   └── structures/
│       └── list.h               # Generic singly-linked list
├── src/
│   └── core/                    # Implementations of the above
├── tests/
│   └── test_*.cpp               # Google Test suites per component
├── main.cpp                     # Entry point
└── CMakeLists.txt               # Build definition
```

## Build Targets

| Target                | Description                                          |
|-----------------------|------------------------------------------------------|
| `RouterLib`           | Static library from `src/*.cpp`                      |
| `RouterSimulator_cpp` | Main executable linked with `RouterLib`              |
| `test_*`              | Test executables (one per `tests/test_*.cpp`)        |
| `check`               | Run `cppcheck` static analysis                       |
| `doc`                 | Generate Doxygen HTML docs (if Doxygen is available) |

## CI Pipeline

Three parallel jobs run on pushes/PRs to `main`, `dev`, and `feat/**` branches:

1. **style-and-static-analysis**: `clang-format` check and `cppcheck` with HTML report upload
2. **build-and-test**: CMake configure, build, and CTest execution
3. **documentation**: Doxygen generation with warning enforcement and artifact upload on failure

## Code Style

- Based on Google style with customizations
- Column limit: 100
- Indent with four spaces, no tabs
- Pointer alignment: left (`int* p`)
- Access modifier offset: -4

## Notes

- GoogleTest is fetched automatically via `FetchContent` during CMake configuration
- The `check` target runs `cppcheck` with exhaustive settings and will fail the build on findings
- Doxygen warnings are treated strictly; undocumented entities and missing parameter docs cause failures
- CI installs tools via `apt-get` and uses reusable actions for style, build, and documentation steps

Wiki pages you might want to explore:

- [Build System and CI (David-A-T-M/RouterSimulator_cpp)](https://deepwiki.com/David-A-T-M/RouterSimulator_cpp)
