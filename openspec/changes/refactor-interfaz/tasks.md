# Tasks: Refactor total de Interfaz.h — extracción modular

Decision needed before apply: Yes
Chained PRs recommended: Yes
Chain strategy: pending
400-line budget risk: High

## Review Workload Forecast

| Field | Value |
|-------|-------|
| Estimated changed lines | ~2850 (542 del + 1150 new in modules + 300 tests + ~860 restructured) |
| 400-line budget risk | **High** |
| Chained PRs recommended | **Yes** |
| Suggested split | PR1 (Fase 1-2-3) → PR2 (Fase 4) → PR3 (Fase 5-6) |
| Delivery strategy | ask-on-risk |

### Suggested Work Units

| Unit | Goal | Likely PR | Notes |
|------|------|-----------|-------|
| 1 | Utilidades + Componentes UI | PR 1 | Fases 1-2-3. ~500 líneas netas nuevas. Base: main |
| 2 | Paneles | PR 2 | Fase 4. ~1400 líneas nuevas. Base: main |
| 3 | Lienzo + Finalización | PR 3 | Fases 5-6. ~800 líneas nuevas + tests. Base: main |

---

## Fase 1 — Preparación

### T001: Eliminar Grafo_old.h
**Fase**: 1
**Depende de**: —
**Archivos**: `src/nucleo/Grafo_old.h` (eliminar, 542 líneas), `CMakeLists.txt` (si hay referencia)
**Riesgo**: Bajo

**Pasos exactos**:
1. Verificar que `grep -r "Grafo_old" src/` no devuelva resultados (ya confirmado: 0 referencias)
2. Eliminar `src/nucleo/Grafo_old.h`
3. Re-compilar: `cmake --build build`

**Criterios de aceptación**:
- [ ] `src/nucleo/Grafo_old.h` no existe en disco
- [ ] `grep -r "Grafo_old" src/` produce 0 resultados
- [ ] Compila sin errores

---

### T002: Forward-declare ImVec2 en Nodo.h
**Fase**: 1
**Depende de**: —
**Archivos**: `src/nucleo/tipos/Nodo.h` (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. En `Nodo.h` línea 3, reemplazar `#include "imgui.h"` por `struct ImVec2;`
2. Agregar `#include "imgui.h"` a `src/nucleo/Grafo.h` ANTES de `#include "tipos/Nodo.h"` (línea 8)
3. Verificar que todos los archivos que incluyen `Grafo.h` tengan acceso a ImVec2 mediante `imgui.h` incluido antes

**Detalle técnico**: `Nodo.h` usa `ImVec2` como member value (`posicion`, línea 10) y parámetro de constructor (línea 14). La forward declaration SOLA no basta — se necesita que `imgui.h` esté incluido antes que `Nodo.h` en cada TU. `Grafo.h` incluye `Nodo.h` y es incluido por `Interfaz.h` que ya tiene `imgui.h` en línea 3. Se agrega `#include "imgui.h"` a `Grafo.h` como safety net.

**Criterios de aceptación**:
- [ ] `Nodo.h` no contiene `#include "imgui.h"`
- [ ] `Nodo.h` tiene `struct ImVec2;` antes de la definición del struct
- [ ] `Grafo.h` tiene `#include "imgui.h"` antes de `#include "tipos/Nodo.h"`
- [ ] Compila sin errores

---

### T003: Crear directorios de módulos
**Fase**: 1
**Depende de**: —
**Archivos**: crear directorios
**Riesgo**: Bajo

**Pasos exactos**:
1. Crear `src/interfaz/util/`
2. Crear `src/interfaz/componentes/`
3. Crear `src/interfaz/paneles/`
4. Crear `src/interfaz/lienzo/`

**Criterios de aceptación**:
- [ ] Los 4 directorios existen

---

## Fase 2 — Utilidades (paralelizable)

### T004: Crear util/Easing.h
**Fase**: 2
**Depende de**: T003
**Archivos**: `src/interfaz/util/Easing.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Bajo

**Pasos exactos**:
1. Crear `src/interfaz/util/Easing.h` con `#pragma once`, `#include <cmath>`
2. En `namespace Easing {}`, copiar las funciones desde `Interfaz.h` líneas 143-162:
   - `inline float easeInOutCubic(float t)` (líneas 143-147)
   - `inline float easeOutBounce(float t)` (líneas 149-162)
3. AGREGAR (no existen en el original): `inline float easeOutBack(float t)` y `inline float easeOutElastic(float t)` según la spec
4. En `Interfaz.h`: eliminar líneas 142-162 (todo el bloque de easing), agregar `#include "util/Easing.h"`
5. Actualizar llamadas: en `Interfaz.h` línea 2018 (`easeOutBounce`) y línea 2084 (`easeInOutCubic`), anteponer `Easing::` si se usa namespace

**Variables extraídas**: Ninguna (stateless)
**Funciones movidas**: `easeInOutCubic`, `easeOutBounce` (existentes) + `easeOutBack`, `easeOutElastic` (nuevas)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] `easeInOutCubic(0.0)=0.0`, `easeInOutCubic(0.5)=0.5`, `easeInOutCubic(1.0)=1.0`
- [ ] `Easing.h` no incluye `imgui.h`

