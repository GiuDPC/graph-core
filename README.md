# graph-core

Visualizador y simulador de grafos interactivo creado con C++17, Dear ImGui, GLFW y OpenGL 3.3.

## Caracteristicas

- Editor de grafos con lienzo interactivo (pan, zoom, arrastrar nodos)
- Algoritmos de grafos: Dijkstra, Kruskal MST, BFS, DFS, deteccion de ciclos, coloreo, verificacion de isomorfismo
- Simulacion de red con visualizacion de paquetes en tiempo real, flujos de trafico, congestion, jitter y eventos automaticos
- Animacion paso a paso de algoritmos
- Pruebas de isomorfismo con editor de doble grafo
- Sonido con feedback para pasos de algoritmos y eventos de red
- Panel de despliegue de hardware para planificacion de red
- Plantillas de topologia (empresarial, malla, estrella, internet simplificada)
- Guardado y carga de proyectos en formato JSON

## Requisitos

- Compilador con soporte C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.20+
- GPU compatible con OpenGL 3.3
- Librerias de desarrollo de GLFW, OpenGL, X11/Wayland (Linux) o Win32 (Windows)

En Arch Linux: `sudo pacman -S cmake glfw-x11 wayland`

En Debian/Ubuntu: `sudo apt install cmake libglfw3-dev libxkbcommon-dev`

## Compilar

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Ejecutar

```bash
./build/graph-core
```

## Ejecutar tests

```bash
./build/test_grafo && ./build/test_dijkstra && ./build/test_unionfind
```

## Cross-compile para Windows

Requiere MinGW-w64. Usar el toolchain incluido:

```bash
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j$(nproc)
```

## Estructura del proyecto

```
src/
  main.cpp                  # Punto de entrada
  audio/                    # Sistema de sonido (miniaudio, sonidos sinteticos)
  interfaz/                 # Capa de UI
    Interfaz.h              # Orquestador principal
    componentes/            # Barra de estado, menus, dialogos, panel de logs
    lienzo/                 # Renderizado del canvas y manejo de entrada
    paneles/                # Editor de grafos, simulacion de red, hardware, isomorfismo
    util/                   # Easing, animacion, colores
  nucleo/                   # Dominio principal
    Grafo.h                 # Estructura de datos del grafo
    SimuladorRed.h          # Motor de simulacion de red
    Topologias.h            # Generadores de topologia
    UnionFind.h             # Estructura Union-Find
    algoritmos/             # Algoritmos de grafos
    tipos/                  # Tipos de nodo, arista, hardware
  persistencia/             # Serializacion JSON
tests/                      # Tests unitarios (GoogleTest)
```

## Licencia

MIT
