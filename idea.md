# IDEA — Mejoras para el Simulador de Red (Modo Red)

> Análisis completo del modo red de GraphCore + plan de implementación por fases.
> Estilo **ponytail**: mínimo código, máximo impacto. Cada fase es autónoma y no rompe nada existente.

---

## ANÁLISIS COMPLETO DEL MODO RED

### Cómo funciona todo (el flujo completo)

```
main.cpp loop
  └─ Interfaz::dibujar()
      ├── SimuladorRed::tick(red, dt)       ← Motor de simulación
      │    ├── actualizarNodos()             ← CPU/RAM/uptime
      │    ├── actualizarAristas()           ← Decaimiento de uso, jitter, pérdida
      │    ├── procesarColas()              ← Genera paquetes, los mueve por ruta Dijkstra
      │    ├── colectarEstadisticas()        ← Sparklines cada 0.5s
      │    └── generarEventos()             ← Spikes aleatorios, micro-cortes
      │
      ├── LienzoRed::dibujar(red, self)     ← Render del canvas (1203 líneas)
      │    ├── Grid, pan (con inercia), zoom, físicas Fruchterman-Reingold
      │    ├── Aristas: color según uso, ondas viajeras, X si caídas, flechas
      │    ├── Paquetes: círculo + estela (4 trailing) + halo + etiqueta
      │    ├── Nodos: color por tipo, anillo de salud, hover tooltip, X si caído
      │    └── Overlays: zoom flotante, resultados de algoritmos
      │
      └── PanelRed::dibujar(self, red)       ← UI de control (500 líneas)
           ├── Topologías predefinidas, condiciones (jitter)
           ├── Iniciar/pausar/reanudar/velocidad, presets rápidos
           ├── Envío manual de tráfico (origen, destino, tipo, duración)
           ├── Fallos (nodo seleccionado + combo de aristas)
           ├── Tabla de ruteo, estado de nodos (CPU/RAM/RX/TX)
           ├── Timeline gráfico, log de eventos
           ├── Inspector de paquetes por click
           └── Sparklines de throughput y packet loss
```

### Componentes clave del motor (`SimuladorRed.h`, 568 líneas)

| Componente               | Líneas  | Rol                                                           |
| ------------------------ | ------- | ------------------------------------------------------------- |
| `inicializar()`          | 47-75   | Prepara estado desde la topología del grafo                   |
| `tick()`                 | 77-96   | Loop principal: dt → dt_sim por velocidad                     |
| `actualizarNodos()`      | 358-395 | CPU/RAM fluctúan, alertas si >80% CPU                         |
| `actualizarAristas()`    | 397-412 | Uso decae (0.9^dt), jitter y pérdida suben con congestión     |
| `procesarColas()`        | 416-514 | Crea paquetes cada 0.1s, los mueve salto a salto, acumula uso |
| `colectarEstadisticas()` | 322-356 | Sparklines de throughput, pérdida, CPU, RAM                   |
| `generarEventos()`       | 517-561 | Spikes aleatorios y micro-cortes auto-restaurados             |

### Lo que falta actualmente

| Aspecto                 | Estado actual                | Problema                                 |
| ----------------------- | ---------------------------- | ---------------------------------------- |
| **Feedback visual**     | Solo log de texto + timeline | El usuario no entiende qué pasa sin leer |
| **TCP handshake**       | No existe                    | Datos viajan directo, irreal             |
| **QoS/prioridad**       | FIFO                         | Todos los protocolos iguales             |
| **Colas/buffer**        | No modelado                  | Congestión invisible                     |
| **Grosor de enlaces**   | Solo por uso                 | No muestra capacidad del enlace          |
| **Tooltips en enlaces** | No hay                       | No se ven stats al hover                 |
| **Traceroute**          | No existe                    | No hay forma de ver la ruta paso a paso  |
| **Evento "Tormenta"**   | No existe                    | No hay demostración de resiliencia       |

---

## PLAN DE IMPLEMENTACIÓN POR FASES