---

### T005: Crear util/Animacion.h
**Fase**: 2
**Depende de**: T004
**Archivos**: `src/interfaz/util/Animacion.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear `src/interfaz/util/Animacion.h` con `#pragma once`, includes: `Easing.h`, `PasoAnimacion.h`, `<set>`, `<map>`, `<vector>`, `<cmath>`, `<algorithm>`
2. Definir struct `ParticulaAnimacion` (reemplaza `Particula` de Interfaz.h líneas 128-140):
   ```cpp
   struct ParticulaAnimacion {
       bool activa = false;
       ImVec2 pos_inicio, pos_fin;
       float progreso = 0.0f, duracion = 0.3f;
       ImU32 color = IM_COL32(0, 255, 200, 255);
       float radio = 6.0f;
   };
   ```
3. Definir `namespace Animacion {}` con struct `EstadoAnimacion` que contenga:
   - `std::vector<PasoAnimacion> pasos` (← `pasos_animacion`, línea 116)
   - `int paso_actual = -1` (← línea 117)
   - `float timer_paso = 0.0f` (← línea 118)
   - `float velocidad_paso = 0.5f` (← línea 119)
   - `bool activa = false` (← `animacion_activa`, línea 120)
   - `bool pausada = false` (← `animacion_pausada`, línea 121)
   - `std::set<int> visitados` (← `nodos_visitados`, línea 122)
   - `std::set<int> procesando` (← `nodos_procesando`, línea 123)
   - `std::set<std::pair<int,int>> exploradas` (← `aristas_exploradas`, línea 124)
   - `std::set<std::pair<int,int>> confirmadas` (← `aristas_confirmadas`, línea 125)
   - `std::set<std::pair<int,int>> descartadas` (← `aristas_descartadas`, línea 126)
   - `ParticulaAnimacion particula` (← `particula_activa`, línea 139)
   - `std::map<int, float> tiempo_visita_nodo` (← línea 140)
4. En `Animacion::`, implementar funciones:
   - `inline void iniciar(EstadoAnimacion& est, std::vector<PasoAnimacion> pasos)` ← `iniciarAnimacion()` líneas 2416-2427 (sin llamadas a registrarLog ni g_sonidos)
   - `inline void aplicarPaso(EstadoAnimacion& est, const PasoAnimacion& p)` ← `aplicarPaso()` líneas 2429-2462 (sin g_sonidos)
   - `inline void reset(EstadoAnimacion& est)` ← `resetAnimacion()` líneas 2464-2477
   - `inline void pausar(EstadoAnimacion& est)` — NUEVA: toggle `est.pausada = !est.pausada`
5. En `Interfaz.h`:
   - Agregar `#include "util/Animacion.h"`
   - Agregar miembro: `Animacion::EstadoAnimacion anim_estado;`
   - Eliminar líneas 116-140 (variables de animación + struct Particula)
   - Eliminar funciones: `iniciarAnimacion` (2416-2427), `aplicarPaso` (2429-2462), `resetAnimacion` (2464-2477)
   - Reemplazar referencias: `pasos_animacion` → `anim_estado.pasos`, `animacion_activa` → `anim_estado.activa`, etc.
6. En llamadas a `aplicarPaso` (ej. línea 213), cambiar a `Animacion::aplicarPaso(anim_estado, paso)`

**Variables extraídas**: Todas las de animación + Particula → struct `EstadoAnimacion`
**Mecanismo de acceso**: Miembro `anim_estado` en Interfaz.h. Módulos acceden vía `self.anim_estado`

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Animación de Dijkstra/Kruskal/BFS/DFS funciona idéntico
- [ ] Partículas cruzan aristas con el mismo timing y color
- [ ] Pausar/Reset funcionan

---

