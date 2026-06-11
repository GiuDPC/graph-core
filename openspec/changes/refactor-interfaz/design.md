# Design: Refactor total de Interfaz.h — extracción modular

## Technical Approach

Extracción en 6 fases secuenciales: cada módulo es un archivo header-only con funciones inline como `namespace {nombre}::` que toman `Interfaz& self` como primer parámetro para acceder al estado compartido. `Interfaz.h` mantiene todas las variables de estado global y delega el dibujado llamando a las funciones de los módulos. Sin cambios en comportamiento, firmas públicas, ni layout visual.

## Architecture Decisions

| Decisión | Opciones | Elección | Rationale |
|----------|----------|----------|-----------|
| Acceso al estado compartido | (A) Pasaje de referencias, (B) Miembros públicos de Interfaz, (C) Structs de estado separados | **A: `Interfaz&`** | Mínimo cambio sintáctico, evita re-factorizar el estado ahora. Cada módulo recibe `Interfaz& self` y accede a sus miembros. Preserva comportamiento exacto. |
| Módulos como clases vs. namespaces | Clase con构造函数, Namespace con funciones inline | **Namespace + inline functions** | Header-only, sin .cpp, sin instanciación. Las funciones son `inline` para evitar múltiples definiciones. |
| Orden de includes | Forward-declare + include al final vs todos al inicio | **Includes al inicio** | No hay dependencias circulares reales (ver abajo). Todos los módulos incluyen `Interfaz.h`. `Interfaz.h` incluye todos los módulos. |

## State Ownership Map

| Variable | Dueño | Usada por |
|----------|-------|-----------|
| `modo_actual`, `modo_panel` | Interfaz.h | PanelGrafos, StatusBar, LienzoRed |
| `nodo_seleccionado`, `nodo_hover`, `arrastrando` | Interfaz.h | PanelGrafos, LienzoRed |
| `system_logs` | Interfaz.h | Todos (vía `registrarLog`) |
| `dijkstra_*`, `ruta_optima`, `aristas_mst`, `colores_nodos`, etc. | Interfaz.h | PanelGrafos (subpaneles), LienzoRed |
| `grafo_iso_g2`, `iso_*` | Interfaz.h | PanelGrafos (Isomorfismo), LienzoRed |
| `simulador`, `sim_*`, `flujo_*`, `fallo_*` | Interfaz.h | PanelRed, LienzoRed, StatusBar |
| `pendiente_arista_*`, `pos_click_derecho`, `buffer_nombre` | Interfaz.h | LienzoRed |
| `fontMono` | Interfaz.h | Matrices (queda inline en Interfaz) |
| `pasos_animacion`, `paso_actual`, `timer_paso`, `velocidad_paso` | **Animacion.h** | Animacion, PanelGrafos, LienzoRed, StatusBar |
| `animacion_activa`, `animacion_pausada` | **Animacion.h** | Animacion, PanelGrafos, LienzoRed, StatusBar |
| `nodos_visitados`, `nodos_procesando`, `aristas_*` | **Animacion.h** | Animacion, LienzoRed |
| `Particula particula_activa`, `tiempo_visita_nodo` | **Animacion.h** | Animacion, LienzoRed |
| `offset_lienzo` | **LienzoRed.h** | Solo LienzoRed |

> Nota: Animacion.h expone su estado como referencias (`std::set<int>&`, `Particula&`, etc.) que se inicializan desde Interfaz.h. LienzoRed.h recibe `offset_lienzo` como `ImVec2&` mutable.

## Dependency Graph

```
Interfaz.h (raíz, sin dep circular)
  ├── util/Easing.h          (solo <cmath>)
  ├── util/Animacion.h       (Easing, PasoAnimacion)
  ├── util/Colores.h         (imgui.h, IconsFontAwesome6)
  ├── componentes/StatusBar.h    (Interfaz.h, Sonidos)
  ├── componentes/MenuPrincipal.h (Interfaz.h, pfd, Sonidos)
  ├── componentes/Dialogos.h      (Interfaz.h, pfd)
  ├── paneles/PanelGrafos.h       (Interfaz.h, todos los algoritmos)
  ├── paneles/PanelRed.h          (Interfaz.h, SimuladorRed, Topologias)
  ├── paneles/PanelHardware.h     (Interfaz.h, TipoHardware)
  ├── paneles/PanelIsomorfismo.h  (Interfaz.h, Isomorfismo)
  └── lienzo/LienzoRed.h         (Interfaz.h, GLFW/glfw3.h, Animacion, Easing)
```

## Circular Dependency Analysis

**No hay dependencias circulares.** Todos los módulos dependen de `Interfaz.h` (hacia arriba), pero `Interfaz.h` solo incluye los módulos — nunca hay un ciclo porque los módulos no se incluyen entre sí.

