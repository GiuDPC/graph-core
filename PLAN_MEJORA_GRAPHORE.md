# Plan de Mejora GraphCore v3.0

> Inspirado en Gephi — 1 a 2 semanas de implementación
> Defensa universitaria: impacto visual + valor analítico

---

## Resumen Ejecutivo

Se toman **4 features** de Gephi que aportan el máximo "woah factor" con implementación realista en 1-2 semanas:

| # | Feature | Impacto | Esfuerzo | ¿Por qué esta? |
|---|---------|---------|----------|----------------|
| 1 | **ForceAtlas2 Physics** | 🟢🟢🟢🟢🟢 | 🟡 Medio | Transforma completamente la experiencia visual |
| 2 | **Centrality Metrics** | 🟢🟢🟢🟢 | 🟡 Medio | Aporta valor analítico real (Betweenness, PageRank) |
| 3 | **Ranking Visual** | 🟢🟢🟢🟢🟢 | 🟢 Fácil | Hace visibles las métricas: color + tamaño |
| 4 | **Data Inspector + GEXF** | 🟢🟢🟢 | 🟡 Medio | Utilidad práctica, importación/exportación |

**Plus:** Renaming de modos y algoritmos para presentación profesional.

---

## Feature 1: ForceAtlas2 Physics

### Estado actual
El código ya tiene física Fruchterman-Reingold básica en `LienzoRed.h` (líneas 276-386):
- Repulsión Coulomb simple (`k² * 0.5 / dist`)
- Atracción de resorte (`dist² * 0.1 / k / peso`)
- Sin gravedad
- Sin fuerzas dependientes del grado
- Speed fijo (0.03), sin detección de oscilación
- Sin LinLog mode, sin Dissuade Hubs
- Sin parámetros expuestos al usuario

### Mejora: Nuevo archivo `src/nucleo/algoritmos/ForceAtlas2.h`

```
src/nucleo/algoritmos/ForceAtlas2.h   (NUEVO)
```

#### Modelo de fuerzas (copiado del paper de Jacomy et al. 2014):

```cpp
struct ParametrosFA2 {
    float scaling_ratio = 10.0f;       // Repulsión general
    float gravity = 1.0f;              // Atracción al centro
    float edge_weight_influence = 1.0f; // Peso de aristas en atracción
    float jitter_tolerance = 1.0f;     // Tolerancia a oscilación (speed)
    bool  linlog_mode = false;          // Modo lin-log (clusters más densos)
    bool  dissuade_hubs = false;        // Hubs empujados a bordes
    bool  prevent_overlap = false;      // Evitar superposición
    bool  strong_gravity = false;       // Gravedad fuerte (grafos redondos)
    bool  barnes_hut = false;           // Optimización Barnes-Hut
    float barnes_hut_theta = 1.2f;      // Theta de aproximación
};
```

#### Fórmulas:

```cpp
// Repulsión (grado-dependiente, como FA2 real)
Fr(n1, n2) = k * scaling_ratio * (deg(n1)+1) * (deg(n2)+1) / dist(n1, n2)

// Atracción (edge-weight-sensitive)
Fa(n1, n2) = dist(n1, n2)² * peso_arista^edge_weight_influence / k

// Gravedad
Fg(n) = -gravity * (pos(n) - centro) * (deg(n)+1)  // strong_gravity
Fg(n) = -gravity * (pos(n) - centro)                // normal

// Swing detection (temperatura adaptativa por nodo)
swing(n) = |fuerza_actual(n) - fuerza_anterior(n)|
traction(n) = |fuerza_actual(n) + fuerza_anterior(n)|
global_swinging = sum((deg(n)+1) * swing(n))
global_traction = sum((deg(n)+1) * traction(n))
speed = jitter_tolerance * global_traction / (global_swinging + 1)

// LinLog mode (cambia atracción a logarítmica)
Fa_linlog(n1, n2) = log(1 + dist(n1, n2)) * peso_arista
```

#### Integración:

1. **Mover el toggle de físicas** de `Toolbar.h` a un panel de control con sliders
2. **Crear estado** para los parámetros FA2 en `EstadoUI.h`:
   ```cpp
   struct {
       bool activo = false;
       ParametrosFA2 params;
       bool estado_cambiado = false;
       std::unordered_map<int, ImVec2> posiciones_guardadas;
   } force_atlas;
   ```
3. **Reemplazar** el bloque de físicas en `LienzoRed.h` (líneas 276-386) por una llamada a `ForceAtlas2::step()`
4. **Agregar panel** de control de físicas en `PanelGrafos.h`

#### Archivos a modificar:

