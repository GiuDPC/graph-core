# GraphCore: Simulador y Visualizador de Grafos y Redes

Version 2.0.0. Aplicacion de escritorio para la visualizacion interactiva de algoritmos sobre
grafos y la simulacion de trafico de red a nivel de paquetes. Construida con C++17, Dear ImGui,
GLFW y OpenGL 3.3.

---

## Caracteristicas

### Algoritmos de grafos

- **Dijkstra** -- Ruta optima entre dos nodos con animacion paso a paso, particula viajera
  sobre la arista en exploracion y sonido sincronizado. Soporta pesos personalizados y latencia
  de hardware. Ejecucion instantanea o animada con tabla de distancias en vivo.
- **Kruskal** -- Arbol de expansion minima (MST) con visualizacion de Union-Find. Muestra el
  ahorro de peso respecto a la red completa. Animacion con particulas y sonido.
- **BFS (Busqueda en Anchura)** -- Recorrido por niveles desde un nodo origen. Muestra la
  distancia en saltos a cada nodo alcanzable.
- **DFS (Busqueda en Profundidad)** -- Recorrido en profundidad con deteccion de back-edges
  e identificacion de ciclos.
- **Deteccion de ciclos** -- Usa Union-Find para determinar si el grafo contiene ciclos y
  muestra las aristas que los forman.
- **Coloreo de grafos** -- Algoritmo greedy secuencial y Welsh-Powell (orden por grado
  descendente). Tabla comparativa de resultados. Numero cromatico estimado.
- **Coloreo fractal** -- Modula los colores del greedy con un fractal de Mandelbrot adaptado
  a la posicion de cada nodo en el lienzo. Morphing animado: los colores "respiran"
  rotando los parametros del fractal en tiempo real.
- **Analisis de arbol** -- Verifica si el grafo es un arbol conexo y aciclico. Calcula altura,
  grado, hojas, ancestros, descendientes, hermanos, primos y ramas mas larga y mas corta.
  Layout jerarquico automatico.
- **Isomorfismo** -- Verifica si dos grafos comparten la misma estructura topologica.
  Editor de doble grafo con generacion aleatoria de candidatos.
- **Planaridad** -- Verificacion de la cota de Euler (|E| <= 3|V| - 6), deteccion
  heuristica de subdivisiones de K5 y K3.3 (teorema de Kuratowski), deteccion de cruces
  entre aristas con resaltado visual.
- **Generadores Fractales** -- Construccion automatizada de topologias matematicas complejas basadas en patrones recursivos. Incluye 5 generadores: Triangulo de Sierpinski, Mandala Geometrico (Planar), Arbol Fractal, Copo de Nieve de Koch y Malla Hexagonal. Ideales para pruebas de estres algoritmico y aplicacion de teorias de coloreo dinamico.
- **Detector Automático de Propiedades** -- Identificación en tiempo real de características del grafo en la barra lateral: Conexo, Bipartito, Árbol, Completo, Regular y Euleriano.
- **Euler y Hamilton** -- Búsqueda de circuitos y caminos eulerianos (algoritmo de Hierholzer) y caminos hamiltonianos mediante backtracking. Detecta automáticamente si el grafo es euleriano.

### Simulacion de red

- Simulacion de trafico a nivel de paquetes con colores por protocolo: HTTP (verde),
  video (purpura), ping (cian), DDoS (rojo), VoIP (naranja), DNS (azul), FTP (marron).
- Visualizacion de paquetes en transito con estela (trail) y renderizado de anillos de
  salud en nodos.
- Animacion de brillo en aristas (edge glow) que refleja el nivel de uso.
- Flujos de trafico configurables: origen, destino, ancho de banda, duracion.
- Presets de trafico: videollamada, empresarial, DDoS y supervivencia.
- Simulacion de fallos: derribo y restauracion de nodos y enlaces con recalculo de rutas.
- Eventos automaticos: spikes de trafico y microcortes aleatorios.
- Estadisticas en vivo con sparklines: throughput, packet loss, latencia promedio.
- Inspector de paquetes: informacion detallada de cualquier paquete en transito.
- Timeline grafico de eventos con marcadores por tipo (corte, spike, sobrecarga,
  restauracion).