Cada fase es independiente, no rompe nada anterior, y tiene ~10-50 líneas de cambio.
El código es **ponytail**: minimalista, eficiente, lo justo para funcionar.

---

## FASE 0: FUNDACIÓN — Colas por nodo + Prioridad QoS

Esta fase es requisito para las fases que necesitan buffer visible y QoS.

### Archivo: `src/nucleo/tipos/EstadoRed.h`

Agregar 2 campos a `EstadoNodo` (después de `paquetes_tx`, línea 48):

```cpp
// En struct EstadoNodo (línea 44), agregar después de paquetes_tx:
float buffer_mb       = 0.0f;     // MB ocupados actualmente
float buffer_max_mb   = 50.0f;    // capacidad máxima del buffer
int   paquetes_cola   = 0;        // paquetes esperando en este nodo
```

### Archivo: `src/nucleo/SimuladorRed.h`

**Cambio 1**: Agregar función de prioridad QoS después de `ColoresProtocolo` (línea 37)

```cpp
// --- Prioridad QoS (menor = más prioritario) ---
inline int prioridadProtocolo(const std::string& tipo) {
    if (tipo == "VOIP")  return 1;
    if (tipo == "VIDEO") return 2;
    if (tipo == "HTTP")  return 3;
    if (tipo == "DNS")   return 4;
    if (tipo == "PING")  return 5;
    if (tipo == "DDOS")  return 6;
    return 5;
}
```

**Cambio 2**: En `procesarColas()`, cuando creas un paquete (después de `p.tamaño_mb = ...`, alrededor de línea 440), agrega:

```cpp
p.prioridad = prioridadProtocolo(p.tipo);
```

Y después de `timer_paquetes = 0.0f;` (línea 447), agrega el ordenamiento QoS:

```cpp
// --- QoS: ordenar paquetes por prioridad cada frame ---
std::sort(paquetes_activos.begin(), paquetes_activos.end(),
    [](const Paquete& a, const Paquete& b) {
        return a.prioridad < b.prioridad;
    });
```

**Cambio 3**: En `procesarColas()`, después de limpiar `uso_actual_mbps` (línea 452-453), resetear buffer:

```cpp
// Reset buffer de nodos
for (auto& [id, en] : estado.nodos) {
    en.buffer_mb = 0.0f;
    en.paquetes_cola = 0;
}
```

**Cambio 4**: En `procesarColas()`, en el loop que avanza paquetes, antes de `++it` (antes de línea 513), calcular buffer:

```cpp
// Calcular buffer en el nodo actual
if (estado.nodos.count(u)) {
    estado.nodos[u].buffer_mb += it->tamaño_mb;
    estado.nodos[u].paquetes_cola++;
}

// Buffer overflow check (si el buffer se llena, dropear)
if (estado.nodos.count(u) && estado.nodos[u].buffer_mb > estado.nodos[u].buffer_max_mb) {
    if (uniformeF(0, 1) < 0.15f) { // 15% de dropear cuando está lleno
        total_paquetes_perdidos++;
        it = paquetes_activos.erase(it);
        continue;
    }
}
```

**Total Fase 0**: ~30 líneas agregadas en 3 archivos.

---

## FASE 1: NOTIFICACIONES VISUALES EN CANVAS (Feedback Inmediato)

**Problema**: El usuario no entiende qué está pasando. El log tiene texto pero hay que leerlo.

**Solución**: Anuncios tipo "achievement" que aparecen en el centro del canvas y se desvanecen.

### Archivo: `src/nucleo/tipos/EstadoRed.h`

Al final, después de `EstadoSimulacion` (línea 132), agregar:

```cpp
// --- Notificación tipo "achievement" para feedback en canvas ---
struct Notificacion {
    float tiempo_real;              // ImGui::GetTime() real, no simulado
    float duracion = 3.0f;
    std::string mensaje;
    uint32_t color;                 // ARGB
};
```

Y agregar un campo en `EstadoSimulacion`:

```cpp
std::vector<Notificacion> notificaciones;  // dentro de EstadoSimulacion
```

### Archivo: `src/nucleo/SimuladorRed.h`

Agregar esta función helper (después de `microcortes_pendientes`, línea 567):

```cpp
// --- Lanzar notificación visual en canvas ---
void notificar(const std::string& msg, uint32_t color, float duracion = 3.0f) {
    estado.notificaciones.push_back({ImGui::GetTime(), duracion, msg, color});
}
```

Agregar llamadas a `notificar()` en estos lugares clave:

- En `simularFalloNodo()` (junto al `registrarTimeline`, línea 174):
  ```cpp
  notificar("🔥 " + g.nombreNodo(nodo_id) + " CAÍDO", 0xFFFF3333);
  ```
- En `restaurarNodo()` (junto al `registrarTimeline`, línea 201):
  ```cpp
  notificar("✅ " + g.nombreNodo(nodo_id) + " restaurado", 0xFF4CAF50);
  ```
- En `simularFalloArista()` (junto al `registrarTimeline`, línea 188):
  ```cpp
  notificar("⚠️ Enlace caído: " + g.nombreNodo(origen) + "→" + g.nombreNodo(destino), 0xFFFF6600);
  ```
- En alerta de CPU > 80% (línea 390):
  ```cpp
  notificar("⚠️ " + g.nombreNodo(n.id) + " SOBRECARGADO", 0xFFFF9800);
  ```

### Archivo: `src/interfaz/lienzo/LienzoRed.h`

Después del bloque de dibujo de tooltips de nodos (después de línea 1031), agregar:

```cpp
// --- Notificaciones emergentes en canvas ---
{
    float now = (float)ImGui::GetTime();
    auto& notifs = self.estado_redes.simulador.estado.notificaciones;
    ImVec2 orig = ImGui::GetCursorScreenPos();

    for (size_t i = 0; i < notifs.size(); ) {
        float edad = now - notifs[i].tiempo_real;
        if (edad > notifs[i].duracion) {
            notifs.erase(notifs.begin() + (int)i);
            continue;
        }
        float alpha = 1.0f;
        if (edad > notifs[i].duracion - 0.8f)
            alpha = (notifs[i].duracion - edad) / 0.8f;

        ImVec2 pos(orig.x + tamano.x * 0.5f, orig.y + 40.0f + i * 32.0f);
        ImVec2 ts = ImGui::CalcTextSize(notifs[i].mensaje.c_str());

        // Fondo translúcido
        dl->AddRectFilled(
            ImVec2(pos.x - ts.x * 0.5f - 12, pos.y - 6),
            ImVec2(pos.x + ts.x * 0.5f + 12, pos.y + ts.y + 6),
            IM_COL32(10, 10, 15, (int)(180 * alpha)), 6.0f);

        // Texto
        uint32_t col = notifs[i].color;
        ImU32 col_final = IM_COL32(
            ((col >> 16) & 0xFF), ((col >> 8) & 0xFF), (col & 0xFF),
            (int)(255 * alpha));
        dl->AddText(ImVec2(pos.x - ts.x * 0.5f, pos.y), col_final, notifs[i].mensaje.c_str());
        i++;
    }
}
```

**Total Fase 1**: ~40 líneas, 3 archivos tocados.

---

## FASE 2: GROSOR DINÁMICO REALISTA (Capacidad + Uso)

**Problema**: El grosor actual solo representa uso. No se ve la capacidad del enlace.

**Solución**: Dibujar cada arista en 2 capas: fondo (capacidad, gris) + brillo (uso, color).

### Archivo: `src/interfaz/lienzo/LienzoRed.h`

Buscar este bloque (alrededor de líneas 710-730):

```cpp
} else if (uso_arista > 0.02f) {
    // Brillo de fondo animado
    float glow_intensity = 0.2f + uso_arista * 0.5f;
    ...
    // color de arista base segun saturacion
    col = colorSaturacion(uso_arista);
    grosor = 2.0f + uso_arista * 4.0f;
}
```

