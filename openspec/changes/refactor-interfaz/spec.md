# Spec: Refactor total de Interfaz.h — extracción modular

**Change**: refactor-interfaz
**Type**: Structural refactor (no behavioral changes)
**Modo**: openspec
**Propósito**: Reducir `Interfaz.h` de ~2500 a ≤300 líneas extrayendo módulos cohesionados en `util/`, `componentes/`, `paneles/` y `lienzo/`, eliminando dead code y agregando tests unitarios.

> Este spec describe transformaciones estructurales. No hay cambios en comportamiento visible ni en capabilidades del sistema. Todas las RQs usan la convención `Fase-NNN`.

---

## Fase 1: Preparación
Riesgo: **Bajo**

### Archivos a crear
- `src/interfaz/util/` — directorio (si no existe)

### Archivos a modificar
- `src/nucleo/tipos/Nodo.h` — reemplazar `#include "imgui.h"` por forward declaration de `struct ImVec2`

### Archivos a eliminar
- `src/nucleo/Grafo_old.h` — eliminación completa del archivo (543 líneas, dead code)

### Requisitos funcionales

- **RQ-101**: El archivo `src/nucleo/Grafo_old.h` DEBE ser eliminado del disco. Ningún otro archivo del proyecto DEBE referenciarlo.
- **RQ-102**: `Nodo.h` NO DEBE incluir `imgui.h` directamente. DEBE usar forward declaration `struct ImVec2;` y mover la directiva `#include "imgui.h"` SOLO a los archivos .cpp o headers de UI que la necesiten.
- **RQ-103**: El directorio `src/interfaz/util/` DEBE existir como destino para los archivos de la Fase 2.

### Criterios de verificación

- [ ] La app compila sin errores ni warnings nuevos
- [ ] `Grafo_old.h` no existe en el árbol de directorios
- [ ] `Nodo.h` no contiene `#include "imgui.h"` y usa `struct ImVec2;`
- [ ] `grep -r "Grafo_old" src/` no produce resultados
- [ ] La UI se renderiza correctamente (ImVec2 sigue funcionando por el include externo)

### Escenarios de prueba

- **ESC-101**: Compilación post-eliminación
  - GIVEN el archivo `Grafo_old.h` eliminado
  - WHEN se ejecuta `cmake --build build`
  - THEN la compilación DEBE finalizar sin errores ni warnings

- **ESC-102**: Forward declaration de ImVec2
  - GIVEN `Nodo.h` con `struct ImVec2;` en lugar de `#include "imgui.h"`
  - WHEN cualquier archivo que incluye `Nodo.h` compila
  - THEN no DEBE haber errores de tipo incompleto para `ImVec2` en `Nodo.h`

- **ESC-103**: Consistencia de includes
  - GIVEN `Grafo_old.h` eliminado
  - WHEN se busca cualquier referencia a `Grafo_old` en los archivos fuente
  - THEN no DEBE haber resultados

---

## Fase 2: Utilidades
Riesgo: **Bajo**

### Archivos a crear
- `src/interfaz/util/Easing.h` — funciones inline de easing: `easeInOutCubic`, `easeOutBack`, `easeOutElastic`
- `src/interfaz/util/Animacion.h` — struct `ParticulaAnimacion` (reemplaza `Particula` interna de `Interfaz.h`), funciones: `iniciar()`, `anular()`, `pausar()`, estado compartido de animación
- `src/interfaz/util/Colores.h` — helpers de color: constantes `ImU32`/`ImVec4` temáticas usadas en la UI (colores de nodos, aristas, paneles, logs)

### Archivos a modificar
- `src/interfaz/Interfaz.h` — eliminar las definiciones inline de easing y el struct `Particula` interno; reemplazar por `#include` de los nuevos archivos

### Requisitos funcionales