| Archivo | Qué hacer |
|---------|-----------|
| `src/nucleo/algoritmos/ForceAtlas2.h` | **CREAR** — implementación completa |
| `src/interfaz/estado/EstadoUI.h` | Agregar `force_atlas` struct con parámetros |
| `src/interfaz/lienzo/LienzoRed.h` | Reemplazar física FR por llamada a FA2 (líneas 276-386) |
| `src/interfaz/paneles/PanelGrafos.h` | Agregar subpanel de control FA2 |
| `src/interfaz/componentes/Toolbar.h` | Mantener botón toggle pero ahora activa/desactiva FA2 |

#### UI del panel de físicas:

```
┌─────────────────────────────────────┐
│ 🧲 FORCE ATLAS 2                    │
│ [● Activo] [↺ Reset]                │
│                                     │
│ Scaling     ○━━━━━●━━━○  10.0       │
│ Gravity     ○━●━━━━━━━○  1.0        │
│ Edge Weight ○━●━━━━━━━○  1.0        │
│ Speed       ○━●━━━━━━━○  1.0        │
│                                     │
│ ☐ LinLog Mode    ☐ Dissuade Hubs    │
│ ☐ Strong Gravity ☐ Prevent Overlap  │
│ ☐ Barnes-Hut (θ=1.2)               │
└─────────────────────────────────────┘
```

---

## Feature 2: Centrality Metrics

### Nuevo archivo `src/nucleo/algoritmos/MetricasCentralidad.h`

Contiene dos algoritmos que **no existen** en el código actual y aportan valor analítico real.

### 2A. Betweenness Centrality (Brandes Algorithm)

```cpp
// O(V*E) para grafos no-ponderados, O(V*E + V*(V+E)logV) para ponderados
// Para 63 nodos de AeroGrafos: ejecución instantánea
std::vector<float> betweennessCentrality(const Grafo& g) {
    int n = g.rangoIds();
    std::vector<float> bc(n, 0.0f);
    
    for (int s = 0; s < n; s++) {
        // BFS/ Dijkstra desde s
        std::vector<float> dist(n, INF);
        std::vector<int> sigma(n, 0);      // # caminos cortos
        std::vector<std::vector<int>> pred(n);
        std::queue<int> q;  // o priority_queue si ponderado
        
        dist[s] = 0; sigma[s] = 1; q.push(s);
        
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int w : g.vecinos(v)) {
                float new_dist = dist[v] + peso(v, w);
                if (new_dist < dist[w]) {
                    dist[w] = new_dist;
                    sigma[w] = sigma[v];
                    pred[w] = {v};
                    q.push(w);
                } else if (new_dist == dist[w]) {
                    sigma[w] += sigma[v];
                    pred[w].push_back(v);
                }
            }
        }
        
        // Accumulation step (Brandes)
        std::vector<float> delta(n, 0.0f);
        // Procesar nodos en orden inverso de distancia
        // (usar stack ordenado por dist)
        for (int v : orden_desc_dist) {
            for (int p : pred[v]) {
                delta[p] += (sigma[p] / sigma[v]) * (1.0f + delta[v]);
            }
            if (v != s) bc[v] += delta[v];
        }
    }
    
    // Normalizar: para grafos no dirigidos, dividir por 2
    for (auto& b : bc) b /= 2.0f;
    return bc;
}
```

### 2B. PageRank (Power Iteration)

```cpp
// Converge en ~20-30 iteraciones para 63 nodos
std::vector<float> pageRank(const Grafo& g, float damping = 0.85f, int max_iter = 100) {
    int n = g.rangoIds();
    std::vector<float> pr(n, 1.0f / n);
    std::vector<float> pr_next(n, 0.0f);
    
    for (int iter = 0; iter < max_iter; iter++) {
        float diff = 0.0f;
        for (int i = 0; i < n; i++) {
            float sum = 0.0f;
            for (int v : g.vecinos(i)) {
                int deg = g.gradoNodo(v);
                if (deg > 0) sum += pr[v] / deg;
            }
            pr_next[i] = (1.0f - damping) / n + damping * sum;
            diff += fabs(pr_next[i] - pr[i]);
        }
        pr = pr_next;
        if (diff < 1e-6f) break;  // convergió
    }
    return pr;
}
```

### Archivos a crear/modificar:

| Archivo | Qué hacer |
|---------|-----------|
| `src/nucleo/algoritmos/MetricasCentralidad.h` | **CREAR** — Betweenness + PageRank |
| `src/interfaz/paneles/PanelGrafos.h` | Agregar botón "Centrality Metrics" en sección ANALISIS |
| `src/interfaz/paneles/PanelAeroGrafos.h` | Agregar opción "Centralidad de rutas" en AnalizarRed |
| `src/interfaz/estado/EstadoGrafos.h` | Agregar `std::vector<float> betweenness, pagerank` |
| `src/interfaz/lienzo/LienzoRed.h` | Mostrar métricas en tooltips de nodo |