Reemplazar con:

```cpp
} else if (uso_arista > 0.02f) {
    // Fondo = capacidad (la línea base gris representa el ancho de banda máximo)
    float bw_factor = std::min(ea.bandwidth_mbps / 100.0f, 4.0f);
    float grosor_fondo = 2.0f + bw_factor * 3.0f;
    lineaArista(dl, o->posicion, d->posicion, punto_control, es_curva,
        IM_COL32(60, 65, 75, 100), grosor_fondo);

    // Brillo animado
    float glow_intensity = 0.2f + uso_arista * 0.4f;
    ImU32 glow_col = colorSaturacion(uso_arista);
    glow_col = (glow_col & 0x00FFFFFF) | ((int)(glow_intensity * 50) << 24);
    lineaArista(dl, o->posicion, d->posicion, punto_control, es_curva,
        glow_col, grosor_fondo + 4.0f);

    // Línea de uso (brillante encima)
    col = colorSaturacion(uso_arista);
    grosor = 1.5f + uso_arista * grosor_fondo * 0.8f;

    // Onda viajera (seguir igual, mantener las ~5 líneas originales)
}

// Si el uso es >95%, borde rojo pulsante
if (uso_arista > 0.92f && !arista_caida) {
    float pulse = sinf(tiempo * 4.0f) * 0.3f + 0.7f;
    lineaArista(dl, o->posicion, d->posicion, punto_control, es_curva,
        IM_COL32(255, 50, 50, (int)(80 * pulse)), grosor + 6.0f);
}
```

**Total Fase 2**: ~15 líneas cambiadas, 1 archivo.

---

## FASE 3: TOOLTIP EN ENLACES (al hacer hover)

**Problema**: Hover sobre enlaces no muestra nada.

### Archivo: `src/interfaz/lienzo/LienzoRed.h`

Dentro del loop de dibujo de aristas, después de dibujar la línea y etiqueta (antes de seguir con la siguiente arista), agregar:

```cpp
// --- Tooltip de arista (hover) ---
if (modo_red && !arista_caida) {
    ImVec2 mouse = ImGui::GetMousePos();
    float dist_min = 999.0f;
    // Samplear 20 puntos de la línea
    for (int s = 0; s < 20; s++) {
        float t = s / 19.0f;
        ImVec2 pt = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t);
        float d = hypotf(mouse.x - pt.x, mouse.y - pt.y);
        if (d < dist_min) dist_min = d;
    }
    if (dist_min < 12.0f) {
        auto key = std::make_pair(a.origen_id, a.destino_id);
        if (self.estado_redes.simulador.estado.aristas.count(key)) {
            const auto& ea = self.estado_redes.simulador.estado.aristas.at(key);
            ImGui::BeginTooltip();
            ImGui::Text("%s → %s",
                g_dib.nombreNodo(a.origen_id).c_str(),
                g_dib.nombreNodo(a.destino_id).c_str());
            ImGui::Text("Uso: %.0f%%", uso_arista * 100.0f);
            ImGui::Text("BW: %.0f Mbps | Lat: %.1f ms", ea.bandwidth_mbps, ea.latencia_ms);
            ImGui::Text("Jitter: %.1f ms | Perdida: %.1f%%", ea.jitter_ms, ea.packet_loss * 100.0f);
            ImGui::EndTooltip();
        }
    }
}
```

**Total Fase 3**: ~25 líneas, 1 archivo.

---

## FASE 4: TRACEROUTE VISUAL

**Problema**: No hay forma de ver la ruta paso a paso.

### Archivo: `src/nucleo/SimuladorRed.h`

Después de `enviarFlujoPreset()` (línea 282), agregar:

```cpp
// --- Traceroute: un solo paquete que viaja lento mostrando cada salto ---
void enviarTraceroute(Grafo& g, int origen, int destino) {
    auto ruta = obtenerRuta(g, origen, destino);
    if (ruta.size() < 2) {
        estado.registrarEvento(estado.tiempo,
            "[Traceroute] No hay ruta entre " + g.nombreNodo(origen) +
            " y " + g.nombreNodo(destino), EventoRed::ADVERTENCIA);
        return;
    }

    Paquete p;
    p.id = contador_paquetes++;
    p.origen_id = origen;
    p.destino_id = destino;
    p.tipo = "TRACE";
    p.mbps = 0.001f;
    p.ruta = ruta;
    p.paso_actual = 0;
    p.tamaño_mb = 0.001f;
    p.prioridad = 0; // máxima prioridad
    paquetes_activos.push_back(p);

    estado.registrarEvento(estado.tiempo,
        "[Traceroute] Rastreando ruta " + g.nombreNodo(origen) +
        " → " + g.nombreNodo(destino) +
        " (" + std::to_string((int)ruta.size()-1) + " saltos)");
    notificar("🔍 Traceroute: " + g.nombreNodo(origen) + " → " + g.nombreNodo(destino),
              0xFF00BCD4, 4.0f);
}
```

En `procesarColas()`, cuando el paquete avanza de paso (`paso_actual++`), agregar:

```cpp
// --- Traceroute logging ---
if (it->tipo == "TRACE" && it->paso_actual > 0) {
    float lat_acum = 0;
    for (int si = 0; si < it->paso_actual && si + 1 < (int)it->ruta.size(); si++) {
        auto k = std::make_pair(it->ruta[si], it->ruta[si + 1]);
        if (estado.aristas.count(k)) lat_acum += estado.aristas.at(k).latencia_ms;
    }
    estado.registrarEvento(estado.tiempo,
        "[Traceroute] Salto " + std::to_string(it->paso_actual) + ": " +
        g.nombreNodo(it->ruta[it->paso_actual]) + " (" +
        std::to_string((int)lat_acum) + "ms)");
}
```

Cuando un TRACE llega a destino (en el bloque de `if (it->paso_actual + 1 >= ...)`):

```cpp
if (it->tipo == "TRACE") {
    float lat_total = 0;
    for (size_t si = 0; si + 1 < it->ruta.size(); si++) {
        auto k = std::make_pair(it->ruta[si], it->ruta[si + 1]);
        if (estado.aristas.count(k)) lat_total += estado.aristas.at(k).latencia_ms;
    }
    estado.registrarEvento(estado.tiempo,
        "[Traceroute] ✅ Destino alcanzado en " +
        std::to_string((int)it->ruta.size()-1) + " saltos (" +
        std::to_string((int)lat_total) + "ms total)",
        EventoRed::INFO);
    notificar("✅ Traceroute completo: " +
        std::to_string((int)it->ruta.size()-1) + " saltos",
        0xFF4CAF50, 4.0f);
}
```

### Archivo: `src/interfaz/lienzo/LienzoRed.h`

En el render de paquetes (alrededor de línea 818), agregar render especial para TRACE:

```cpp
if (pkt.tipo == "TRACE") {
    float tam_trace = 6.0f + sinf(tiempo * 3.0f) * 2.0f;
    dl->AddCircleFilled(pos_pkt, tam_trace * 6.0f,
        IM_COL32(0, 188, 212, (int)(20 + sinf(tiempo * 2.0f) * 10)), 20);
    dl->AddCircleFilled(pos_pkt, tam_trace, IM_COL32(0, 220, 255, 255), 20);
    dl->AddCircleFilled(pos_pkt, tam_trace * 0.5f, IM_COL32(255, 255, 255, 200), 12);
    dl->AddText(ImVec2(pos_pkt.x - 5, pos_pkt.y - 6), IM_COL32(255, 255, 255, 255), "T");
} else {
    // ... el código normal de paquete ...
}
```

### Archivo: `src/interfaz/paneles/PanelRed.h`

En la sección de envío de tráfico (después de los botones de presets rápidos, línea 143), agregar:

```cpp
ImGui::SameLine();
if (ImGui::SmallButton("Traceroute")) {
    int o = self.estado_redes.flujo_origen;
    int d = self.estado_redes.flujo_destino;
    if (o != d)
        self.estado_redes.simulador.enviarTraceroute(red, o, d);
}
```

**Total Fase 4**: ~60 líneas, 3 archivos.

---

## FASE 5: BUFFER VISIBLE (Anillo Interior en Nodos)

**Problema**: La congestión en nodos no se ve.

### Archivo: `src/interfaz/lienzo/LienzoRed.h`

Después del anillo de salud (después de línea 931-932), antes del bloque de selección, agregar:

```cpp
// --- Anillo de buffer (congestión) ---
if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
    const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
    if (en.activo) {
        float buf_ratio = std::min(1.0f, en.buffer_mb / std::max(0.1f, en.buffer_max_mb));
        if (buf_ratio > 0.02f) {
            float r_inner = n.radio * 0.65f;
            ImU32 col_buf = IM_COL32(
                (int)(buf_ratio * 255),
                (int)((1.0f - buf_ratio) * 180),
                (int)((1.0f - buf_ratio) * 100), 180);
            dl->AddCircle(n.posicion, r_inner, col_buf, 24, 2.0f + buf_ratio * 3.0f);
        }
    }
}
// --- Indicador de cola de paquetes ---
if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
    const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
    if (en.paquetes_cola > 0) {
        char qtxt[8];
        snprintf(qtxt, sizeof(qtxt), "%d", en.paquetes_cola);
        ImVec2 qs = ImGui::CalcTextSize(qtxt);
        dl->AddText(ImVec2(n.posicion.x - qs.x * 0.5f,
                           n.posicion.y + n.radio + 14),
                    IM_COL32(255, 200, 100, 200), qtxt);
    }
}
```

**Total Fase 5**: ~25 líneas, 1 archivo.

---

## FASE 6: TORMENTA DE RED (Botón de Caos)

**Problema**: No hay forma de demostrar resiliencia con un solo click.

### Archivo: `src/nucleo/SimuladorRed.h`

Después de `enviarTraceroute()`, agregar:

```cpp
// --- Tormenta de red: tumbar % de enlaces aleatorios ---
void simularTormenta(Grafo& g, float porcentaje = 0.30f, float duracion = 5.0f) {
    int a_tumbar = std::max(1, (int)(g.aristas.size() * porcentaje));
    std::vector<int> indices(g.aristas.size());
    for (size_t i = 0; i < indices.size(); i++) indices[i] = (int)i;
    std::shuffle(indices.begin(), indices.end(), gen);

    for (int i = 0; i < a_tumbar && i < (int)indices.size(); i++) {
        const auto& a = g.aristas[indices[i]];
        simularFalloArista(a.origen_id, a.destino_id, g);
        microcortes_pendientes.push_back({
            (int)a.origen_id, (int)a.destino_id,
            estado.tiempo + duracion + uniformeF(0, 2.0f)
        });
    }
    notificar("🌪️ TORMENTA DE RED: " + std::to_string(a_tumbar) + " enlaces caídos",
              0xFFFF3333, 5.0f);
    estado.registrarEvento(estado.tiempo,
        "🌪️ Tormenta de red: " + std::to_string(a_tumbar) +
        " enlaces caídos (" + std::to_string((int)(porcentaje*100)) + "%)",
        EventoRed::ERROR_RED);
}
```

### Archivo: `src/interfaz/paneles/PanelRed.h`

En la sección "Simular Fallos" (antes de cerrar el collapsing header), agregar:

```cpp
// --- Tormenta de red ---
ImGui::Spacing();
ImGui::Separator();
ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.1f, 1.0f), "🌪️ CAOS");
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.1f, 1.0f));
if (ImGui::Button("🌪️ Tormenta de Red (30%)", ImVec2(-1, 32))) {
    self.estado_redes.simulador.simularTormenta(red, 0.30f, 5.0f);
    g_sonidos.reproducir(Sonidos::NODO_CAIDO);
}
ImGui::PopStyleColor(2);
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Tumba el 30%% de enlaces aleatoriamente durante 5s.\n"
                      "Muestra cómo Dijkstra recalcula rutas en tiempo real.");
```

