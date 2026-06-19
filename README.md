# GraphCore: Simulador y Visualizador de Grafos y AeroGrafos

Version 2.1.0. Aplicacion de escritorio para la visualizacion interactiva de algoritmos sobre
grafos matematicos y el analisis de una base de datos mundial de vuelos (AeroGrafos). Construida
con C++17, Dear ImGui, GLFW y OpenGL 3.3.

---

## Caracteristicas

### Algoritmos de Grafos

- Dijkstra: Ruta optima entre dos nodos con animacion paso a paso y sonido sincronizado.
- Kruskal: Arbol de expansion minima (MST) con visualizacion de Union-Find.
- BFS y DFS: Busqueda en anchura y profundidad con deteccion de ciclos.
- Coloreo de grafos: Algoritmo greedy secuencial y Welsh-Powell.
- Coloreo fractal: Modulacion de colores con un fractal de Mandelbrot animado.
- Isomorfismo: Verifica si dos grafos comparten la misma estructura topologica.
- Euler y Hamilton: Busqueda de circuitos eulerianos y caminos hamiltonianos mediante backtracking.
- Generadores Fractales: Topologias complejas (Triangulo de Sierpinski, Mandala, Arbol Fractal).
- Detector Automatico: Identificacion en tiempo real (Conexo, Bipartito, Arbol, Regular, Euleriano).

### Modo AeroGrafos (Simulacion Geopolitica y de Rutas Aereas)

- Mapa Global Equirectangular: Despliegue interactivo del globo terráqueo con 63 hubs internacionales.
- Distancias Reales (Haversine): Las aristas calculan el kilometraje exacto contemplando la curvatura terrestre.
- Rutas Ortodromicas (Great Circle): Trazado realista de vuelos con curvatura progresiva.
- Restriccion Geopolitica: Simulador de cierre del espacio aereo de Rusia (ej. Moscu), afectando de inmediato las conexiones entre occidente y oriente.
- Dashboard Analitico: Tablero de informacion con la densidad de red y top de hubs aereos.
- Ejecucion Academica: Comparativa de costo logistico entre Dijkstra y busquedas base como BFS en vuelos intercontinentales.

### Sonido y UX

- Motor de audio espacial (miniaudio) con generacion sintetica para eventos (ping suave, acorde de victoria).
- Dos modos principales: Grafos (libre y puramente matematico) y AeroGrafos (mapa del mundo con datos fijos).
- Paneles acoplables, layout oscuro profesional ("engineering").

---

## Requisitos

- Compilador C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.20+
- GPU compatible con OpenGL 3.3

Dependencias automáticas via FetchContent: GLFW 3.4, Dear ImGui, nlohmann/json 3.11.3, miniaudio, GoogleTest v1.14.0.

### Linux (Debian/Ubuntu)
```bash
sudo apt install cmake libglfw3-dev libxkbcommon-dev
```

### Windows (MinGW / MSYS2)
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
```

---

## Compilacion

### Linux
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Windows (MSYS2 / MinGW)
```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Ejecucion

- Linux: `./build/graph-core`
- Windows: `.\build\graph-core.exe`

## Tests

```bash
cd build && ctest
```

---

## Estructura del proyecto

src/
  main.cpp
  audio/
    Sonidos.h
    miniaudio.h
    miniaudio_impl.cpp
  interfaz/
    Interfaz.h
    componentes/
      Toolbar.h
    estado/
      EstadoUI.h
      EstadoAeroGrafos.h
    lienzo/
      LienzoAeroGrafos.h
    paneles/
      PanelGrafos.h
      PanelAeroGrafos.h
      Matrices.h
    ventanas/
      VentanaAyuda.h
  nucleo/
    Grafo.h
    Topologias.h
    UnionFind.h
    algoritmos/
      Dijkstra.h
      Kruskal.h
      BFS.h
      DFS.h
      Euler.h
      Hamilton.h
      Coloreo.h
      Isomorfismo.h
  persistencia/
    SerializadorJSON.h
tests/
  test_grafo.cpp
  test_dijkstra.cpp
  test_unionfind.cpp

---

## Licencia

MIT. Ver el archivo LICENSE para mas informacion.