### T006: Crear util/Colores.h
**Fase**: 2
**Depende de**: T003
**Archivos**: `src/interfaz/util/Colores.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Bajo

**Pasos exactos**:
1. Crear `src/interfaz/util/Colores.h` con `#pragma once`, includes: `imgui.h`, `IconsFontAwesome6.h`
2. En `namespace Colores {}`, definir constantes semánticas. Extraer valores literales de `Interfaz.h`:
   - `NODO_VISITADO = IM_COL32(0, 230, 118, 200)` (línea 1948)
   - `NODO_PROCESANDO = IM_COL32(255, 215, 0, 200)` (línea 1945)
   - `ARISTA_CONFIRMADA = IM_COL32(255, 179, 0, 255)` (línea 1812)
   - `ARISTA_EXPLORADA = IM_COL32(0, 188, 212, 220)` (línea 1819)
   - `ARISTA_DESCARTADA_BASE = IM_COL32(255, 68, 68, ...)` (línea 1826)
   - `PARTICULA_EXPLORAR = IM_COL32(0, 200, 255, 255)` (línea 229)
   - `PARTICULA_CONFIRMAR = IM_COL32(255, 180, 0, 255)` (línea 232)
   - `PARTICULA_DESCARTAR = IM_COL32(255, 60, 60, 200)` (línea 236)
   - `PAQUETE_PING = IM_COL32(0, 255, 100, 220)` (línea 2092)
   - `PAQUETE_HTTP = IM_COL32(80, 180, 255, 220)` (línea 2093)
   - `PAQUETE_VIDEO = IM_COL32(255, 150, 50, 220)` (línea 2094)
   - `PAQUETE_DDOS = IM_COL32(255, 50, 50, 220)` (línea 2095)
3. En `Interfaz.h`: agregar `#include "util/Colores.h"`
4. Reemplazar literales duplicados en el código con `Colores::NOMBRE_CONSTANTE` (esto puede hacerse progresivamente en fases 3-5)

**Variables extraídas**: Ninguna (solo constantes)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] `Colores.h` es ≤40 líneas
- [ ] Colores de nodos/aristas/partículas son visualmente idénticos

---

## Fase 3 — Componentes UI (paralelizable)

### T007: Crear componentes/StatusBar.h
**Fase**: 3
**Depende de**: T003
**Archivos**: `src/interfaz/componentes/StatusBar.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Bajo

**Pasos exactos**:
1. Crear `src/interfaz/componentes/StatusBar.h` con `#pragma once`, incluye `Interfaz.h`
2. En `namespace StatusBar {}`, crear:
   ```cpp
   inline void dibujar(Interfaz& self);
   ```
3. Mover el cuerpo de `dibujarStatusBar()` (líneas 2363-2413) a la función, cambiando accesos directos a `self.`:
   - `modo_actual` → `self.modo_actual`
   - `animacion_activa` → `self.anim_estado.activa`
   - `simulador` → `self.simulador`
   - `sim_inicializada` → `self.sim_inicializada`
   - `simulacion_jitter` → `self.simulacion_jitter`
   - `g_sonidos` se queda como está (extern)
4. En `Interfaz.h`:
   - Agregar `#include "componentes/StatusBar.h"`
   - Eliminar función `dibujarStatusBar()` (líneas 2362-2413)
   - En `dibujar()` línea 308, reemplazar `dibujarStatusBar()` por `StatusBar::dibujar(*this)`

**Variables accedidas**: `modo_actual`, `anim_estado`, `simulador`, `sim_inicializada`, `simulacion_jitter`, `g_sonidos`

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] StatusBar muestra modo, FPS, estado simulación y audio idéntico

---

### T008: Crear componentes/MenuPrincipal.h
**Fase**: 3
**Depende de**: T003
**Archivos**: `src/interfaz/componentes/MenuPrincipal.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear `src/interfaz/componentes/MenuPrincipal.h` con `#pragma once`, incluye `Interfaz.h`, `persistencia/SerializadorJSON.h`, `portable-file-dialogs.h`
2. En `namespace MenuPrincipal {}`:
   ```cpp
   inline void dibujar(Interfaz& self, Grafo& red, GLFWwindow* ventana);
   ```
3. Mover cuerpo de `dibujarMenuBar()` (líneas 313-379) a la función, cambiando a `self.`:
   - `self.resetAnimacion()` → `Animacion::reset(self.anim_estado)`
   - `self.ruta_optima`, `self.aristas_mst`, `self.mostrar_mst`
   - `self.registrarLog(...)` → se queda como `self.registrarLog()` (sigue en Interfaz)