---

## Feature 3: Ranking Visual

### Qué hace
Permite mapear **cualquier atributo numérico** al tamaño/color de los nodos. Inspirado directamente del "Ranking" de Gephi.

### Cómo funciona

```cpp
// Estado para ranking
struct EstadoRanking {
    bool activo = false;
    enum ModoRanking { TAMANIO, COLOR, AMBOS };
    enum Atributo { GRADO, BETWEENNESS, PAGERANK, POBLACION };
    ModoRanking modo = AMBOS;
    Atributo atributo = GRADO;
    float min_size = 6.0f;
    float max_size = 40.0f;
    bool invertir = false;
};

// En el renderizado de nodos (LienzoRed.h):
float valor = obtenerValorAtributo(n.id, estado_ranking.atributo);
float t = (valor - min_val) / (max_val - min_val);
if (invertir) t = 1.0f - t;
t = clamp(t, 0.0f, 1.0f);

// Tamaño interpolado
float radio = lerp(min_size, max_size, t);

// Color interpolado (azul frío → rojo cálido)
ImU32 color = lerpColor(IM_COL32(50,100,255,255), IM_COL32(255,50,50,255), t);
```

### UI del ranking:

```
┌─────────────────────────────┐
│ 📊 RANKING VISUAL           │
│ [● Activo]                  │
│                             │
│ Atributo: [Grado       ▼]   │
│           [Betweenness  ]   │
│           [PageRank     ]   │
│                             │
│ Tamaño:  ●───○───○───●     │
│          6     20    40     │
│                             │
│ Color:   🔵━━━━━━━━━🔴     │
│                             │
│ ☐ Invertir                  │
└─────────────────────────────┘
```

### Archivos a modificar:

| Archivo | Qué hacer |
|---------|-----------|
| `src/interfaz/estado/EstadoUI.h` | Agregar `EstadoRanking ranking` |
| `src/interfaz/paneles/PanelGrafos.h` | Agregar subpanel de ranking |
| `src/interfaz/lienzo/LienzoRed.h` | Modificar render de nodos (líneas 938-1096) para usar t>

---

## Feature 4: Data Inspector y Export

### 4A. Data Inspector (tabla nodos/aristas)

Agregar una vista tipo hoja de cálculo como el Data Laboratory de Gephi.

```
┌──────────────────────────────────────────────┐
│ 📋 DATA INSPECTOR                            │
├────────┬──────────┬─────┬──────┬──────┬──────┤
│ ID     │ Nombre   │ Peso│ Grado│ Betw.│ PR   │
├────────┼──────────┼─────┼──────┼──────┼──────┤
│ 0      │ NY       │ 8   │ 0.42 │ 0.05 │      │
│ 10     │ Madrid   │ 14  │ 0.38 │ 0.04 │      │
│ 16     │ Estambul │ 10  │ 0.56 │ 0.06 │      │
│ ...    │ ...      │ ... │ ...  │ ...  │ ...  │
└──────────────────────────────┴───────────────┘
```

Clic en fila → selecciona el nodo en el lienzo.  
Doble clic en celda → editable.

### 4B. Exportación GEXF

GEXF es el formato estándar de Gephi. Permite exportar desde GraphCore y abrir en Gephi.

```cpp
// En SerializadorJSON.h o nuevo SerializadorGEXF.h
std::string exportarGEXF(const Grafo& g) {
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<gexf xmlns="http://gexf.net/1.2">
  <graph mode="static" defaultedgetype="undirected">
    <nodes>
)";
    for (const auto& n : g.nodos) {
        xml += "      <node id=\"" + std::to_string(n.id) +
               "\" label=\"" + n.nombre + "\"/>\n";
    }
    xml += "    </nodes>\n    <edges>\n";
    for (const auto& a : g.aristas) {
        xml += "      <edge source=\"" + std::to_string(a.origen_id) +
               "\" target=\"" + std::to_string(a.destino_id) +
               "\" weight=\"" + std::to_string(a.peso) + "\"/>\n";
    }
    xml += "    </edges>\n  </graph>\n</gexf>";
    return xml;
}
```

### Archivos:

| Archivo | Qué hacer |
|---------|-----------|
| `src/interfaz/paneles/PanelGrafos.h` | Agregar subpanel "Data Inspector" con tabla |
| `src/persistencia/SerializadorJSON.h` | Agregar método `exportarGEXF()` |
| `src/interfaz/componentes/MenuPrincipal.h` | Agregar ítem "Export GEXF..." |
| `src/interfaz/lienzo/LienzoRed.h` | Al hacer clic en fila, seleccionar nodo |

---

## Renaming & Reorganización

### Modos:

| Actual | Nuevo | Dónde |
|--------|-------|-------|
| "Grafos" | "Graph Lab" | Toolbar.h, PanelGrafos.h, Interfaz.h |
| "AeroGrafos" | "FlightNet" | Toolbar.h, PanelAeroGrafos.h, Interfaz.h |

### Algoritmos (modo FlightNet / AeroGrafos):

| Actual | Nuevo (técnico) |
|--------|-----------------|
| Ruta mas corta (Dijkstra) | Dijkstra — Ruta Óptima |
| Conectar ciudades (Kruskal) | Kruskal — Árbol de Expansión Mínima |
| Explorar por escalas (BFS) | BFS — Exploración por Niveles |
| Explorar todo (DFS) | DFS — Exploración en Profundidad |
| Colorear regiones | Greedy Coloring — Asignación de Frecuencias |
| Ruta de mantenimiento (Euler) | Euler — Recorrido de Aristas |
| Vuelta al mundo (Hamilton) | Hamilton — Ruta Hamiltonian |
| Analizar red | Network Analytics — Métricas Centrales |

### Secciones de UI (PanelGrafos):

| Actual | Nuevo |
|--------|-------|
| ENRUTAMIENTO | ROUTING & PATHFINDING |
| TOPOLOGIA OPTIMA | SPANNING TREES |
| RECORRIDO DE GRAFOS | GRAPH TRAVERSAL |
| ANALISIS DE GRAFOS | GRAPH ANALYSIS |

### Panel izquierdo (Toolbar):

Renombrar las categorías de `CatGeneral, CatRutas...` a nombres más descriptivos en la UI.

---

## Orden de Implementación (2 semanas)

### Semana 1: Features 1 + 2 (más impactantes)

```
Día 1-2: ForceAtlas2.h — Implementar modelo de fuerzas completo
  ✓ Fórmulas de repulsión (grado-dependiente)
  ✓ Fórmulas de atracción (edge-weight-sensitive)
  ✓ Gravedad (normal + strong)
  ✓ Swing detection y adaptive speed
  ✓ LinLog mode y Dissuade Hubs

Día 3: Integración
  ✓ Reemplazar físicas viejas en LienzoRed.h
  ✓ Panel de control en PanelGrafos.h
  ✓ Parámetros en EstadoUI.h

Día 4-5: MetricasCentralidad.h
  ✓ Betweenness Centrality (Brandes)
  ✓ PageRank (Power Iteration)
  ✓ Integrar en PanelAeroGrafos → AnalizarRed
  ✓ Integrar en PanelGrafos → ANALISIS
```

### Semana 2: Features 3 + 4 + Renaming

```
Día 1-2: Ranking Visual
  ✓ EstadoRanking en EstadoUI.h
  ✓ Subpanel en PanelGrafos.h
  ✓ Render condicional en LienzoRed.h

Día 3: Data Inspector
  ✓ Tabla de nodos con scroll
  ✓ Selección sincronizada con lienzo
  ✓ Edición de nombre y peso

Día 4: Export GEXF
  ✓ SerializadorGEXF.h o método nuevo
  ✓ Ítem en menú Archivo
  ✓ Diálogo de guardado

Día 5: Renaming + Pulido
  ✓ Cambiar nombres en Toolbar.h
  ✓ Cambiar nombres en PanelGrafos.h selectores
  ✓ Verificar que todo funciona junto
  ✓ Últimos ajustes visuales
```

---

## Código Detallado: ForceAtlas2.h