- **RQ-201**: `Easing.h` DEBE contener las funciones `easeInOutCubic`, `easeOutBack` y `easeOutElastic` como funciones `inline` o `static inline`. DEBE usar `#pragma once`. NO DEBE depender de ImGui ni de ningún otro header de UI.
- **RQ-202**: `Animacion.h` DEBE contener el struct `ParticulaAnimacion` (reemplazando el `Particula` anidado en `Interfaz.h`), el estado de animación (pasos, timer, velocidad, conjuntos de nodos/aristas visitados) y las funciones `iniciarAnimacion()`, `anularAnimacion()`, `pausarAnimacion()`. DEBE preservar las firmas exactas de `Interfaz.h`.
- **RQ-203**: `Colores.h` DEBE contener constantes `ImU32` o `ImVec4` con nombre semántico para colores usados en la UI (e.g., `COLOR_NODO_VISITADO`, `COLOR_ARISTA_CONFIRMADA`, `COLOR_PANEL_TITULO`). DEBE usar `#pragma once`. PUEDE ser un archivo pequeño (≤30 líneas).

### Criterios de verificación

- [ ] La app compila sin errores ni warnings nuevos
- [ ] `easeInOutCubic`, `easeOutBack`, `easeOutElastic` están disponibles desde `Interfaz.h` vía include
- [ ] `ParticulaAnimacion` reemplaza a `Particula` sin cambios de comportamiento
- [ ] Animaciones de algoritmos (Dijkstra, Kruskal, etc.) funcionan idéntico a antes
- [ ] No hay duplicación de código de easing entre `Interfaz.h` y `Easing.h`

### Escenarios de prueba

- **ESC-201**: Easing produce mismo output
  - GIVEN los mismos inputs `t = 0.0, 0.25, 0.5, 0.75, 1.0`
  - WHEN se llama `easeInOutCubic(t)`, `easeOutBack(t)`, `easeOutElastic(t)`
  - THEN los valores DEBEN coincidir con los producidos por el código original inline

- **ESC-202**: Ciclo de vida de animación
  - GIVEN una animación con 3 pasos
  - WHEN se llama `iniciarAnimacion(pasos)`, luego `pausarAnimacion()`, luego `anularAnimacion()`
  - THEN el estado interno DEBE reflejar correctamente activa/pausada/reseteada

- **ESC-203**: Sin dependencia de ImGui en Easing.h
  - GIVEN `Easing.h` incluye solo `<cmath>`
  - WHEN se compila un archivo que solo incluye `Easing.h`
  - THEN no DEBE haber errores de compilación

---

## Fase 3: Componentes UI
Riesgo: **Medio**

### Archivos a crear
- `src/interfaz/componentes/StatusBar.h` — barra inferior con modo de operación, estado de simulación, FPS y audio
- `src/interfaz/componentes/MenuPrincipal.h` — menú Archivo, Opciones, Acerca de
- `src/interfaz/componentes/Dialogos.h` — popups y modales (AcercaDe, confirmaciones, diálogos de archivo fallback)

### Archivos a modificar
- `src/interfaz/Interfaz.h` — eliminar las funciones `dibujarMenuBar()` y `dibujarStatusBar()`; reemplazar por includes

### Requisitos funcionales

- **RQ-301**: `StatusBar.h` DEBE contener la función `dibujarStatusBar()` con la barra inferior que muestra el modo actual (`Grafos`/`Redes`), estado de simulación (inicializada/detenida), FPS y estado del audio (volumen o "SIN AUDIO"). DEBE preservar el estilo visual exacto (colores, iconos FontAwesome, padding, layout).
- **RQ-302**: `MenuPrincipal.h` DEBE contener `dibujarMenuBar()` con las entradas Archivo (Nuevo, Cargar, Guardar, Salir), Opciones (control de volumen, versión) y Acerca de. DEBE preservar el uso de `portable-file-dialogs` y el popup fallback con `ImGui::OpenPopup`. DEBE llamar a `registrarLog()` (función que permanece en `Interfaz.h`).
- **RQ-303**: `Dialogos.h` DEBE contener los popups modales: AcercaDe, confirmaciones, diálogos de carga/guardado fallback. DEBE usar `ImGui::OpenPopup`/`ImGui::BeginPopupModal`. DEBE preservar cadenas en español.

