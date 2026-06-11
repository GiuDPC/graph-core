# graph-core

Interactive graph visualization and network simulation tool built with C++17, Dear ImGui, GLFW, and OpenGL 3.3.

## Features

- Graph editing with interactive canvas (pan, zoom, drag nodes)
- Graph algorithms: Dijkstra, Kruskal MST, BFS, DFS, cycle detection, graph coloring, isomorphism verification
- Network simulation with real-time packet visualization, traffic flows, congestion, jitter, and automatic events
- Step-by-step algorithm animation
- Isomorphism testing with dual-graph editing
- Audio feedback for algorithm steps and network events
- Hardware deployment panel for network planning
- Topology templates (enterprise, mesh, star, simplified internet)
- Project save/load in JSON format

## Requirements

- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.20+
- OpenGL 3.3 capable GPU
- GLFW, OpenGL, X11/Wayland (Linux) or Win32 (Windows) development libraries

On Arch Linux: `sudo pacman -S cmake glfw-x11 wayland`

On Debian/Ubuntu: `sudo apt install cmake libglfw3-dev libxkbcommon-dev`

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Run

```bash
./build/graph-core
```

## Run tests

```bash
./build/test_grafo && ./build/test_dijkstra && ./build/test_unionfind
```

## Cross-compile for Windows

Requires MinGW-w64. Use the provided toolchain file:

```bash
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j$(nproc)
```

## Project structure

```
src/
  main.cpp                  # Entry point
  audio/                    # Sound system (miniaudio, synthetic sounds)
  interfaz/                 # UI layer
    Interfaz.h              # Main orchestrator
    componentes/            # Status bar, menus, dialogs, log panel
    lienzo/                 # Canvas rendering and input handling
    paneles/                # Graph editor, network simulation, hardware, isomorphism
    util/                   # Easing, animation, colors
  nucleo/                   # Core domain
    Grafo.h                 # Graph data structure
    SimuladorRed.h          # Network simulation engine
    Topologias.h            # Topology generators
    UnionFind.h             # Union-Find data structure
    algoritmos/             # Graph algorithms
    tipos/                  # Node, edge, hardware types
  persistencia/             # JSON serialization
tests/                      # Unit tests (GoogleTest)
```

## License

MIT