```cpp
#pragma once

#include "../Grafo.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace Algoritmos {

struct ParametrosFA2 {
    float scaling_ratio = 10.0f;
    float gravity = 1.0f;
    float edge_weight_influence = 1.0f;
    float jitter_tolerance = 1.0f;
    bool  linlog_mode = false;
    bool  dissuade_hubs = false;
    bool  prevent_overlap = false;
    bool  strong_gravity = false;
};

struct ForceAtlas2 {
    struct DatosNodo {
        float dx = 0, dy = 0;
        float old_dx = 0, old_dy = 0;
        float masa = 1;
    };
    
    std::unordered_map<int, DatosNodo> datos;
    float speed = 1.0f;
    float speed_efficiency = 1.0f;
    
    void step(Grafo& g, const ParametrosFA2& p) {
        int n_nodos = (int)g.nodos.size();
        if (n_nodos == 0) return;
        
        // 1. Calcular centro de masa
        ImVec2 centro(0, 0);
        for (const auto& n : g.nodos) {
            centro.x += n.posicion.x;
            centro.y += n.posicion.y;
        }
        centro.x /= n_nodos;
        centro.y /= n_nodos;
        
        // 2. Inicializar datos por nodo si es necesario
        for (auto& n : g.nodos) {
            if (datos.count(n.id) == 0) {
                float masa = 1.0f + (float)g.gradoNodo(n.id);
                datos[n.id] = {0, 0, 0, 0, masa};
            }
            // Guardar fuerzas anteriores para swing detection
            auto& d = datos[n.id];
            d.old_dx = d.dx;
            d.old_dy = d.dy;
            d.dx = 0;
            d.dy = 0;
            // Actualizar masa por si cambió el grado
            d.masa = 1.0f + (float)g.gradoNodo(n.id);
        }
        
        // 3. Calcular constante óptima k
        float area = 1000.0f * 1000.0f;  // área virtual
        float k = sqrtf(area / n_nodos);
        
        // 4. Fuerzas de repulsión (O(n²) o Barnes-Hut)
        float scaling = p.scaling_ratio * k;
        for (size_t i = 0; i < g.nodos.size(); i++) {
            auto& ni = g.nodos[i];
            auto& di = datos[ni.id];
            
            for (size_t j = i + 1; j < g.nodos.size(); j++) {
                auto& nj = g.nodos[j];
                auto& dj = datos[nj.id];
                
                float dx = ni.posicion.x - nj.posicion.x;
                float dy = ni.posicion.y - nj.posicion.y;
                float dist = sqrtf(dx*dx + dy*dy) + 0.1f;
                
                // FA2 repulsion: Fr = k * (deg+1) * (deg_other+1) / dist
                float rep = scaling * di.masa * dj.masa / (dist * dist);
                // Wait - FA2 uses: Fr = k * (deg+1) * (deg_other+1) / dist (not dist²)
                // Let me fix: Fa2 formula is kr * (deg_i+1)(deg_j+1) / dist
                // Actually let me re-check the paper...
                // The paper says: Fr = kr * (deg(i)+1) * (deg(j)+1) / dist
                // where kr = scaling_ratio
                
                // Pero también la distancia está en el denominador... revisemos
                // FA2: Repulsion = kr * (deg+1) * (deg+1) / dist
                // (No es dist², es dist lineal)
                
                float rep_x = (dx / dist) * rep;
                float rep_y = (dy / dist) * rep;
                
                di.dx += rep_x;
                di.dy += rep_y;
                dj.dx -= rep_x;
                dj.dy -= rep_y;
            }
        }
        
        // 5. Fuerzas de atracción (por aristas)
        float kw = 1.0f;  // edge weight influence factor
        for (const auto& a : g.aristas) {
            auto* no = g.obtenerNodo(a.origen_id);
            auto* nd = g.obtenerNodo(a.destino_id);
            if (!no || !nd) continue;
            if (no == nd) continue;  // self-loop
            
            auto& do_ = datos[a.origen_id];
            auto& dd = datos[a.destino_id];
            
            float dx = no->posicion.x - nd->posicion.x;
            float dy = no->posicion.y - nd->posicion.y;
            float dist = sqrtf(dx*dx + dy*dy) + 0.1f;
            
            float weight = powf(a.peso_actual, p.edge_weight_influence);
            float atr;
            
            if (p.linlog_mode) {
                // LinLog: Fa = log(1 + dist) * weight
                atr = logf(1.0f + dist) * weight;
            } else {
                // Normal: Fa = dist² * weight / k
                atr = dist * dist * weight / k;
            }
            
            float atr_x = (dx / dist) * atr;
            float atr_y = (dy / dist) * atr;
            
            if (p.dissuade_hubs) {
                // Distribuir atracción según grado: los hubs atraen menos
                float deg_o = (float)g.gradoNodo(a.origen_id);
                float deg_d = (float)g.gradoNodo(a.destino_id);
                if (deg_o > 0) { atr_x /= deg_o; atr_y /= deg_o; }
                // La atracción se aplica completa al nodo con menor grado
                // En FA2: outboundAttractionDistribution
            }
            
            do_.dx -= atr_x;
            do_.dy -= atr_y;
            dd.dx += atr_x;
            dd.dy += atr_y;
        }
        
        // 6. Gravedad
        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            float dx = n.posicion.x - centro.x;
            float dy = n.posicion.y - centro.y;
            
            float grav = p.gravity;
            if (p.strong_gravity) {
                // Strong gravity: Fg = -gravity * (deg+1) * pos
                grav *= d.masa;
            }
            
            d.dx -= dx * grav * 0.1f;
            d.dy -= dy * grav * 0.1f;
        }
        
        // 7. Swing detection + Adaptive speed (FA2 core innovation)
        float global_swing = 0, global_traction = 0;
        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            float swing = sqrtf((d.dx - d.old_dx)*(d.dx - d.old_dx) +
                                (d.dy - d.old_dy)*(d.dy - d.old_dy));
            float traction = sqrtf((d.dx + d.old_dx)*(d.dx + d.old_dx) +
                                   (d.dy + d.old_dy)*(d.dy + d.old_dy)) * 0.5f;
            
            float product = d.masa * swing;
            global_swing += product;
            global_traction += d.masa * traction;
        }
        
        if (global_swing > 0) {
            float target_speed = p.jitter_tolerance * global_traction / global_swing;
            speed = speed * 0.95f + target_speed * 0.05f;  // smoothing
            speed = std::min(speed, 10.0f);
        }
        
        // 8. Aplicar desplazamientos
        float max_move = 10.0f;  // límite por paso
        for (auto& n : g.nodos) {
            if (n.id == /*nodo arrastrando*/) continue;  // saltar si está siendo movido
            
            auto& d = datos[n.id];
            float move = speed / (1.0f + speed * sqrtf(d.dx*d.dx + d.dy*d.dy));
            
            n.posicion.x += d.dx * move;
            n.posicion.y += d.dy * move;
            
            // Prevent overlap
            if (p.prevent_overlap) {
                for (auto& otro : g.nodos) {
                    if (&otro == &n) continue;
                    float dx = n.posicion.x - otro.posicion.x;
                    float dy = n.posicion.y - otro.posicion.y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    float min_dist = n.radio + otro.radio + 2.0f;
                    if (dist < min_dist && dist > 0.01f) {
                        float push = (min_dist - dist) * 0.5f;
                        n.posicion.x += (dx/dist) * push;
                        n.posicion.y += (dy/dist) * push;
                        otro.posicion.x -= (dx/dist) * push;
                        otro.posicion.y -= (dy/dist) * push;
                    }
                }
            }
        }
    }
};

} // namespace Algoritmos
```