4. En `Interfaz.h`:
   - Agregar `#include "componentes/MenuPrincipal.h"`
   - Eliminar `dibujarMenuBar()` (líneas 312-379)
   - En `dibujar()` línea 254, reemplazar por `MenuPrincipal::dibujar(*this, red, ventana)`

**Variables accedidas**: `ruta_optima`, `aristas_mst`, `mostrar_mst`, `anim_estado`, `modo_panel`, `system_logs` (via registrarLog), y llama a funciones `resetAnimacion`, `registrarLog` que ahora son `Animacion::reset`, `this->registrarLog`

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Menú Archivo (Nuevo/Cargar/Guardar/Salir) funciona
- [ ] Opciones de audio funcionan
- [ ] Acerca de se abre desde el menú

---

### T009: Crear componentes/Dialogos.h
**Fase**: 3
**Depende de**: T003
**Archivos**: `src/interfaz/componentes/Dialogos.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear `src/interfaz/componentes/Dialogos.h` con `#pragma once`, incluye `Interfaz.h`, `persistencia/SerializadorJSON.h`
2. En `namespace Dialogos {}`:
   - `inline void acercaDe()` — contenido del menú "Acerca de" actualmente en líneas 371-376 (inline en MenuPrincipal, extraer a diálogo modal)
   - `inline void fallbackCargar(Interfaz& self, Grafo& red)` — contenido actualmente en líneas 2134-2154 (DENTRO de `dibujarLienzo`)
   - `inline void fallbackGuardar(Interfaz& self, Grafo& red)` — contenido actualmente en líneas 2157-2177 (DENTRO de `dibujarLienzo`)
3. En `Interfaz.h`:
   - Agregar `#include "componentes/Dialogos.h"`
   - Eliminar los popups FallbackCargar y FallbackGuardar de `dibujarLienzo()` (líneas 2133-2177)
   - Agregar llamada `Dialogos::acercaDe()` en el lugar del menú (y en `dibujar()` loop o donde se manejen popups)
   - Agregar llamadas `Dialogos::fallbackCargar(*this, red)` y `Dialogos::fallbackGuardar(*this, red)` en `dibujar()`

**Variables accedidas**: `system_logs` (via registrarLog), `anim_estado`, `ruta_optima`, `aristas_mst`, `mostrar_mst`

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Modal "Acerca de" muestra "OptiClusters v4.0 — NetSim Pro" correctamente
- [ ] Fallback de carga/guardado funciona desde menú Archivo

---

## Fase 4 — Paneles

### T010: Crear paneles/PanelGrafos.h
**Fase**: 4
**Depende de**: T005, T006, T008
**Archivos**: `src/interfaz/paneles/PanelGrafos.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Alto

**Pasos exactos**:
1. Crear `src/interfaz/paneles/PanelGrafos.h` con `#pragma once`, incluye `Interfaz.h` y todos los algoritmos (`Dijkstra.h`, `Kruskal.h`, `BFS.h`, `DFS.h`, `Ciclos.h`, `Coloreo.h`, `Isomorfismo.h`, `Arbol.h`)
2. En `namespace PanelGrafos {}`, mover las siguientes funciones DESDE Interfaz.h (como `self.`):
   - `dibujarSelectorModo(Grafo& red)` ← líneas 382-404
   - `dibujarControlesAnimacion()` ← líneas 406-454
   - `dibujarPropiedadesNodo(Grafo& red)` ← líneas 456-511
   - `dibujarPanelHerramientas(Grafo& red)` ← líneas 513-553
   - `dibujarMenuGeneral(Grafo& red)` ← líneas 555-633
   - `dibujarSubpanelDijkstra(Grafo& red)` ← líneas 635-733
   - `dibujarSubpanelKruskal(Grafo& red)` ← líneas 735-791
   - `dibujarSubpanelBFS(Grafo& red)` ← líneas 793-862
   - `dibujarSubpanelDFS(Grafo& red)` ← líneas 864-923
   - `dibujarSubpanelCiclos(Grafo& red)` ← líneas 925-962
   - `dibujarSubpanelColoreo(Grafo& red)` ← líneas 964-1005
   - `dibujarSubpanelIsomorfismo(Grafo& red)` ← líneas 1007-1143
   - `aplicarLayoutArbol(Grafo& red, ...)` ← líneas 1145-1172
   - `dibujarSubpanelArbol(Grafo& red)` ← líneas 1174-1375
3. Firmas con `Interfaz& self` como primer parámetro. Ejemplo:
   ```cpp
   inline void selectorModo(Interfaz& self, Grafo& red);
   ```