### Criterios de verificación

- [ ] La app compila sin errores ni warnings nuevos
- [ ] La barra de estado se renderiza idéntica (colores, iconos, contenido)
- [ ] El menú Archivo → Nuevo/Cargar/Guardar/Salir funciona igual
- [ ] Los popups modales se abren y cierran correctamente
- [ ] FPS y volumen se actualizan en tiempo real en la barra de estado

### Escenarios de prueba

- **ESC-301**: Barra de estado muestra datos correctos
  - GIVEN la app corriendo en modo Grafos con audio funcional
  - WHEN se inspecciona la barra inferior
  - THEN DEBE mostrar "Grafos", FPS > 0, volumen actual y sin errores de audio

- **ESC-302**: Menú Archivo → Nuevo
  - GIVEN el menú Archivo abierto
  - WHEN se clickea "Nuevo Proyecto"
  - THEN el grafo DEBE limpiarse, la animación DEBE resetearse y el log DEBE registrar "Proyecto nuevo creado"

- **ESC-303**: Modal Acerca de
  - GIVEN el menú Acerca de abierto
  - WHEN se visualiza el modal
  - THEN DEBE mostrar "OptiClusters v4.0 — NetSim Pro" y "Motor avanzado de visualización de grafos"

---

## Fase 4: Paneles
Riesgo: **Alto**

### Archivos a crear
- `src/interfaz/paneles/PanelGrafos.h` — selector de modo, subpaneles: General, Dijkstra, Kruskal, BFS, DFS, Ciclos, Coloreo, Isomorfismo, Árbol; tabla de aristas editables, propiedades de nodo, controles de animación
- `src/interfaz/paneles/PanelRed.h` — topologías, simulación (iniciar/detener tick), tráfico, tabla de nodos, log de eventos
- `src/interfaz/paneles/PanelHardware.h` — despliegue de hardware (servidores, routers, etc.)
- `src/interfaz/paneles/PanelIsomorfismo.h` — G1/G2 dual: grafo izquierdo, grafo derecho, análisis y resultados

### Archivos a modificar
- `src/interfaz/Interfaz.h` — eliminar `dibujarPanelHerramientas()`, `dibujarSelectorModo()`, `dibujarMenuGeneral()`, `dibujarSubpanel{Dijkstra|Kruskal|BFS|DFS|Ciclos|Coloreo|Isomorfismo|Arbol}()`, `dibujarPropiedadesNodo()`, `dibujarControlesAnimacion()`, `dibujarPanelRedes()`, `dibujarPanelLogs()`, `dibujarMatrices()`, `dibujarMatrizAdyacencia()`, `dibujarMatrizIncidencia()`; reemplazar por includes

### Requisitos funcionales

- **RQ-401**: `PanelGrafos.h` DEBE contener el panel izquierdo "Herramientas de Red" completo: selector modo (Grafos/Redes), menú general con botones de algoritmos, subpaneles por algoritmo, tabla de aristas editables, propiedades del nodo seleccionado y controles de animación. DEBE preservar el switch-case de `ModoPanel` y la navegación con botón "Volver". (Precaución: alto acoplamiento con estado compartido en `Interfaz.h`.)
- **RQ-402**: `PanelRed.h` DEBE contener el panel "Panel de Red" con: selector de topología (cargar/predefinida), controles de simulación (iniciar/detener tick con slider de velocidad), tabla de nodos (ID, nombre, tipo, latencia, paquetes, estado) con botón de fallo, y log de eventos de simulación. DEBE incluir la gestión del popup "Crear Equipo".
- **RQ-403**: `PanelHardware.h` DEBE contener el panel de despliegue de hardware con la visualización de tipos de hardware disponibles y su asignación a nodos. DEBE corresponder a la funcionalidad de "Crear Equipo" y selección de tipo.
- **RQ-404**: `PanelIsomorfismo.h` DEBE contener la interfaz dual G1/G2: botones para cargar/limpiar cada grafo, análisis de isomorfismo con visualización de resultados (mapeo de nodos, matrices) y estado de edición (`iso_editando_g2`).