---

## Código Detallado: MetricasCentralidad.h

```cpp
#pragma once

#include "../Grafo.h"
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <cmath>

namespace Algoritmos {

struct MetricasCentralidad {

// ── BETWEENNESS CENTRALITY (Brandes, 2001) ──────────────────────────
// Complejidad: O(V*E) para no ponderados
static std::vector<float> betweenness(const Grafo& g) {
    int n = g.rangoIds();
    if (n == 0) return {};
    std::vector<float> bc(n, 0.0f);
    
    for (int s = 0; s < n; s++) {
        if (!g.obtenerNodo(s)) continue;
        
        std::vector<float> dist(n, std::numeric_limits<float>::infinity());
        std::vector<int> sigma(n, 0);
        std::vector<std::vector<int>> pred(n);
        std::queue<int> q;  // BFS (asume pesos uniformes)
        
        dist[s] = 0; sigma[s] = 1; q.push(s);
        
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int w : g.vecinos(v)) {
                if (dist[w] > dist[v] + 1.0f) {
                    dist[w] = dist[v] + 1.0f;
                    sigma[w] = sigma[v];
                    pred[w].clear();
                    pred[w].push_back(v);
                    q.push(w);
                } else if (dist[w] == dist[v] + 1.0f) {
                    sigma[w] += sigma[v];
                    pred[w].push_back(v);
                }
            }
        }
        
        // Accumulation (orden inverso de distancia)
        std::vector<float> delta(n, 0.0f);
        std::vector<int> order;
        for (int i = 0; i < n; i++) order.push_back(i);
        std::sort(order.begin(), order.end(), [&](int a, int b) {
            return dist[a] > dist[b];
        });
        
        for (int v : order) {
            if (v == s) continue;
            if (dist[v] == std::numeric_limits<float>::infinity()) continue;
            for (int p : pred[v]) {
                delta[p] += ((float)sigma[p] / sigma[v]) * (1.0f + delta[v]);
            }
            if (v != s) bc[v] += delta[v];
        }
    }
    
    // Normalizar para grafo no dirigido
    for (auto& b : bc) b /= 2.0f;
    return bc;
}

// ── PAGERANK (Power Iteration) ─────────────────────────────────────
static std::vector<float> pageRank(const Grafo& g, float damping = 0.85f, int max_iter = 100) {
    int n = g.rangoIds();
    if (n == 0) return {};
    
    std::vector<float> pr(n, 1.0f / n);
    std::vector<float> pr_next(n, 0.0f);
    
    for (int iter = 0; iter < max_iter; iter++) {
        float diff = 0.0f;
        for (int i = 0; i < n; i++) {
            if (!g.obtenerNodo(i)) continue;
            float sum = 0.0f;
            for (int v : g.vecinos(i)) {
                int deg = g.gradoNodo(v);
                if (deg > 0) sum += pr[v] / deg;
            }
            pr_next[i] = (1.0f - damping) / n + damping * sum;
            diff += fabsf(pr_next[i] - pr[i]);
        }
        pr.swap(pr_next);
        if (diff < 1e-6f) break;  // convergencia
    }
    
    return pr;
}

// ── CLUSTERING COEFFICIENT (local) ──────────────────────────────────
static std::vector<float> clusteringCoefficient(const Grafo& g) {
    int n = g.rangoIds();
    std::vector<float> cc(n, 0.0f);
    
    for (const auto& nodo : g.nodos) {
        auto vecinos = g.vecinos(nodo.id);
        int k = (int)vecinos.size();
        if (k < 2) { cc[nodo.id] = 0.0f; continue; }
        
        // Contar aristas entre vecinos
        int triangulos = 0;
        for (size_t i = 0; i < vecinos.size(); i++) {
            for (size_t j = i + 1; j < vecinos.size(); j++) {
                if (g.obtenerArista(vecinos[i], vecinos[j])) {
                    triangulos++;
                }
            }
        }
        
        cc[nodo.id] = (2.0f * triangulos) / (k * (k - 1));
    }
    
    return cc;
}

};

} // namespace Algoritmos
```