4. Cada variable miembro de Interfaz usada se accede como `self.xxx`:
   - `modo_actual`, `modo_panel`, `nodo_seleccionado`, `buffer_nombre`
   - `anim_estado.pasos`, `anim_estado.paso_actual`, `anim_estado.activa`, `anim_estado.pausada`, `anim_estado.velocidad_paso`
   - `dijkstra_origen`, `dijkstra_destino`, `dijkstra_usar_latencia`, `dijkstra_tabla_dist`, `ruta_optima`
   - `aristas_mst`, `mostrar_mst`, `colores_nodos`, `mostrar_coloreo`
   - `tiene_ciclo`, `ciclo_analizado`, `resultado_ciclos`, `resultado_coloreo`
   - `bfs_nodo_inicio`, `bfs_resultado`, `dfs_nodo_inicio`, `dfs_resultado`
   - `grafo_iso_g2`, `iso_analizado`, `iso_editando_g2`, `resultado_iso`
   - `arbol_props`, `arbol_analizado`, `arbol_raiz_id`, `arbol_layout_aplicado`
   - `simulacion_jitter`, `jitter_porcentaje`, `modo_actual`
   - Llamadas a `self.resetGrafoIsomorfismo()` y `self.registrarLog()`
5. Reemplazar llamadas a `iniciarAnimacion`, `resetAnimacion` por `Animacion::iniciar(self.anim_estado, ...)`, `Animacion::reset(self.anim_estado)`
6. En `Interfaz.h`:
   - Agregar `#include "paneles/PanelGrafos.h"`
   - Eliminar líneas 381-1375 (todo el bloque de funciones de PanelGrafos)
   - En `dibujar()` línea 301, reemplazar `dibujarPanelHerramientas(red)` por `PanelGrafos::dibujarPanelHerramientas(*this, red)`

**Variables accedidas**: ~30 variables de estado de algoritmos + animación + selección + modo (ver diseño §State Ownership Map)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Selector de modo (Grafos/Redes) funciona
- [ ] Navegación entre subpaneles funciona (General → Dijkstra → Volver, etc.)
- [ ] Cada subpanel ejecuta su algoritmo correctamente
- [ ] Controles de animación (Play/Pausa/Paso/Reset) funcionan
- [ ] Tabla de aristas editables funciona
- [ ] Propiedades del nodo seleccionado se muestran

---

### T011: Crear paneles/PanelRed.h
**Fase**: 4
**Depende de**: T005
**Archivos**: `src/interfaz/paneles/PanelRed.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Alto

**Pasos exactos**:
1. Crear `src/interfaz/paneles/PanelRed.h` con `#pragma once`, incluye `Interfaz.h`, `nucleo/Topologias.h`, `nucleo/SimuladorRed.h`
2. En `namespace PanelRed {}`:
   ```cpp
   inline void dibujar(Interfaz& self, Grafo& red);
   ```
3. Mover cuerpo de `dibujarPanelRedes()` (líneas 1378-1613) a la función con `self.`:
   - `sim_inicializada`, `simulador`, `mostrar_modal_inicio`
   - `flujo_origen`, `flujo_destino`, `flujo_mbps`, `flujo_tipo`, `flujo_dur`
   - `fallo_nodo_id`, `fallo_arista_org`, `fallo_arista_dst`
   - `nodo_seleccionado`, `ruta_optima`, `aristas_mst`, `mostrar_mst`, `anim_estado` (para reset)
   - `g_sonidos`
   - `self.registrarLog()`
4. En `Interfaz.h`:
   - Agregar `#include "paneles/PanelRed.h"`
   - Eliminar `dibujarPanelRedes()` (líneas 1377-1613)
   - En `dibujar()` líneas 302-304, reemplazar por `PanelRed::dibujar(*this, red)`

**Variables accedidas**: `simulador`, `sim_inicializada`, `mostrar_modal_inicio`, `flujo_*`, `fallo_*`, `nodo_seleccionado`, `ruta_optima`, `aristas_mst`, `g_sonidos`

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Topologías predefinidas se cargan
- [ ] Iniciar/Detener simulación funciona
- [ ] Enviar tráfico HTTP/Video/Ping/DDoS funciona
- [ ] Fallo de nodos funciona
- [ ] Tabla de estado de nodos se muestra
- [ ] Log de eventos de red se muestra

---

