# GraphCore: Analizador Visual de Grafos y Redes Aéreas Mundiales

Versión 3.0.0 (Edición para Proyector / Defensa Académica). Aplicación de escritorio avanzada diseñada para la visualización interactiva y el estudio profundo de algoritmos aplicados sobre grafos matemáticos y redes de transporte reales.

Desarrollada con **C++17**, **Dear ImGui**, **GLFW**, y renderizado acelerado por **OpenGL 3.3**.

---

## Modos de Operación

### 1. Modo AeroGrafos (Simulación Global)
Modo centrado en una red logística real simulada que incluye **63 hubs aéreos internacionales**. Está diseñado para presentaciones académicas y análisis profundo de redes de transporte.

* **Algoritmos y Simulaciones:**
  * **Dijkstra:** Encuentra la ruta más corta garantizada calculando kilómetros reales mediante distancia Haversine. Incluye animaciones de búsqueda que muestran el progreso mediante ondas expansivas e indicadores de distancia.
  * **Kruskal (MST):** Construye el Árbol de Expansión Mínima (Minimum Spanning Tree) con animación paso a paso, visualizando el proceso de chequeo de ciclos y la selección óptima de aristas.
  * **BFS y DFS:** Búsquedas topológicas puras. La búsqueda en anchura (BFS) se ilustra con un efecto concéntrico tipo "sonar". La búsqueda en profundidad (DFS) detalla el avance en la red y las conexiones retrocesivas (back-edges).
  * **Coloreo Greedy (Aproximación):** Algoritmo secuencial que evita monopolios aéreos, asignando frecuencias o recursos únicos a aeropuertos vecinos para simular restricciones operativas.
  * **Análisis de Red:** Ejecuta un escaneo topológico de la infraestructura para reportar métricas vitales como densidad, grado promedio de los nodos y los hubs más críticos.
  * **Caminos de Euler y Hamilton:** Demostración de las limitaciones en la vida real de estos algoritmos sobre mapas mundiales no estructurados, evidenciando problemas de complejidad computacional (NP-Hard).
* **Restricción Geopolítica:** Sistema interactivo que permite simular el cierre de espacios aéreos (por ejemplo, rutas bloqueadas). El motor recalcula en tiempo real cómo los algoritmos de enrutamiento se adaptan bordeando continentes.

### 2. Modo Grafos Libres (Laboratorio Matemático)
Consiste en un entorno interactivo y lienzo libre donde es posible construir, modificar y analizar redes abstractas a medida.

* **Físicas de Partículas (Fruchterman-Reingold):** Motor de ordenamiento automático que desenreda estructuras caóticas aplicando fuerzas de repulsión (Coulomb) y atracción (Hooke).
* **Detector de Propiedades:** Panel analítico que informa en tiempo real las propiedades del grafo diseñado (ej. si es Bipartito, Euleriano, o Conexo).
* **Isomorfismo y Generadores:** Prueba de algoritmos matemáticos, incluyendo detección de isomorfismo, y herramientas de autogeneración de mallas fractales y topologías complejas.

---

## Estructura del Proyecto

La arquitectura completa del proyecto está organizada de la siguiente manera:

```text
.
├── build-all.sh
├── CMakeLists.txt
├── compile_commands.json -> build/compile_commands.json
├── .gitignore
├── graph-core
├── graph-core-logo.png
├── imgui.ini
├── mingw-toolchain.cmake
├── muestras
│   ├── arbol_binario.json
│   ├── arbol_jerarquico.json
│   ├── grafo_bipartito.json
│   ├── grafo_completo.json
│   ├── grafo_exotico_espiral.json
│   ├── grafo_intermedio_anillos.json
│   ├── grafo_malla.json
│   ├── grafo_normal_1.json
│   └── grafo_normal_2.json
├── README.md
├── recursos
│   ├── fuentes
│   │   ├── Font Awesome 6 Free-Solid-900.otf
│   │   ├── JetBrainsMono-Regular.ttf
│   │   └── Roboto-Regular.ttf
│   └── texturas
│       └── mundo_equirectangular.jpg
├── src
│   ├── audio
│   │   ├── miniaudio.h
│   │   ├── miniaudio_impl.cpp
│   │   └── Sonidos.h
│   ├── IconsFontAwesome6.h
│   ├── interfaz
│   │   ├── componentes
│   │   │   ├── Dialogos.h
│   │   │   ├── LogPanel.h
│   │   │   ├── MenuPrincipal.h
│   │   │   ├── StatusBar.h
│   │   │   └── Toolbar.h
│   │   ├── estado
│   │   │   ├── EstadoAeroGrafos.h
│   │   │   ├── EstadoGrafos.h
│   │   │   ├── EstadoRedes.h
│   │   │   └── EstadoUI.h
│   │   ├── Interfaz.h
│   │   ├── lienzo
│   │   │   ├── LienzoAeroGrafos.h
│   │   │   └── LienzoRed.h
│   │   ├── paneles
│   │   │   ├── Matrices.h
│   │   │   ├── PanelAeroGrafos.h
│   │   │   ├── PanelGrafos.h
│   │   │   ├── PanelHardware.h
│   │   │   └── PanelIsomorfismo.h
│   │   ├── util
│   │   │   ├── Animacion.h
│   │   │   ├── AnimacionUI.h
│   │   │   ├── Colores.h
│   │   │   ├── Easing.h
│   │   │   ├── stb_image.h
│   │   │   └── TextureLoader.h
│   │   └── ventanas
│   │       └── VentanaAyuda.h
│   ├── main.cpp
│   ├── nucleo
│   │   ├── algoritmos
│   │   │   ├── AnalizadorGrafo.h
│   │   │   ├── Arbol.h
│   │   │   ├── BFS.h
│   │   │   ├── Ciclos.h
│   │   │   ├── Coloreo.h
│   │   │   ├── ColorFractal.h
│   │   │   ├── DFS.h
│   │   │   ├── Dijkstra.h
│   │   │   ├── EulerHamilton.h
│   │   │   ├── ForceAtlas2.h
│   │   │   ├── Isomorfismo.h
│   │   │   ├── Kruskal.h
│   │   │   ├── Planaridad.h
│   │   │   ├── QuadTree.h
│   │   │   └── TopologiasFractales.h
│   │   ├── datos
│   │   │   └── DatosMundo.h
│   │   ├── Grafo.h
│   │   ├── SimuladorRed.h
│   │   ├── tipos
│   │   │   ├── Arista.h
│   │   │   ├── EstadoRed.h
│   │   │   ├── Nodo.h
│   │   │   ├── PasoAnimacion.h
│   │   │   └── TipoHardware.h
│   │   ├── Topologias.h
│   │   └── UnionFind.h
│   ├── persistencia
│   │   ├── SerializadorGEXF.h
│   │   └── SerializadorJSON.h
│   └── portable-file-dialogs.h
└── tests
    ├── test_bfs.cpp
    ├── test_ciclos.cpp
    ├── test_dijkstra.cpp
    ├── test_grafo.cpp
    └── test_unionfind.cpp
```