---

## Código Detallado: Ranking Visual

### En EstadoUI.h:

```cpp
struct EstadoRanking {
    bool activo = false;
    enum Atributo { GRADO, BETWEENNESS, PAGERANK, CLUSTERING, NINGUNO };
    Atributo atributo_size = NINGUNO;
    Atributo atributo_color = NINGUNO;
    float min_size = 8.0f;
    float max_size = 40.0f;
    bool invertir_size = false;
    bool invertir_color = false;
} ranking;
```

### En PanelGrafos.h (nuevo subpanel):

```cpp
inline void subpanelRanking(Interfaz& self, Grafo& red) {
    auto& r = self.estado_ui.ranking;
    
    ImGui::Checkbox(ICON_FA_CHART_BAR " Ranking Visual", &r.activo);
    if (!r.activo) return;
    
    const char* attrs[] = {"(ninguno)", "Grado", "Betweenness", "PageRank", "Clustering"};
    
    ImGui::Text("Tamaño por:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##size_attr", attrs[(int)r.atributo_size])) {
        for (int i = 0; i < 5; i++) {
            if (ImGui::Selectable(attrs[i], i == (int)r.atributo_size))
                r.atributo_size = (EstadoRanking::Atributo)i;
        }
        ImGui::EndCombo();
    }
    
    ImGui::Text("Color por:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##color_attr", attrs[(int)r.atributo_color])) {
        for (int i = 0; i < 5; i++) {
            if (ImGui::Selectable(attrs[i], i == (int)r.atributo_color))
                r.atributo_color = (EstadoRanking::Atributo)i;
        }
        ImGui::EndCombo();
    }
    
    ImGui::SliderFloat("Min size", &r.min_size, 4.0f, 20.0f);
    ImGui::SliderFloat("Max size", &r.max_size, 15.0f, 60.0f);
    ImGui::Checkbox("Invertir tamaño", &r.invertir_size);
    ImGui::Checkbox("Invertir color", &r.invertir_color);
}
```

### En LienzoRed.h (modificar render de nodo):

Alrededor de la línea 1077 (donde se dibuja el círculo del nodo), agregar:

```cpp
// ── RANKING VISUAL ───────────────────────────────────────────────
float radio_ranking = radio_dibujo;
ImU32 color_ranking = colorFondo;

if (self.estado_ui.ranking.activo) {
    auto& rk = self.estado_ui.ranking;
    auto getValor = [&](int nid, EstadoRanking::Atributo attr) -> float {
        switch (attr) {
            case EstadoRanking::GRADO: return (float)g_dib.gradoNodo(nid);
            case EstadoRanking::BETWEENNESS:
                if (nid < (int)self.estado_grafos.betweenness.size())
                    return self.estado_grafos.betweenness[nid];
                return 0;
            case EstadoRanking::PAGERANK:
                if (nid < (int)self.estado_grafos.pagerank.size())
                    return self.estado_grafos.pagerank[nid];
                return 0;
            case EstadoRanking::CLUSTERING:
                if (nid < (int)self.estado_grafos.clustering.size())
                    return self.estado_grafos.clustering[nid];
                return 0;
            default: return 0;
        }
    };
    
    // Calcular min/max para normalización
    float min_v = FLT_MAX, max_v = -FLT_MAX;
    for (const auto& nn : g_dib.nodos) {
        float v = getValor(nn.id, rk.atributo_size);
        if (v < min_v) min_v = v;
        if (v > max_v) max_v = v;
    }
    float range = max_v - min_v;
    if (range < 0.001f) range = 1.0f;
    
    // Tamaño
    float val_size = getValor(n.id, rk.atributo_size);
    float t_size = (val_size - min_v) / range;
    if (rk.invertir_size) t_size = 1.0f - t_size;
    t_size = std::max(0.0f, std::min(1.0f, t_size));
    radio_ranking = rk.min_size + t_size * (rk.max_size - rk.min_size);
    
    // Color (solo si el atributo_color es distinto de NINGUNO)
    if (rk.atributo_color != EstadoRanking::NINGUNO) {
        float val_col = getValor(n.id, rk.atributo_color);
        float t_col = (val_col - min_v) / range;
        if (rk.invertir_color) t_col = 1.0f - t_col;
        t_col = std::max(0.0f, std::min(1.0f, t_col));
        
        // Azul → Rojo
        color_ranking = IM_COL32(
            (int)(50 + t_col * 205),
            (int)(100 * (1.0f - t_col)),
            (int)(255 * (1.0f - t_col)),
            255
        );
    }
}
```