### T012: Crear paneles/PanelHardware.h
**Fase**: 4
**Depende de**: T003
**Archivos**: `src/interfaz/paneles/PanelHardware.h` (crear), `src/interfaz/Interfaz.h` (modificar), `src/interfaz/lienzo/LienzoRed.h` (crear en Fase 5)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear `src/interfaz/paneles/PanelHardware.h` con `#pragma once`, incluye `Interfaz.h`, `nucleo/tipos/TipoHardware.h`
2. En `namespace PanelHardware {}`:
   ```cpp
   inline void desplegar(Interfaz& self, Grafo& red, const ImVec2& pos_click);
   ```
3. Mover el popup "CrearEquipo" (actualmente en LienzoRed, líneas 1736-1752) a esta función. Parámetros:
   - `self.pos_click_derecho` → `pos_click` (pasado por LienzoRed)
   - `self.ruta_optima`, `self.aristas_mst`, `self.mostrar_mst`
   - `self.registrarLog()`
4. En `LienzoRed.h` (cuando se cree): reemplazar el popup inline por llamada a `PanelHardware::desplegar(*this, grafo_actual, pos_click_derecho)`
5. En `Interfaz.h`: agregar `#include "paneles/PanelHardware.h"`

**Variables accedidas**: `ruta_optima`, `aristas_mst`, `mostrar_mst`, `system_logs` (via registrarLog)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Click derecho en modo Redes abre popup de creación de equipo
- [ ] Los 5 tipos de hardware se listan correctamente

---

### T013: Crear paneles/PanelIsomorfismo.h
**Fase**: 4
**Depende de**: T005, T010
**Archivos**: `src/interfaz/paneles/PanelIsomorfismo.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear `src/interfaz/paneles/PanelIsomorfismo.h` con `#pragma once`, incluye `Interfaz.h`, `nucleo/algoritmos/Isomorfismo.h`
2. En `namespace PanelIsomorfismo {}`:
   ```cpp
   inline void dibujar(Interfaz& self, Grafo& red);
   ```
3. Extraer el subpanel de isomorfismo de `dibujarSubpanelIsomorfismo()` (líneas 1007-1143). No es una función separada — es parte de PanelGrafos. La tarea consiste en crear PanelIsomorfismo con la función `dibujar()` que contiene exactamente el mismo código pero como función standalone:
   - `self.grafo_iso_g2`, `self.iso_analizado`, `self.iso_editando_g2`, `self.resultado_iso`
   - `self.registrarLog()`
   - `self.nodo_seleccionado` (usado en línea 1299 de Arbol, NO en isomorfismo directo)
4. En `PanelGrafos.h`: la función `dibujarSubpanelIsomorfismo` se simplifica para delegar a `PanelIsomorfismo::dibujar(self, red)`

**Nota**: `aplicarLayoutArbol` (líneas 1145-1172) se queda en PanelGrafos porque solo la usa Arbol.

**Variables accedidas**: `grafo_iso_g2`, `iso_analizado`, `iso_editando_g2`, `resultado_iso`, `modo_panel` (para verificar Isomorfismo)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Pestañas G1/G2 se muestran
- [ ] "Generar Isomorfo Automático" funciona
- [ ] "Verificar Isomorfismo" muestra resultado correcto

---

## Fase 5 — Lienzo

### T014: Crear lienzo/LienzoRed.h
**Fase**: 5
**Depende de**: T004, T005, T012
**Archivos**: `src/interfaz/lienzo/LienzoRed.h` (crear), `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Muy alto

**Pasos exactos**:
1. Crear `src/interfaz/lienzo/LienzoRed.h` con `#pragma once`, incluye `Interfaz.h`, `GLFW/glfw3.h`, `util/Easing.h`, `util/Animacion.h`, `util/Colores.h`, `paneles/PanelHardware.h`
2. En `namespace LienzoRed {}`:
   ```cpp
   inline void dibujar(Interfaz& self, Grafo& red);
   ```