**Total Fase 6**: ~30 líneas, 2 archivos.

---

## FASE 7: GLASSMORPHISM (solo en tooltips e inspector)

### Archivo: `src/main.cpp`

En `configurarTemaIngenieria()` (líneas 19-101), ajustar popup:

```cpp
// --- Glassmorphism ligero ---
s.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.85f);
s.PopupRounding = 8.0f;
s.PopupBorderSize = 0.5f;
```

**Total Fase 7**: 1 línea de cambio efectiva (subir PopupRounding de 4 a 8).

---

## RESUMEN: LÍNEAS TOTALES POR FASE

| Fase      | Descripción              | Archivos | Líneas          |
| --------- | ------------------------ | -------- | --------------- |
| 0         | Colas + QoS              | 3        | ~30             |
| 1         | Notificaciones visuales  | 3        | ~40             |
| 2         | Grosor dinámico realista | 1        | ~15             |
| 3         | Tooltip en enlaces       | 1        | ~25             |
| 4         | Traceroute visual        | 3        | ~60             |
| 5         | Buffer visible (anillo)  | 1        | ~25             |
| 6         | Tormenta de red          | 2        | ~30             |
| 7         | Glassmorphism            | 1        | ~1              |
| **Total** |                          |          | **~226 líneas** |

---

## ORDEN DE IMPLEMENTACIÓN RECOMENDADO

```
Fase 0 (Fundación: colas + QoS) → requisito para Fase 5
  └── Fase 1 (Notificaciones) → impacto inmediato en entendimiento
       └── Fase 2 (Grosor dinámico) → 15 líneas, efecto premium
            └── Fase 3 (Tooltip enlaces) → 25 líneas
                 └── Fase 4 (Traceroute) → 60 líneas, la más educativa
                      └── Fase 5 (Buffer visible) → requiere Fase 0
                           └── Fase 6 (Tormenta) → requiere Fase 1
                                └── Fase 7 (Glassmorphism) → cosmético final
```

Cada fase funciona independientemente. Puedes implementar de 1 a 7 sin orden estricto,
excepto Fase 5 que necesita Fase 0, y Fase 6 que se beneficia de Fase 1.

---

## PRINCIPIOS PONYTAIL APLICADOS

1. **No abstracciones innecesarias**: Cada cambio toca el mínimo de archivos.
2. **Código mínimo**: 226 líneas para 8 mejoras sustanciales.
3. **Sin dependencias nuevas**: Todo usa ImGui + lo que ya existe.
4. **Boring over clever**: Las notificaciones usan `ImGui::GetTime()`, no temporizadores complejos.
5. **Sin boilerplate**: Los structs se agregan donde ya existen, las funciones al lado de otras similares.
6. **Deletion over addition**: Las fases 2 y 5 reemplazan código existente, no añaden versiones alternativas.
7. **Cada fase deja un checkpoint funcional**: No importa en qué fase pares, el programa compila y funciona.

---

## INSTRUCCIONES PARA EL AGENTE IA

Cada fase arriba tiene:

- **Archivo**: ruta exacta dentro de `src/`
- **Dónde**: línea de referencia o función
- **Código**: exactamente lo que hay que agregar/reemplazar

Pasos:

1. Leer primero los archivos a modificar para verificar líneas actuales
2. Aplicar los cambios en orden de fase
3. Compilar con `cd build && cmake .. && make -j$(nproc)` después de cada fase
4. Verificar que las nuevas features aparecen en la UI

**Regla de oro**: No borrar nada existente. Solo agregar structs, funciones, y bloques de renderizado. Si hay que modificar una función existente, se agregan líneas nuevas sin tocar las originales.
