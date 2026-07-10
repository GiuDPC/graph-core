# graphCore v3.1

**Advanced graph visualization and network simulation engine.**

graphCore is an interactive application for creating, editing, visualizing, and analyzing graphs and networks. Built with C++17, ImGui, GLFW, and OpenGL.

## Features

- **Graph Engine** — directed/undirected weighted graphs, 15+ algorithms (Dijkstra, Kruskal, BFS, DFS, Cycle Detection, Coloring, Isomorphism, Planarity, Euler/Hamilton, and more)
- **Network Simulation** — real-time traffic simulation (HTTP, DNS, VOIP, DDOS) with latency, jitter, packet loss, failover, traceroute, and network storms
- **ForceAtlas2 Layout** — Barnes-Hut physics simulation for large graphs (QuadTree-accelerated)
- **Graph Templates** — Instant generators for classic structures (Grids, Binary Trees, Watts-Strogatz, Petersen, etc.)
- **Visualization** — interactive canvas with zoom, pan, drag, real-time algorithm visualization with animated paths
- **Audio** — procedurally generated sound effects (no external samples)
- **Persistence** — save/load in JSON and GEXF formats

## Requirements

- **CMake** ≥ 3.20
- **C++17** compiler (GCC, Clang, MSVC)
- **GLFW3**, **OpenGL** (graphics)
- **ALSA** or **PulseAudio** (audio, optional)

### Linux (Debian/Ubuntu)
```bash
sudo apt-get install cmake g++ libgl1-mesa-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libxi-dev libxext-dev \
  libglfw3-dev libasound2-dev libpulse-dev
```

### Linux (Arch)
```bash
sudo pacman -S cmake gcc glfw-x11 mesa
```

## Quick Start

```bash
git clone <repo-url> graphCore
cd graphCore

# Build + test
make test

# Run
make run
```

### Manual build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/graph-core
```

### Scripts
```bash
./scripts/build.sh      # dev build with tests
./scripts/test.sh        # build + run tests
./scripts/package.sh     # release + packaging
./scripts/clean.sh       # remove build artifacts
```

## Project Structure

```
src/
├── nucleo/              # Core — graph, algorithms, simulation
│   ├── Grafo.hpp/.cpp
│   ├── SimuladorRed.hpp/.cpp
│   ├── algoritmos/      # 15 graph algorithms
│   └── datos/           # data structures
├── interfaz/            # UI (ImGui)
│   ├── Interfaz.hpp/.cpp
│   ├── paneles/         # control panels
│   ├── lienzo/          # canvas rendering
│   ├── componentes/     # UI components
│   └── estado/          # UI state
├── audio/               # procedural audio
└── persistencia/        # save/load JSON, GEXF
tests/                   # 55 tests (GTest)
muestras/                # sample graphs (.json)
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

55 tests covering: graph CRUD, Dijkstra (with directed/undirected), BFS, cycle detection, Union-Find, and network simulation.

## Sample Graphs

| File | Nodes | Edges | Description |
|------|-------|-------|-------------|
| `Grafo_Dodecaedro.json` | 20 | 30 | Dodecahedron graph — platonic solid structure |
| `Grafo_Desargues.json` | 20 | 50 | Desargues graph — symmetric transitive graph |
| `Muestra_Arbol_BFS.json` | 40 | 39 | BFS tree — hierarchical structure |
| `Muestra_Dijkstra_Caminos.json` | 35 | 58 | Grid graph — Dijkstra pathfinding demo |
| `Muestra_Estrella_Doble.json` | 12 | 11 | Double star graph — central hub topology |
| `Muestra_Anillo_Enlazado.json` | 20 | 40 | Linked ring — circular graph with long-range edges |
| `Muestra_Fractal_Sierpinski.json` | 9 | 9 | Sierpinski fractal triangle (depth 2) |

## Version History

- **v3.2** — Added Graph Templates (Grids, Trees, Watts-Strogatz), cleaned up UI, removed annotations and zoom bar.
- **v3.1** — Massive codebase refactor: all headers split into .hpp + .cpp, modular build, 55 tests, CI pipeline, Dijkstra bugfixes, ForceAtlas2 reset fix
- **v3.0** — Network simulation, audio engine, algorithm animation, ForceAtlas2 layout
- **v2.0** — ImGui migration, graph editing, visualization
- **v1.0** — Initial console-based graph algorithms

## License

MIT