Riesgo potencial: si en el futuro `PanelGrafos.h` necesita incluir `LienzoRed.h` o viceversa. Mitigación: si ocurre, se extrae un struct de estado compartido a un tercer archivo (`SharedCanvasState.h`). Por ahora todo el estado compartido vive en `Interfaz.h`.

## Module Interfaces

### `util/Easing.h`
```cpp
namespace Easing {
    inline float easeInOutCubic(float t);
    inline float easeOutBounce(float t);
    // easeOutBack, easeOutElastic — idénticos al original
}
```
Sin estado. Solo `<cmath>`.

### `util/Animacion.h`
```cpp
struct ParticulaAnimacion { bool activa; ImVec2 pos_inicio, pos_fin; float progreso, duracion; ImU32 color; float radio; };

namespace Animacion {
    struct EstadoAnimacion {
        std::vector<PasoAnimacion> pasos;
        int paso_actual = -1;
        float timer_paso = 0.0f, velocidad_paso = 0.5f;
        bool activa = false, pausada = false;
        std::set<int> visitados, procesando;
        std::set<std::pair<int,int>> exploradas, confirmadas, descartadas;
        ParticulaAnimacion particula;
        std::map<int, float> tiempo_visita_nodo;
    };
    inline void iniciar(EstadoAnimacion& est, std::vector<PasoAnimacion> pasos);
    inline void aplicarPaso(EstadoAnimacion& est, const PasoAnimacion& p, Sonidos& snd);
    inline void reset(EstadoAnimacion& est);
    inline void pausar(EstadoAnimacion& est);
}
```
Depende de `Easing.h`, `PasoAnimacion.h`. NO depende de `Interfaz.h`.

### `util/Colores.h`
```cpp
namespace Colores {
    inline constexpr ImU32 NODO_VISITADO    = IM_COL32(0, 230, 118, 200);
    inline constexpr ImU32 ARISTA_CONFIRMADA = IM_COL32(255, 179, 0, 255);
    // ... todas las constantes de color usadas en la UI
}
```
Depende de `imgui.h` e `IconsFontAwesome6.h`.

### `componentes/StatusBar.h`
```cpp
namespace StatusBar {
    inline void dibujar(Interfaz& self);
}
```
Accede a `self.modo_actual`, `self.animacion_activa`, `self.simulador`, etc.

### `componentes/MenuPrincipal.h`
```cpp
namespace MenuPrincipal {
    inline void dibujar(Interfaz& self, Grafo& red, GLFWwindow* ventana);
}
```

### `componentes/Dialogos.h`
```cpp
namespace Dialogos {
    inline void acercaDe();
    inline void fallbackCargar(Interfaz& self, Grafo& red);
    inline void fallbackGuardar(Interfaz& self, Grafo& red);
}
```

### `paneles/PanelGrafos.h`
```cpp
namespace PanelGrafos {
    inline void dibujar(Interfaz& self, Grafo& red);
    // funciones auxiliares internas al namespace (no llamadas desde Interfaz)
    inline void selectorModo(Interfaz& self, Grafo& red);
    inline void menuGeneral(Interfaz& self, Grafo& red);
    inline void subpanelDijkstra(Interfaz& self, Grafo& red);
    inline void subpanelKruskal(Interfaz& self, Grafo& red);
    inline void subpanelBFS(Interfaz& self, Grafo& red);
    inline void subpanelDFS(Interfaz& self, Grafo& red);
    inline void subpanelCiclos(Interfaz& self, Grafo& red);
    inline void subpanelColoreo(Interfaz& self, Grafo& red);
    inline void subpanelArbol(Interfaz& self, Grafo& red);
    inline void controlesAnimacion(Interfaz& self);
    inline void propiedadesNodo(Interfaz& self, Grafo& red);
}
```
Requiere includes de todos los algoritmos (Dijkstra, Kruskal, BFS, DFS, Ciclos, Coloreo, Arbol).

### `paneles/PanelRed.h`
```cpp
namespace PanelRed {
    inline void dibujar(Interfaz& self, Grafo& red);
}
```

### `paneles/PanelHardware.h`
```cpp
namespace PanelHardware {
    inline void desplegar(Interfaz& self, Grafo& red);
}
```

### `paneles/PanelIsomorfismo.h`
```cpp
namespace PanelIsomorfismo {
    inline void dibujar(Interfaz& self, Grafo& red);
}
```

### `lienzo/LienzoRed.h`
```cpp
namespace LienzoRed {
    inline void dibujar(Interfaz& self, Grafo& red);
    // maneja: input (click, drag, scroll, delete, right-click),
    // render (grid, edges, nodes, particles, packets, overlays),
    // popups (CrearArista, CrearEquipo)
}
```
Requiere `GLFW/glfw3.h`, `Animacion.h`, `Easing.h`.