### Criterios de verificación

- [ ] La app compila sin errores ni warnings nuevos
- [ ] Cada subpanel de algoritmo se muestra correctamente al seleccionarlo
- [ ] La navegación Volver funciona en todos los subpaneles
- [ ] El panel de Red muestra topologías y controles de simulación
- [ ] Isomorfismo G1/G2 funciona con carga, edición y análisis
- [ ] Click, drag, zoom y popups funcionan exactamente igual
- [ ] El log de eventos registra las mismas entradas que antes

### Escenarios de prueba

- **ESC-401**: Navegación de paneles
  - GIVEN el panel "Herramientas de Red" visible
  - WHEN se clickea "Dijkstra" en el menú general
  - THEN el panel DEBE mostrar el subpanel Dijkstra con campos de origen, destino y botón "Ejecutar"
  - WHEN se clickea "Volver"
  - THEN el panel DEBE mostrar el menú general nuevamente

- **ESC-402**: Panel de Red con simulación
  - GIVEN modo Redes activo y simulación no iniciada
  - WHEN se selecciona una topología predefinida y se clickea "Iniciar"
  - THEN `sim_inicializada` DEBE ser `true` y el tick DEBE ejecutarse
  - WHEN se clickea "Detener"
  - THEN la simulación DEBE pausarse sin pérdida de estado

- **ESC-403**: Isomorfismo dual
  - GIVEN el subpanel Isomorfismo activo y dos grafos cargados (G1, G2)
  - WHEN se clickea "Analizar Isomorfismo"
  - THEN `iso_analizado` DEBE ser `true` y el resultado DEBE mostrar si son isomorfos

---

## Fase 5: Lienzo
Riesgo: **Muy alto**

### Archivos a crear
- `src/interfaz/lienzo/LienzoRed.h` — canvas principal con render de nodos, aristas, paquetes/partículas; input handling (click para seleccionar, drag para mover, scroll para zoom, click derecho para menú contextual)

### Archivos a modificar
- `src/interfaz/Interfaz.h` — eliminar `dibujarLienzo()`, `#include <GLFW/glfw3.h>`, variables de panning (`offset_lienzo`), popups de creación de arista/equipo; reemplazar por include de `LienzoRed.h`

### Requisitos funcionales

- **RQ-501**: `LienzoRed.h` DEBE contener la función `dibujarLienzo()` con el canvas de render que dibuja nodos (círculos con etiqueta, color según estado), aristas (línea con peso, color según estado), animación de partículas/paquetes y efectos de hover/selección. DEBE manejar input: click izquierdo (seleccionar nodo, arrastrar), scroll (zoom), click derecho (menú contextual para crear arista/equipo). DEBE preservar el orden exacto de operaciones de render (fondo → aristas → nodos → partículas → overlays). DEBE incluir los popups contextuales "Crear Arista con peso" y "Crear Equipo (Redes)".

### Criterios de verificación

- [ ] La app compila sin errores ni warnings nuevos
- [ ] Click izquierdo selecciona nodos correctamente
- [ ] Drag mueve nodos con la misma suavidad y snapping que antes
- [ ] Scroll hace zoom in/out correctamente
- [ ] Click derecho en nodo/vacío muestra menú contextual
- [ ] Aristas se dibujan con peso, color y dirección correctos
- [ ] Partículas de animación cruzan aristas con el mismo timing y color
- [ ] Popups "Crear Arista" y "Crear Equipo" funcionan idéntico

### Escenarios de prueba

- **ESC-501**: Selección y arrastre de nodo
  - GIVEN un grafo con al menos un nodo visible
  - WHEN se clickea sobre un nodo
  - THEN `nodo_seleccionado` DEBE ser el ID del nodo clickeado y DEBE mostrar efecto visual de selección
  - WHEN se arrastra el nodo a una nueva posición
  - THEN `Nodo::posicion` DEBE actualizarse y el nodo DEBE redibujarse en la nueva posición

