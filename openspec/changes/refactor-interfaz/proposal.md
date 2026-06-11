# Proposal: Refactor total de Interfaz.h — extracción modular

## Intent

`Interfaz.h` (~2500 líneas) concentra TODO el UI del proyecto: menús, paneles, canvas, animaciones, partículas, popups, easing. Esto genera alta carga cognitiva, imposibilita testing unitario, mezcla input handling con drawing, y hace que cualquier cambio sea riesgoso. La refactorización extrae módulos cohesionados sin cambiar lógica de negocio ni firmas públicas.

## Scope

### In Scope
- Extraer funciones de easing, animación y color a `util/`
- Extraer StatusBar, menú principal y diálogos a `componentes/`
- Extraer paneles de grafos, red, hardware e isomorfismo a `paneles/`
- Extraer canvas/lienzo a `lienzo/LienzoRed.h`
- `Interfaz.h` reducido a orquestador delgado (~200-300 líneas)
- Eliminar `src/nucleo/Grafo_old.h` (dead code, 543 líneas)
- Forward-declare `ImVec2` en `Nodo.h` en vez de incluir `imgui.h`
- Agregar tests unitarios para `Grafo.h`, Dijkstra, UnionFind

### Out of Scope
- Cambiar lógica de negocio, algoritmos o comportamiento visible
- Agregar nuevas features de UI
- Refactorizar `SimuladorRed.h`, `Sonidos.h` o `SerializadorJSON.h`
- Cambios en CMakeLists.txt (es header-only, no requiere re-link)
- Modernización a C++20 o cambio de estilo de código

## Capabilities

Refactor puro — sin cambios en comportamiento a nivel spec.

### New Capabilities
None.

### Modified Capabilities
None.

## Approach

6 fases secuenciales, cada una compila y funciona antes de pasar a la siguiente:

| Fase | Riesgo | Qué se hace |
|------|--------|-------------|
| 1. Preparación | 0 | Eliminar `Grafo_old.h`, forward-declare `ImVec2` en `Nodo.h`, crear estructura `util/` |
| 2. Utilidades | Bajo | Extraer `util/Easing.h`, `util/Animacion.h`, `util/Colores.h` |
| 3. Componentes UI | Medio | Extraer `componentes/StatusBar.h`, `MenuPrincipal.h`, `Dialogos.h` |
| 4. Paneles | Alto | Extraer `paneles/PanelGrafos.h`, `PanelRed.h`, `PanelHardware.h`, `PanelIsomorfismo.h` |
| 5. Lienzo | Muy alto | Extraer `lienzo/LienzoRed.h` — canvas con input + render |
| 6. Finalización | Bajo | Adelgazar `Interfaz.h` (orquestador), agregar tests unitarios |

Cada archivo extraído usa `#pragma once`, preserva firmas, y se incluye desde `Interfaz.h` en lugar del código inline original.

## Affected Areas

| Area | Impact | Description |
|------|--------|-------------|
| `src/interfaz/Interfaz.h` | Modified | Se reduce de ~2500 → ~250 líneas (orquestador) |
| `src/interfaz/util/` | New | Easing, Animacion, Colores |
| `src/interfaz/componentes/` | New | StatusBar, MenuPrincipal, Dialogos |
| `src/interfaz/paneles/` | New | PanelGrafos, PanelRed, PanelHardware, PanelIsomorfismo |
| `src/interfaz/lienzo/` | New | LienzoRed |
| `src/nucleo/Grafo_old.h` | Removed | Dead code, 543 líneas |
| `src/nucleo/tipos/Nodo.h` | Modified | Forward-declare ImVec2 |
| `src/nucleo/Grafo.h` | Tested | Agregar tests unitarios |
| `src/nucleo/algoritmos/Dijkstra.h` | Tested | Agregar tests unitarios |
| `src/nucleo/UnionFind.h` | Tested | Agregar tests unitarios |

## Risks

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Fase 4-5: acoplamiento oculto entre paneles y el canvas (variables compartidas en Interfaz.h) | High | Identificar todas las dependencias antes de mover. Extraer estado compartido a structs separados |
| Fase 5: input handling mezclado con drawing — fácil romper interacción | High | Preservar orden exacto de operaciones. Probar manualmente click, drag, zoom tras cada cambio |
| Fase 2-3: funciones inline referenciadas desde múltiples lugares | Med | Usar `inline` en los nuevos headers. Compilar después de cada extracción |
| Regresiones silenciosas (UI compila pero dibuja mal) | Med | Inspección visual post cada fase. Sin tests de UI existentes, verify es manual |
| Riesgo de merge si otro equipo toca `Interfaz.h` simultáneamente | Bajo | Comunicar bloqueo del archivo durante la refactorización |

## Rollback Plan

Cada fase es un commit atómico. Rollback = `git revert <sha>` de esa fase. Como no hay cambios de lógica, revertir no pierde funcionalidad. Si una fase no compila, no se avanza — se corrige in-place.

## Dependencies

- Compilador C++17 con soporte completo (GCC 16.1.1 / MinGW)
- GLFW 3.4, ImGui docking branch (solo para compilación)
- Ninguna externa nueva

## Success Criteria

- [ ] `Interfaz.h` reducido de ~2500 a ≤300 líneas
- [ ] Cada nuevo módulo compila individualmente (`#pragma once` + includes)
- [ ] La app compila y ejecuta sin errores post cada fase
- [ ] Click, drag, zoom, popups, animaciones funcionan idéntico a pre-refactor
- [ ] `Grafo_old.h` eliminado del árbol
- [ ] `Nodo.h` ya no incluye `imgui.h` (usa forward declaration)
- [ ] Tests unitarios pasan para Grafo, Dijkstra, UnionFind