## Extraction Strategy per Phase

| Fase | Crear | Modificar | Verificar |
|------|-------|-----------|-----------|
| **1. Preparación** | `util/` directorio | `Nodo.h` (forward-declare ImVec2), eliminar `Grafo_old.h` | Compila, grep Grafo_old vacío |
| **2. Utilidades** | `util/Easing.h`, `util/Animacion.h`, `util/Colores.h` | `Interfaz.h` (eliminar easing inline + struct Particula, agregar #includes) | Compila, easing outputs idénticos |
| **3. Componentes** | `componentes/StatusBar.h`, `componentes/MenuPrincipal.h`, `componentes/Dialogos.h` | `Interfaz.h` (reemplazar funciones por includes + llamadas) | Compila, barra/menú/popups iguales |
| **4. Paneles** | `paneles/PanelGrafos.h`, `paneles/PanelRed.h`, `paneles/PanelHardware.h`, `paneles/PanelIsomorfismo.h` | `Interfaz.h` (reemplazar funciones por includes) | Compila, navegación/subpaneles/matrices funcionan |
| **5. Lienzo** | `lienzo/LienzoRed.h` | `Interfaz.h` (mover dibujarLienzo + popups + #include glfw3.h) | Compila, click/drag/zoom/partículas iguales |
| **6. Finalización** | Tests (Grafo, Dijkstra, UnionFind) | `Interfaz.h` (adelgazar ≤300 líneas) | Compila, tests pasan, `wc -l` ≤ 300 |

## File Changes

| File | Acción | Description |
|------|--------|-------------|
| `src/interfaz/Interfaz.h` | Modify | Reducir de ~2500 → ≤300 líneas (orquestador) |
| `src/interfaz/util/Easing.h` | Create | 3 funciones inline de easing |
| `src/interfaz/util/Animacion.h` | Create | Struct ParticulaAnimacion + EstadoAnimacion + 4 funciones |
| `src/interfaz/util/Colores.h` | Create | ~20 constantes ImU32/ImVec4 semánticas |
| `src/interfaz/componentes/StatusBar.h` | Create | Barra inferior con modo, FPS, audio |
| `src/interfaz/componentes/MenuPrincipal.h` | Create | Menú Archivo/Opciones/Acerca de |
| `src/interfaz/componentes/Dialogos.h` | Create | Popups modales (AcercaDe, fallbacks) |
| `src/interfaz/paneles/PanelGrafos.h` | Create | Panel izquierdo con todos los subpaneles |
| `src/interfaz/paneles/PanelRed.h` | Create | Panel de simulación de red |
| `src/interfaz/paneles/PanelHardware.h` | Create | Despliegue de hardware |
| `src/interfaz/paneles/PanelIsomorfismo.h` | Create | Panel dual G1/G2 |
| `src/interfaz/lienzo/LienzoRed.h` | Create | Canvas con render + input + popups |
| `src/nucleo/Grafo_old.h` | Delete | Dead code (543 líneas) |
| `src/nucleo/tipos/Nodo.h` | Modify | Forward-declare ImVec2 |

## Testing Strategy

| Layer | Qué probar | Cómo |
|-------|-----------|------|
| Unit (Fase 6) | `Grafo.h`: crear nodos/aristas, obtener | Test directo de la API pública |
| Unit | `Dijkstra.h`: ruta óptima (existe/inexiste) | Grafo pequeño, verificar camino y distancia |
| Unit | `UnionFind.h`: find/union con 5 elementos | Verificar representantes |
| Compile (cada fase) | Todos los módulos compilan sin errores | `cmake --build build` |
| Visual (cada fase) | UI se renderiza idéntica | Inspección manual post-cada fase |

## Migration / Rollout

No migration required. Cada fase es un commit atómico. Rollback = `git revert <sha>`. Sin cambios en datos ni persistencia.

## Open Questions

- [ ] `Animacion.h`: ¿el estado de animación se pasa como struct independiente o se queda como miembros de Interfaz y se accede vía `self.`? Decidido → struct `EstadoAnimacion` independiente, no depende de Interfaz. LienzoRed.h y StatusBar.h acceden al estado de animación vía `self.anim_estado` (miembro de Interfaz).
- [ ] Verificar que `easeOutBack` y `easeOutElastic` existen en el código original o hay que extraerlas de las implementaciones inline en Interfaz.h. (Confirmado: solo existen `easeInOutCubic` y `easeOutBounce`. Las funciones adicionales de la spec se agregarán como nuevas.)