3. Mover TODO el cuerpo de `dibujarLienzo()` (líneas 1616-2180, ~565 líneas) a esta función, cambiando accesos a `self.`:
   - `self.offset_lienzo` — **se mueve a LienzoRed.h como variable estática o se pasa como referencia**. Decisión de diseño: `offset_lienzo` se declara en el namespace como variable `inline` o se mantiene en Interfaz y se pasa como `ImVec2&`. Se recomienda moverla a LienzoRed como variable interna.
   - `self.nodo_seleccionado`, `self.nodo_hover`, `self.arrastrando`
   - `self.modo_panel`, `self.modo_actual`
   - `self.anim_estado.particula`, `self.anim_estado.activa`, `self.anim_estado.paso_actual`, `self.anim_estado.pasos`
   - `self.anim_estado.visitados`, `self.anim_estado.procesando`
   - `self.anim_estado.exploradas`, `self.anim_estado.confirmadas`, `self.anim_estado.descartadas`
   - `self.anim_estado.tiempo_visita_nodo`
   - `self.grafo_iso_g2`, `self.iso_editando_g2`
   - `self.ruta_optima`, `self.aristas_mst`, `self.mostrar_mst`
   - `self.colores_nodos`, `self.mostrar_coloreo`
   - `self.arbol_props`, `self.arbol_analizado`, `self.arbol_raiz_id`, `self.arbol_layout_aplicado` (para highlight en canvas)
   - `self.simulador`, `self.sim_inicializada`
   - `self.pos_click_derecho`, `self.pendiente_arista_origen`, `self.pendiente_arista_destino`, `self.pendiente_arista_peso`
   - `self.registrarLog()`, `g_sonidos`
4. Reemplazar popup "CrearEquipo" inline por llamada a `PanelHardware::desplegar(self, grafo_actual, pos_click_derecho)` — esperar a T012
5. En `Interfaz.h`:
   - Agregar `#include "lienzo/LienzoRed.h"`
   - Eliminar `dibujarLienzo()` (líneas 1615-2180)
   - Mover miembro `offset_lienzo` (línea 182) o mantenerlo en Interfaz (según decisión)
   - En `dibujar()` línea 305, reemplazar `dibujarLienzo(red)` por `LienzoRed::dibujar(*this, red)`

**Variables accedidas**: ~20 variables de estado (selección, animación, simulación, algoritmo, geometría)
**Variable que se mueve**: `offset_lienzo` → LienzoRed (variable interna del namespace)

**Criterios de aceptación**:
- [ ] Compila sin errores
- [ ] Click selecciona nodos correctamente
- [ ] Drag mueve nodos (individual y Shift+click para mover todo)
- [ ] Scroll hace zoom in/out centrado en cursor
- [ ] Click derecho crea nodo (modo Grafos) o abre popup (modo Redes)
- [ ] Click derecho entre 2 nodos crea arista
- [ ] Delete/Backspace elimina nodo seleccionado
- [ ] Aristas se dibujan con peso, color, dirección
- [ ] Partículas de animación cruzan aristas con timing correcto
- [ ] Paquetes de simulación (HTTP/PING/VIDEO/DDOS) se dibujan
- [ ] Overlays de CPU/RAM en modo Redes se muestran
- [ ] Highlight de árbol (raíz/hojas/niveles) se muestra

---

## Fase 6 — Finalización

### T015: Adelgazar Interfaz.h (orquestador final)
**Fase**: 6
**Depende de**: TODAS (T001-T014)
**Archivos**: `src/interfaz/Interfaz.h` (modificar)
**Riesgo**: Alto

**Pasos exactos**:
1. Verificar que Interfaz.h incluya SOLO:
   - Headers de dominio: `Grafo.h`, `SimuladorRed.h`, `Topologias.h`, `SerializadorJSON.h`, `Sonidos.h`
   - Módulos extraídos: `util/Easing.h`, `util/Animacion.h`, `util/Colores.h`, `componentes/StatusBar.h`, `componentes/MenuPrincipal.h`, `componentes/Dialogos.h`, `paneles/PanelGrafos.h`, `paneles/PanelRed.h`, `paneles/PanelHardware.h`, `paneles/PanelIsomorfismo.h`, `lienzo/LienzoRed.h`
   - ImGui y GLFW
2. Mantener en Interfaz:
   - Enums: `ModoPanel`, `ModoApp` (líneas 34-49)
   - Estado compartido que NO se extrajo: `modo_panel`, `modo_actual`, `nodo_seleccionado`, `nodo_hover`, `arrastrando`, `system_logs`, `dijkstra_*`, `bfs_*`, `dfs_*`, `resultado_*`, `ruta_optima`, `aristas_mst`, `colores_nodos`, `grafo_iso_g2`, `iso_*`, `arbol_*`, `simulador`, `sim_*`, `flujo_*`, `fallo_*`, `pos_click_derecho`, `pendiente_*`, `buffer_nombre`, `fontMono`, `simulacion_jitter`, `jitter_porcentaje`
   - `anim_estado` (miembro de tipo `Animacion::EstadoAnimacion`)
   - Función pública `dibujar(Grafo&, GLFWwindow*)` (líneas 184-309)
   - Funciones que se quedan: `resetGrafoIsomorfismo()` (69-74), `dibujarMatrices()` (2182-2201), `dibujarMatrizAdyacencia()` (2203-2271), `dibujarMatrizIncidencia()` (2273-2335), `dibujarPanelLogs()` (2337-2360), `registrarLog()` (2479-2483), `iconoHardware()` (2485-2494)
