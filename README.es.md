# graphCore v3.1

**Motor avanzado de visualización y análisis de grafos y redes.**

graphCore es una aplicación interactiva para crear, editar, visualizar y analizar grafos y redes. Construido con C++17, ImGui, GLFW y OpenGL.

## Características

- **Motor de Grafos** — grafos dirigidos/no-dirigidos con pesos, 15+ algoritmos (Dijkstra, Kruskal, BFS, DFS, Detección de Ciclos, Coloreo, Isomorfismo, Planaridad, Euler/Hamilton, y más)
- **Simulación de Red** — tráfico en tiempo real (HTTP, DNS, VOIP, DDOS) con latencia, jitter, pérdida de paquetes, failover, traceroute y tormentas de red
- **ForceAtlas2** — simulación física Barnes-Hut para grafos grandes (acelerado con QuadTree)
- **Plantillas de Grafos** — Generadores instantáneos para estructuras clásicas (Mallas, Árboles, Watts-Strogatz, Petersen, etc.)
- **Visualización** — lienzo interactivo con zoom, arrastre, visualización de algoritmos en tiempo real con caminos animados
- **Audio** — efectos de sonido generados proceduralmente (sin samples externos)
- **Persistencia** — guardar/cargar en JSON y GEXF

## Requisitos

- **CMake** ≥ 3.20
- **Compilador C++17** (GCC, Clang, MSVC)
- **GLFW3**, **OpenGL** (gráficos)
- **ALSA** o **PulseAudio** (audio, opcional)

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

## Inicio Rápido

```bash
cd graphCore

# Compilar + tests
make test

# Ejecutar
make run
```

### Compilación manual
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/graph-core
```

### Scripts
```bash
./scripts/build.sh      # compilación dev con tests
./scripts/test.sh        # compilar + ejecutar tests
./scripts/package.sh     # release + empaquetado
./scripts/clean.sh       # limpiar builds
```

## Estructura del Proyecto

```
src/
├── nucleo/              # Núcleo — grafo, algoritmos, simulación
│   ├── Grafo.hpp/.cpp
│   ├── SimuladorRed.hpp/.cpp
│   ├── algoritmos/      # 15 algoritmos de grafos
│   └── datos/           # estructuras de datos
├── interfaz/            # UI (ImGui)
│   ├── Interfaz.hpp/.cpp
│   ├── paneles/         # paneles de control
│   ├── lienzo/          # renderizado del lienzo
│   ├── componentes/     # componentes de UI
│   └── estado/          # estado de la UI
├── audio/               # audio procedural
└── persistencia/        # guardar/cargar JSON, GEXF
tests/                   # 55 tests (GTest)
muestras/                # grafos de ejemplo (.json)
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

55 tests cubriendo: CRUD de grafos, Dijkstra (dirigido/no-dirigido), BFS, detección de ciclos, Union-Find, y simulación de red.

## Grafos de Ejemplo

| Archivo | Nodos | Aristas | Descripción |
|---------|-------|---------|-------------|
| `Grafo_Dodecaedro.json` | 20 | 30 | Grafo de dodecaedro — estructura de sólido platónico |
| `Grafo_Desargues.json` | 20 | 50 | Grafo de Desargues — grafo simétrico transitivo |
| `Muestra_Arbol_BFS.json` | 40 | 39 | Árbol BFS — estructura jerárquica |
| `Muestra_Dijkstra_Caminos.json` | 35 | 58 | Grafo reticular — demostración de Dijkstra |
| `Muestra_Estrella_Doble.json` | 12 | 11 | Estrella doble — topología de hub central |
| `Muestra_Anillo_Enlazado.json` | 20 | 40 | Anillo enlazado — red circular con atajos largos |
| `Muestra_Fractal_Sierpinski.json` | 9 | 9 | Triángulo fractal de Sierpinski (nivel 2) |

## Historial de Versiones

- **v3.2** — Nuevas plantillas de grafos (Mallas, Árboles, Watts-Strogatz), limpieza general de la interfaz (eliminadas anotaciones y barra de zoom).
- **v3.1** — Refactor masivo del código: todos los headers divididos en .hpp + .cpp, build modular, 55 tests, CI pipeline, correcciones de Dijkstra, fix de reset en ForceAtlas2
- **v3.0** — Simulación de red, motor de audio, animación de algoritmos, layout ForceAtlas2
- **v2.0** — Migración a ImGui, edición de grafos, visualización
- **v1.0** — Versión inicial en consola con algoritmos de grafos

## Licencia

MIT