- Log de eventos de red con severidad (informacion, advertencia, error).
- Jitter configurable: variacion aleatoria de latencia en enlaces.

### Sonido

- Motor de audio espacial basado en miniaudio con generacion sintetica de formas de onda.
- Sonidos diferenciados para cada tipo de evento algoritmico: visita de nodo (ping suave),
  confirmacion de ruta (acorde ascendente), descarte (descenso), finalizacion de algoritmo
  (fanfarria), envio de paquete (chirrido), nodo caido (alarma), arista saturada (rumble
  grave), error (tono desafinado).
- Sonido exclusivo de triunfo para Dijkstra (arpegio con acorde sostenido).
- Supresion automatica de sonidos cuando la velocidad de animacion es alta, para evitar
  saturacion auditiva.
- Volumen global ajustable desde la interfaz.

### Interfaz de usuario

- Dos modos principales: Grafos (algoritmos) y Redes (simulacion de trafico).
- Paneles acoplables con layout adaptativo que se reconfigura al cambiar de modo.
- Lienzo interactivo con zoom, pan, seleccion de nodos y arrastre.
- Menu contextual en el lienzo con clic derecho.
- Panel de propiedades del nodo seleccionado con edicion de nombre y tipo de hardware.
- Tabla de aristas editable con pesos modificables.
- Tema oscuro "engineering" con paleta turquesa profesional.
- Barra de estado con informacion del grafo, backend de audio y FPS.
- Panel de registro (log) con historial de operaciones.
- Dialogo modal de informacion sobre la aplicacion.

### Persistencia

- Guardado y carga de proyectos en formato JSON mediante nlohmann/json.
- Serializacion completa del grafo: nodos, aristas, posiciones, nombres y pesos.

---

## Requisitos

- Compilador con soporte C++17 (GCC 9+, Clang 10+, MSVC 2019+).
- CMake 3.20 o superior.
- GPU compatible con OpenGL 3.3.
- Sistema Linux (primario). Compatible con Windows mediante ajustes menores en el backend
  de audio.

Todas las dependencias se descargan automaticamente via `FetchContent`:

- GLFW 3.4 (creacion de ventana y contexto OpenGL)
- Dear ImGui (interfaz grafica, branch docking)
- nlohmann/json 3.11.3 (serializacion JSON)
- miniaudio (motor de audio)
- GoogleTest v1.14.0 (tests unitarios)

No es necesario instalar dependencias adicionales del sistema mas alla de las
librerias de desarrollo de OpenGL, pthreads y el cargador dinamico (`dlopen`).

En distribuciones basadas en Debian/Ubuntu:

```
sudo apt install cmake libglfw3-dev libxkbcommon-dev
```

En distribuciones basadas en Arch Linux:

```
sudo pacman -S cmake glfw-x11 wayland
```

---

## Compilacion

### Linux (GCC/Clang)
```bash
git clone <repositorio>
cd GraphCore
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Windows (MinGW / MSYS2)
Si usas Windows con MinGW-w64 (a traves de MSYS2), asegúrate de instalar las herramientas base y compilar así:
```bash
# Instalar toolchain en MSYS2 MSYS terminal
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake

# En la terminal MSYS2 MinGW 64-bit:
git clone <repositorio>
cd GraphCore
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Ejecucion

### Linux
```bash
./build/graph-core
```

### Windows
```cmd
.\build\graph-core.exe
```

## Tests

```bash
./build/test_grafo && ./build/test_dijkstra && ./build/test_unionfind
```

O mediante CTest:

```bash
cd build && ctest
```

---

## Uso

La aplicacion presenta dos modos de operacion seleccionables desde el panel lateral:

- **Modo Grafos**: acceso a todos los algoritmos de grafos (Dijkstra, Kruskal, BFS, DFS,
  ciclos, coloreo, isomorfismo, analisis de arbol, planaridad). Seleccione un algoritmo
  desde el menu general, configure los parametros (nodo origen/destino, pesos) y ejecute
  la animacion paso a paso o el calculo instantaneo.

- **Modo Redes**: simulacion de trafico de red sobre la topologia actual. Cargue una
  topologia predefinida (empresarial, malla, estrella, internet simplificado) o construya
  la propia. Inicie la simulacion y observe los paquetes viajando por la red. Use el
  inspector para ver detalles de cualquier paquete. Simule fallos en nodos y enlaces para
  observar el recalculo de rutas.

El lienzo central permite interactuar con el grafo:

- Arrastrar nodos con el raton para reposicionarlos.
- Rueda del raton para hacer zoom.
- Clic derecho en el lienzo para abrir el menu contextual de creacion de nodos y aristas.
- Clic en un nodo para seleccionarlo y editar sus propiedades en el panel lateral.
- Clic en un paquete en transito (modo Redes) para abrir el inspector.

---

## Estructura del proyecto

```
src/
  main.cpp                       Punto de entrada e inicializacion de la aplicacion.
  audio/
    Sonidos.h                    Motor de audio sintetico con generacion de formas de onda.
    miniaudio.h                  Libreria miniaudio (terceros).
    miniaudio_impl.cpp           Implementacion del backend de audio.
  interfaz/
    Interfaz.h                   Orquestador principal de la interfaz de usuario.
    componentes/                 Barra de estado, menu principal, dialogos, panel de log.
    estado/                      Estado de la aplicacion dividido por dominio.
    lienzo/                      Renderizado del canvas y manejo de entrada del raton.
    paneles/
      PanelGrafos.h              Panel de herramientas de grafos y algoritmos.
      PanelRed.h                 Panel de configuracion y control de simulacion de red.
      PanelHardware.h            Panel de despliegue de hardware.
      PanelIsomorfismo.h         Editor de doble grafo para verificacion de isomorfismo.
      Matrices.h                 Visualizacion de matrices de adyacencia e incidencia.
    util/                        Funciones de easing, animacion y colores.
  nucleo/
    Grafo.h                      Estructura de datos del grafo (nodos y aristas).
    SimuladorRed.h               Motor de simulacion de trafico de red.
    Topologias.h                 Generadores de topologias predefinidas.
    UnionFind.h                  Estructura Union-Find para deteccion de ciclos.
    algoritmos/
      Dijkstra.h                 Algoritmo de ruta mas corta.
      Kruskal.h                  Algoritmo de arbol de expansion minima.
      BFS.h                      Busqueda en anchura.
      DFS.h                      Busqueda en profundidad.
      Ciclos.h                   Deteccion de ciclos.
      Coloreo.h                  Coloreo greedy y Welsh-Powell.
      ColorFractal.h             Coloreo con modulacion fractal de Mandelbrot.
      Arbol.h                    Analisis de arboles.
      Isomorfismo.h              Verificacion de isomorfismo entre grafos.
      Planaridad.h               Deteccion de planaridad (Euler, Kuratowski).
    tipos/
      EstadoRed.h                Tipos de datos para la simulacion de red.
      PasoAnimacion.h            Estructura para animacion paso a paso.
      TipoHardware.h             Enumeracion de tipos de hardware de red.
  persistencia/
    SerializadorJSON.h           Serializacion y deserializacion JSON del grafo.
tests/
  test_grafo.cpp                 Tests unitarios para la estructura Grafo.
  test_dijkstra.cpp              Tests unitarios para el algoritmo Dijkstra.
  test_unionfind.cpp             Tests unitarios para Union-Find.
```

---

## Licencia

MIT. Ver el archivo LICENSE para mas informacion.