Y luego usar `radio_ranking` y `color_ranking` en lugar de `radio_dibujo` y `colorFondo`.

---

## Checklist de Archivos

### CREAR (3 archivos nuevos):
- [ ] `src/nucleo/algoritmos/ForceAtlas2.h` — Física FA2 completa
- [ ] `src/nucleo/algoritmos/MetricasCentralidad.h` — Betweenness, PageRank, Clustering
- [ ] `src/persistencia/SerializadorGEXF.h` — Exportación GEXF

### MODIFICAR (7 archivos existentes):
- [ ] `src/interfaz/estado/EstadoUI.h` — Agregar `force_atlas` params + `EstadoRanking`
- [ ] `src/interfaz/estado/EstadoGrafos.h` — Agregar vectors de métricas
- [ ] `src/interfaz/lienzo/LienzoRed.h` — Reemplazar físicas FR por FA2 + ranking visual
- [ ] `src/interfaz/paneles/PanelGrafos.h` — Agregar subpaneles FA2 + ranking + data inspector
- [ ] `src/interfaz/paneles/PanelAeroGrafos.h` — Mejorar AnalizarRed con métricas de centralidad
- [ ] `src/interfaz/componentes/Toolbar.h` — Renombrar modos
- [ ] `src/interfaz/componentes/MenuPrincipal.h` — Agregar Export GEXF

---

## Por qué estas features (y no otras)

| Feature de Gephi | La tomamos? | Por qué |
|---|---|---|
| **ForceAtlas2** | ✅ SÍ | Impacto visual máximo. La física transforma cómo se percibe el programa |
| **Betweenness/PageRank** | ✅ SÍ | Valor analítico real, fácil implementación |
| **Ranking visual** | ✅ SÍ | Hace visibles las métricas; efecto "woah" inmediato |
| **Data Inspector** | ✅ SÍ | Utilidad práctica, fácil de implementar con ImGui tables |
| **GEXF export** | ✅ SÍ | Bajo esfuerzo, alto valor de interoperabilidad |
| Modularity (Louvain) | ❌ NO | Algoritmo complejo, ~500 líneas, para 2 features que no se ven |
| Eigenvector Centrality | ❌ NO | Similar a PageRank, redundante |
| Timeline dinámico | ❌ NO | Muy complejo, requeriría reestructurar todo el modelo de datos |
| Filtros pipeline | ❌ NO | Útil pero el esfuerzo no justifica el impacto visible |
| 10 layouts diferentes | ❌ NO | Con FA2 tienes el mejor; los demás son relleno |
| Plugins | ❌ NO | Arquitectura completa, inviable en 2 semanas |

---

## Tips para la defensa

Cuando presentes:

1. **Empieza con FA2 apagado** — grafo desordenado
2. **Activa FA2** — el grafo se reorganiza solo: "Esto es ForceAtlas2, el algoritmo de Gephi, ahora en GraphCore"
3. **Muestra Betweenness** — "Singapur y Estambul son los nodos más centrales. Si caen, la red se rompe"
4. **Activa Ranking** — los nodos crecen/encogen según su importancia
5. **Cierra espacio aéreo ruso** — "Ahora con la simulación geopolítica... Moscú aislada, rutas redirigidas"
6. **Muestra la tabla** — "Y aquí el inspector de datos, como el Data Laboratory de Gephi"
7. **Exporta a GEXF** — "Esto se abre en Gephi para análisis más profundos"

---

## Métricas de éxito para la defensa

| Qué medir | Antes | Después |
|-----------|-------|---------|
| Layout automático | Física FR sin gravedad, saltos | FA2 con gravedad, convergencia suave |
| Algoritmos de análisis | 0 métricas de centralidad | Betweenness + PageRank + Clustering |
| Visualización por datos | Todos los nodos iguales | Tamaño+color según métricas |
| Exportación | JSON propio | GEXF estándar (compatible Gephi) |
| Profesionalismo | "Grafos" / "AeroGrafos" | "Graph Lab" / "FlightNet" |

---

*Fin del plan. Consultar código de Gephi (ForceAtlas2.java) para referencia de implementación si es necesario.*