3. En `dibujar()`:
   - Simplificar: delegar a módulos en lugar de llamar funciones internas
   - Mantener tick de simulación (líneas 186-189), avance de animación (192-245), jitter (247-252), dockspace (256-298)
   - Reemplazar llamadas a funciones internas por módulos: `MenuPrincipal::dibujar(...)`, `PanelGrafos::dibujarPanelHerramientas(...)`, `PanelRed::dibujar(...)`, `LienzoRed::dibujar(...)`, `StatusBar::dibujar(...)`
4. Contar líneas: `wc -l src/interfaz/Interfaz.h` debe ser ≤ 300

**Criterios de aceptación**:
- [ ] `wc -l src/interfaz/Interfaz.h` ≤ 300
- [ ] Compila sin errores
- [ ] Todos los módulos se incluyen correctamente
- [ ] La app funciona exactamente igual

---

### T016: Agregar tests unitarios (Grafo, Dijkstra, UnionFind)
**Fase**: 6
**Depende de**: —
**Archivos**: Nuevos tests + CMakeLists.txt (modificar)
**Riesgo**: Medio

**Pasos exactos**:
1. Crear directorio `tests/` en la raíz
2. Crear `tests/test_grafo.cpp`:
   - Crear Grafo, agregar 3 nodos, verificar `obtenerNodo(id)` funciona
   - Agregar 2 aristas, verificar conteo
   - Verificar limpiar() funciona
3. Crear `tests/test_dijkstra.cpp`:
   - Crear grafo A-B-C donde A-B (peso 4), B-C (peso 3), A-C (peso 10)
   - Ejecutar Dijkstra A→C, verificar ruta es A→B→C con distancia 7
   - Ejecutar Dijkstra con destino inalcanzable, verificar `hay_ruta == false`
4. Crear `tests/test_unionfind.cpp`:
   - Crear UnionFind con 5 elementos
   - Unir (0,1), (1,2), (3,4)
   - Verificar `find(0) == find(2)` (mismo representante)
   - Verificar `find(2) != find(3)` (distintos representantes)
5. Modificar `CMakeLists.txt`:
   - Agregar `enable_testing()`
   - Agregar `find_package(Catch2)` o usar `add_test` con asserts manuales (sin framework externo)
   - Agregar target de test que compile y ejecute los 3 tests
   - Incluir directorios necesarios: `src/`, `src/nucleo`, etc.
6. Compilar y ejecutar tests

**Nota**: El proyecto no tiene test framework. Se recomienda usar Catch2 (simple, header-only, `#include "catch.hpp"`) o asserts manuales con `add_test()`. Si se usa Catch2, agregar `FetchContent_Declare(catch2 ...)` en CMakeLists.txt.

**Criterios de aceptación**:
- [ ] Tests compilan sin errores
- [ ] Todos los tests pasan
- [ ] Cobertura: crear nodos/aristas, Dijkstra (camino existe/inexiste), UnionFind (unión/búsqueda)
- [ ] `ctest` o ejecución manual reporta éxito

---

### Verificación de Review Workload

| Sub-total | Líneas |
|-----------|--------|
| Fase 1 (Grafo_old.h eliminado) | -542 |
| Fase 1 (Nodo.h modificado) | +2 |
| Fase 2 (3 archivos nuevos + modificaciones) | ~+260 |
| Fase 3 (3 archivos nuevos + modificaciones) | ~+220 |
| Fase 4 (4 archivos nuevos + modificaciones) | ~+1440 |
| Fase 5 (1 archivo nuevo + modificaciones) | ~+600 |
| Fase 6 (Interfaz.h reducido + tests + CMake) | ~+210 (tests) |
| **Total neto** | **~+2190 neto** (~2850 contando eliminaciones) |

La Fase 4 sola (~1440 líneas) excede drásticamente el budget de 400 líneas. La Fase 5 (~600 líneas) también excede el budget. **Se recomiendan PRs encadenados**:
- **PR 1**: Fases 1-3 (preparación + utilidades + componentes) — ~500 líneas netas
- **PR 2**: Fase 4 (paneles grandes) — ~1440 líneas
- **PR 3**: Fases 5-6 (lienzo + finalización + tests) — ~810 líneas netas