- **ESC-502**: Zoom con scroll
  - GIVEN el lienzo visible con nodos
  - WHEN se usa la rueda del scroll
  - THEN el lienzo DEBE hacer zoom in/out centrado en la posición del cursor

- **ESC-503**: Menú contextual y creación de arista
  - GIVEN un nodo seleccionado y click derecho sobre otro nodo
  - WHEN se selecciona "Conectar" en el menú contextual
  - THEN DEBE abrirse el popup "Crear Arista con peso"
  - WHEN se ingresa un peso y se confirma
  - THEN la arista DEBE aparecer en el grafo con el peso especificado

- **ESC-504**: Partícula de animación
  - GIVEN una animación activa (e.g., Dijkstra ejecutándose)
  - WHEN se avanza un paso que involucra una arista
  - THEN la partícula DEBE animarse desde `origen.posicion` a `destino.posicion` con el color correspondiente al tipo de paso

---

## Fase 6: Finalización
Riesgo: **Bajo**

### Archivos a crear
Ninguno (los archivos de tests se crean como parte de la infraestructura de testing)

### Archivos a modificar
- `src/interfaz/Interfaz.h` — reducir a orquestador delgado (~200-300 líneas): incluir módulos, delegar llamadas, mantener variables de estado global
- Tests unitarios para `Grafo.h`, `Dijkstra.h`, `UnionFind.h` (archivos nuevos en estructura de tests a definir)

### Requisitos funcionales

- **RQ-601**: `Interfaz.h` DEBE reducirse a un orquestador de ~200-300 líneas que incluye los nuevos módulos, declara las variables de estado compartido (selección, animación, simulación, algoritmos) y delega el dibujado a `dibujar()`. NO DEBE contener implementaciones de UI inline. DEBE incluir SOLO los headers de los módulos extraídos más los headers de dominio (`Grafo.h`, `SimuladorRed.h`, etc.). DEBE preservar la función pública `dibujar(Grafo&, GLFWwindow*)` con la misma firma.
- **RQ-602**: DEBEN existir tests unitarios para `Grafo.h`, `Dijkstra.h` y `UnionFind.h` que cubran: creación de nodos/aristas, Dijkstra con camino existente e inexistente, UnionFind con operaciones de unión/búsqueda. Los tests DEBEN compilar y ejecutarse sin errores.

### Criterios de verificación

- [ ] `Interfaz.h` tiene ≤ 300 líneas
- [ ] La app compila sin errores ni warnings nuevos
- [ ] Todos los tests unitarios pasan
- [ ] La app funciona exactamente igual que antes del refactor (verificación manual completa)
- [ ] `wc -l src/interfaz/Interfaz.h` ≤ 300

### Escenarios de prueba

- **ESC-601**: Interfaz.h como orquestador
  - GIVEN `Interfaz.h` reducido
  - WHEN se cuenta el número de líneas
  - THEN DEBE ser ≤ 300
  - WHEN se compila la app y se ejecuta
  - THEN todas las funciones de UI DEBEN estar disponibles y funcionar igual

- **ESC-602**: Tests de Grafo
  - GIVEN un grafo vacío
  - WHEN se agregan 3 nodos y 2 aristas
  - THEN `obtenerNodo(id)` DEBE devolver los nodos correctos y el conteo de aristas DEBE ser 2

- **ESC-603**: Tests de Dijkstra
  - GIVEN un grafo con nodos A-B-C donde A-B (peso 4) y B-C (peso 3), A-C (peso 10)
  - WHEN se ejecuta Dijkstra de A a C
  - THEN la ruta óptima DEBE ser A → B → C con distancia total 7

- **ESC-604**: Tests de UnionFind
  - GIVEN un UnionFind con 5 elementos
  - WHEN se unen (0,1), (1,2), (3,4)
  - THEN `find(0)` y `find(2)` DEBEN tener el mismo representante, y `find(2)` y `find(3)` DEBEN tener representantes distintos