---

## Requisitos y Dependencias

* Compilador compatible con C++17 (GCC 9+, Clang 10+, MSVC 2019+)
* CMake 3.20 o superior
* Tarjeta gráfica compatible con OpenGL 3.3

*Dependencias integradas y obtenidas automáticamente en la compilación:*
* GLFW 3.4
* Dear ImGui
* nlohmann/json
* miniaudio
* GoogleTest

---

## Instalación y Compilación

### Entornos Linux (Debian / Ubuntu)

1. Instale las herramientas de desarrollo y dependencias básicas del sistema:
   ```bash
   sudo apt update
   sudo apt install cmake gcc g++ libglfw3-dev libxkbcommon-dev build-essential
   ```
2. Configure el proyecto utilizando CMake:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```
3. Compile el código fuente:
   ```bash
   cmake --build build -j$(nproc)
   ```
4. Ejecute la aplicación:
   ```bash
   ./build/graph-core
   ```

### Entornos Windows (MSYS2 / MinGW)

1. A través de la terminal de MSYS2, instale la cadena de herramientas:
   ```bash
   pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
   ```
2. Genere los archivos de construcción especificando el generador de MinGW:
   ```bash
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   ```
3. Compile el binario:
   ```bash
   cmake --build build -j$(nproc)
   ```
4. Ejecute la aplicación generada:
   ```cmd
   .\build\graph-core.exe
   ```

*(Alternativamente, puede utilizar el script `build-all.sh` incluido en la raíz si dispone de un entorno compatible).*

---

## Guía de Uso

Una vez ejecutada la aplicación, se desplegará el menú principal donde podrá navegar entre los distintos modos. 

### Navegación General
* **Paneles de Control:** Utilice las barras laterales para configurar parámetros de visualización, como la velocidad de las animaciones, la activación del sonido o el modo de representación.
* **Cámara y Movimiento:** En los entornos gráficos, puede arrastrar el lienzo con el ratón y hacer uso de la rueda de desplazamiento para aplicar zoom sobre secciones específicas del grafo.
* **Manual Integrado:** Presione en cualquier momento el botón de Ayuda para abrir una enciclopedia detallada con la teoría, casos de uso y la complejidad computacional teórica de cada algoritmo.

### Modo AeroGrafos
* **Selección de Origen y Destino:** Haga clic sobre los nodos que representan aeropuertos (hubs) para establecer la ruta de partida y el destino deseado.
* **Ejecución de Algoritmos:** Seleccione un algoritmo específico desde el panel (Dijkstra, BFS, DFS, etc.) y presione iniciar para observar su comportamiento iterativo en tiempo real.
* **Alteración de Rutas (Cierres de Espacio Aéreo):** Inhabilite selectivamente nodos específicos para simular un cierre de rutas y observar cómo el motor recalcula alternativas inmediatas.

### Modo Laboratorio (Grafos Libres)
* **Creación Manual:** Añada nodos al hacer clic en zonas vacías del lienzo y genere conexiones (aristas) arrastrando el ratón desde un nodo de origen hasta uno de destino.
* **Uso de Generadores:** A través del menú de topologías puede cargar modelos predefinidos (ubicados en el directorio `muestras/`) o autogenerar grafos aleatorios, bipartitos y mallas estructurales de forma automatizada.
* **Análisis en Tiempo Real:** Observe las métricas matemáticas del grafo en el panel lateral. Estas se actualizan instantáneamente con cualquier adición o remoción estructural.

---
*Desarrollado para la materia de Optimización y Algoritmia Avanzada. Licencia MIT.*
