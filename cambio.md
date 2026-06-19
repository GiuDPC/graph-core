#pragma once
#include "imgui.h"
}
}
} else if (uso*arista > 0.02f) {
// Brillo de fondo animado
float glow_intensity = 0.2f + uso_arista * 0.5f;
// --- Fase 2: Grosor dinámico realista (capacidad + uso) ---
auto key*ea = std::make_pair(a.origen_id, a.destino_id);
float bw_factor = 1.0f;
if (self.estado_redes.simulador.estado.aristas.count(key_ea)) {
const auto& ea = self.estado_redes.simulador.estado.aristas.at(key_ea);
bw_factor = std::min(ea.bandwidth_mbps / 100.0f, 4.0f);
}
float grosor_fondo = 2.0f + bw_factor * 3.0f;
// Fondo = capacidad (linea base gris representa ancho de banda maximo)
lineaArista(dl, o->posicion, d->posicion, punto*control, es_curva,
IM_COL32(60, 65, 75, 100), grosor_fondo);
// Brillo animado
float glow_intensity = 0.2f + uso_arista * 0.4f;
ImU32 glow*col = colorSaturacion(uso_arista);
glow_col = (glow_col & 0x00FFFFFF) | ((int)(glow_intensity * 60) << 24);
lineaArista(dl, o->posicion, d->posicion, punto*control, es_curva, glow_col, grosor + 4.0f);
glow_col = (glow_col & 0x00FFFFFF) | ((int)(glow_intensity * 50) << 24);
lineaArista(dl, o->posicion, d->posicion, punto*control, es_curva,
glow_col, grosor_fondo + 4.0f);
// onda viajera
float t_onda = fmod(tiempo * (0.5f + uso*arista * 2.0f), 1.0f);
ImVec2 pos*onda = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t_onda);
ImU32 col_onda = colorSaturacion(uso_arista);
dl->AddCircleFilled(pos_onda, 3.0f + uso_arista * 3.0f, col*onda, 10);
// color de arista base segun saturacion
// Linea de uso (brillante encima)
col = colorSaturacion(uso_arista);
grosor = 2.0f + uso_arista * 4.0f;
grosor = 1.5f + uso*arista * grosor*fondo * 0.8f;
}
// Si el uso es >92%, borde rojo pulsante
if (uso*arista > 0.92f && !arista_caida) {
float pulse = sinf(tiempo * 4.0f) _ 0.3f + 0.7f;
lineaArista(dl, o->posicion, d->posicion, punto_control, es_curva,
IM_COL32(255, 50, 50, (int)(80 _ pulse)), grosor + 6.0f);
}
}
// linea base
dl->AddLine(ImVec2(mid2.x - sz, mid2.y - sz), ImVec2(mid2.x + sz, mid2.y + sz), col*x, 3.0f);
dl->AddLine(ImVec2(mid2.x + sz, mid2.y - sz), ImVec2(mid2.x - sz, mid2.y + sz), col_x, 3.0f);
}
// --- Fase 3: Tooltip de arista (hover) ---
if (modo_red && !arista_caida) {
ImVec2 mouse_edge = ImGui::GetMousePos();
float dist_min_edge = 999.0f;
for (int s = 0; s < 20; s++) {
float t_s = s / 19.0f;
ImVec2 pt = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t_s);
float d_e = hypotf(mouse_edge.x - pt.x, mouse_edge.y - pt.y);
if (d_e < dist_min_edge) dist_min_edge = d_e;
}
if (dist_min_edge < 12.0f) {
auto key_tt = std::make_pair(a.origen_id, a.destino_id);
if (self.estado_redes.simulador.estado.aristas.count(key_tt)) {
const auto& ea_tt = self.estado_redes.simulador.estado.aristas.at(key_tt);
ImGui::BeginTooltip();
ImGui::Text("%s -> %s",
g_dib.nombreNodo(a.origen_id).c_str(),
g_dib.nombreNodo(a.destino_id).c_str());
ImGui::Text("Uso: %.0f%%", uso_arista * 100.0f);
ImGui::Text("BW: %.0f Mbps | Lat: %.1f ms", ea*tt.bandwidth_mbps, ea_tt.latencia_ms);
ImGui::Text("Jitter: %.1f ms | Perdida: %.1f%%", ea_tt.jitter_ms, ea_tt.packet_loss * 100.0f);
ImGui::EndTooltip();
}
}
}
}
// dibujar paquetes
float t = Easing::easeInOutCubic(pkt.progreso);
ImVec2 pos*pkt = puntoEnArista(no->posicion, nd->posicion, pkt_pc, pkt_curvo, t);
// --- Fase 4: Render especial para TRACE ---
if (pkt.tipo == "TRACE") {
float tam_trace = 6.0f + sinf(tiempo * 3.0f) _ 2.0f;
dl->AddCircleFilled(pos_pkt, tam_trace _ 6.0f,
IM*COL32(0, 188, 212, (int)(20 + sinf(tiempo * 2.0f) _ 10)), 20);
dl->AddCircleFilled(pos_pkt, tam_trace, IM_COL32(0, 220, 255, 255), 20);
dl->AddCircleFilled(pos_pkt, tam_trace _ 0.5f, IM*COL32(255, 255, 255, 200), 12);
dl->AddText(ImVec2(pos_pkt.x - 5, pos_pkt.y - 6), IM_COL32(255, 255, 255, 255), "T");
} else {
ImU32 col_pkt = imColorProtocolo(pkt.tipo);
float tam = 2.5f + pkt.tamaño_mb * 0.3f;
if (tam > 5.5f) tam = 5.5f;
dl->AddText(ImVec2(pos*pkt.x - ls.x * 0.5f - 8, pos*pkt.y - ls.y * 0.5f - 8),
IM*COL32(255, 255, 255, 180), label);
} // end else (not TRACE)
// tooltip de inspeccion (hitbox expandido para atraparlos facil)
ImVec2 mouse = ImGui::GetMousePos();
if (std::hypot(mouse.x - pos_pkt.x, mouse.y - pos_pkt.y) < 25.0f) {
}
}
// --- Fase 5: Anillo de buffer (congestion) ---
if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
const auto& en_buf = self.estado_redes.simulador.estado.nodos.at(n.id);
if (en_buf.activo) {
float buf_ratio = std::min(1.0f, en_buf.buffer_mb / std::max(0.1f, en_buf.buffer_max_mb));
if (buf_ratio > 0.02f) {
float r_inner = n.radio * 0.65f;
ImU32 col*buf = IM_COL32(
(int)(buf_ratio * 255),
(int)((1.0f - buf*ratio) * 180),
(int)((1.0f - buf*ratio) * 100), 180);
dl->AddCircle(n.posicion, r*inner, col_buf, 24, 2.0f + buf_ratio * 3.0f);
}
}
}
// --- Indicador de cola de paquetes ---
if (modo*red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
const auto& en_q = self.estado_redes.simulador.estado.nodos.at(n.id);
if (en_q.paquetes_cola > 0) {
char qtxt[8];
snprintf(qtxt, sizeof(qtxt), "%d", en_q.paquetes_cola);
ImVec2 qs = ImGui::CalcTextSize(qtxt);
dl->AddText(ImVec2(n.posicion.x - qs.x * 0.5f,
n.posicion.y + n.radio + 14),
IM*COL32(255, 200, 100, 200), qtxt);
}
}
// seleccion / hover
if ((es_g2 && editando_g2 && n.id == self.estado_ui.nodo_seleccionado) ||
(!es_g2 && !editando_g2 && n.id == self.estado_ui.nodo_seleccionado)) {
}
}
// --- Fase 1: Notificaciones emergentes en canvas ---
{
float now = (float)ImGui::GetTime();
auto& notifs = self.estado_redes.simulador.estado.notificaciones;
for (size_t i = 0; i < notifs.size(); ) {
float edad = now - notifs[i].tiempo_real;
if (edad > notifs[i].duracion) {
notifs.erase(notifs.begin() + (int)i);
continue;
}
float alpha = 1.0f;
if (edad > notifs[i].duracion - 0.8f)
alpha = (notifs[i].duracion - edad) / 0.8f;
ImVec2 pos_n(origin.x + tamano.x * 0.5f, origin.y + 40.0f + i _ 32.0f);
ImVec2 ts_n = ImGui::CalcTextSize(notifs[i].mensaje.c_str());
// Fondo translucido
dl->AddRectFilled(
ImVec2(pos_n.x - ts_n.x _ 0.5f - 12, pos*n.y - 6),
ImVec2(pos_n.x + ts_n.x * 0.5f + 12, pos*n.y + ts_n.y + 6),
IM_COL32(10, 10, 15, (int)(180 * alpha)), 6.0f);
// Texto
uint32*t col_n = notifs[i].color;
ImU32 col_final = IM_COL32(
((col_n >> 16) & 0xFF), ((col_n >> 8) & 0xFF), (col_n & 0xFF),
(int)(255 * alpha));
dl->AddText(ImVec2(pos*n.x - ts_n.x * 0.5f, pos*n.y), col_final, notifs[i].mensaje.c_str());
i++;
}
}
//overlays de resultados de algoritmo en el lienzo
if (self.estado_ui.modo_actual == Interfaz::ModoApp::Grafos) {
float ox = origin.x + 12, oy = origin.y + 12;
}
}
#pragma once
#include "imgui.h"
if (ImGui::SmallButton("DDoS")) sim.enviarFlujoPreset(red, 3);
ImGui::SameLine();
if (ImGui::SmallButton("Supervivencia")) sim.enviarFlujoPreset(red, 4);
ImGui::SameLine();
if (ImGui::SmallButton("Traceroute")) {
int o = self.estado_redes.flujo_origen;
int d = self.estado_redes.flujo_destino;
if (o != d)
self.estado_redes.simulador.enviarTraceroute(red, o, d);
}
if (ImGui::IsItemHovered())
ImGui::SetTooltip("Envia un paquete TRACE que viaja lento mostrando cada salto.\nSelecciona origen/destino en 'Enviar Trafico'.");
}
// Leyenda de colores de protocolo
if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simula un corte de cable en el enlace seleccionado.");
ImGui::PopStyleColor();
}
// --- Fase 6: Tormenta de red ---
ImGui::Spacing();
ImGui::Separator();
ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.1f, 1.0f), "CAOS");
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.1f, 1.0f));
if (ImGui::Button("Tormenta de Red (30%)", ImVec2(-1, 32))) {
self.estado_redes.simulador.simularTormenta(red, 0.30f, 5.0f);
g_sonidos.reproducir(Sonidos::NODO_CAIDO);
}
ImGui::PopStyleColor(2);
if (ImGui::IsItemHovered())
ImGui::SetTooltip("Tumba el 30%% de enlaces aleatoriamente durante 5s.\n"
"Muestra como Dijkstra recalcula rutas en tiempo real.");
}
// Tabla de ruteo
}
#pragma once
#include <random>
}
}
// --- Prioridad QoS (menor = mas prioritario) ---
inline int prioridadProtocolo(const std::string& tipo) {
if (tipo == "VOIP") return 1;
if (tipo == "VIDEO") return 2;
if (tipo == "HTTP") return 3;
if (tipo == "DNS") return 4;
if (tipo == "PING") return 5;
if (tipo == "DDOS") return 6;
return 5;
}
// Motor de simulacion de red mejorado
class SimuladorRed {
public:
float tamaño_mb;
int paso_actual = 0;
std::vector<int> ruta;
int prioridad = 5; // QoS: menor = mas prioritario
};
const std::vector<Paquete>& obtenerPaquetes() const { return paquetes_activos; }
}
// ── Fallos / Restauraciones con timeline ────────────────────────────────
// --- Lanzar notificacion visual en canvas ---
void notificar(const std::string& msg, uint32_t color, float duracion = 3.0f) {
estado.notificaciones.push_back({(float)ImGui::GetTime(), duracion, msg, color});
}
void simularFalloNodo(int nodo_id, Grafo& g) {
if (!estado.nodos.count(nodo_id)) return;
estado.nodos[nodo_id].activo = false;
estado.nodos[nodo_id].tiempo_caida = 0.0f;
estado.registrarTimeline(estado.tiempo, g.nombreNodo(nodo_id) + " CAIDO",
0xFFFF3333, TimelineEvent::CORTE);
estado.registrarEvento(estado.tiempo,
g.nombreNodo(nodo_id) + " CAIDO — recalculando rutas...",
EventoRed::ERROR_RED);
notificar(g.nombreNodo(nodo_id) + " CAIDO", 0xFFFF3333);
cache_rutas.clear();
g_sonidos.reproducir(Sonidos::NODO_CAIDO);
}
estado.registrarEvento(estado.tiempo,
"Enlace " + g.nombreNodo(origen) + " -> " + g.nombreNodo(destino) + " CAIDO",
EventoRed::ERROR_RED);
notificar("Enlace caido: " + g.nombreNodo(origen) + " -> " + g.nombreNodo(destino), 0xFFFF6600);
cache_rutas.clear();
g_sonidos.reproducir(Sonidos::NODO_CAIDO);
}
void restaurarNodo(int nodo_id, Grafo& g) {
if (!estado.nodos.count(nodo_id)) return;
estado.nodos[nodo_id].activo = true;
estado.registrarTimeline(estado.tiempo, g.nombreNodo(nodo_id) + " restaurado",
0xFF4CAF50, TimelineEvent::RESTAURACION);
estado.registrarEvento(estado.tiempo,
g.nombreNodo(nodo_id) + " restaurado", EventoRed::INFO);
notificar(g.nombreNodo(nodo_id) + " restaurado", 0xFF4CAF50);
cache_rutas.clear();
g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
}
}
}
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
p.prioridad = 0; // maxima prioridad
paquetes_activos.push_back(p);
estado.registrarEvento(estado.tiempo,
"[Traceroute] Rastreando ruta " + g.nombreNodo(origen) +
" -> " + g.nombreNodo(destino) +
" (" + std::to_string((int)ruta.size()-1) + " saltos)");
notificar("Traceroute: " + g.nombreNodo(origen) + " -> " + g.nombreNodo(destino),
0xFF00BCD4, 4.0f);
}
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
notificar("TORMENTA DE RED: " + std::to_string(a_tumbar) + " enlaces caidos",
0xFFFF3333, 5.0f);
estado.registrarEvento(estado.tiempo,
"Tormenta de red: " + std::to_string(a_tumbar) +
" enlaces caidos (" + std::to_string((int)(porcentaje*100)) + "%)",
EventoRed::ERROR_RED);
}
private:
std::mt19937 gen{42};
std::unordered_map<std::string, std::vector<int>> cache_rutas;
EventoRed::ADVERTENCIA);
estado.registrarTimeline(estado.tiempo, g.nombreNodo(n.id) + " sobrecargado",
0xFFFF9800, TimelineEvent::SOBRECARGA);
notificar(g.nombreNodo(n.id) + " SOBRECARGADO", 0xFFFF9800);
g_sonidos.reproducir(Sonidos::ARISTA_SATURADA);
}
}
p.ruta = ruta;
p.paso_actual = 0;
p.tamaño_mb = 0.5f + std::min(it->mbps * 0.01f, 3.5f);
p.prioridad = prioridadProtocolo(p.tipo);
paquetes*activos.push_back(p);
total_paquetes_enviados++;
}
++it;
}
if (timer_paquetes >= 0.1f) timer_paquetes = 0.0f;
// --- QoS: ordenar paquetes por prioridad cada frame ---
std::sort(paquetes_activos.begin(), paquetes_activos.end(),
[](const Paquete& a, const Paquete& b) {
return a.prioridad < b.prioridad;
});
// 2. Avanzar paquetes activos + acumular uso en aristas
// Resetear uso_actual_mbps a solo paquetes activos (sistema mas realista)
for (auto& [key, ea] : estado.aristas) {
if (ea.activa) ea.uso_actual_mbps = 0.0f;
}
// Reset buffer de nodos
for (auto& [id, en] : estado.nodos) {
en.buffer_mb = 0.0f;
en.paquetes_cola = 0;
}
for (auto it = paquetes_activos.begin(); it != paquetes_activos.end(); ) {
if (it->paso_actual + 1 >= (int)it->ruta.size()) {
int dst = it->destino_id;
if (estado.nodos.count(dst)) {
estado.nodos[dst].paquetes_rx++;
}
// --- Traceroute: llegada a destino ---
if (it->tipo == "TRACE") {
float lat_total = 0;
for (size_t si = 0; si + 1 < it->ruta.size(); si++) {
auto k = std::make_pair(it->ruta[si], it->ruta[si + 1]);
if (estado.aristas.count(k)) lat_total += estado.aristas.at(k).latencia_ms;
}
estado.registrarEvento(estado.tiempo,
"[Traceroute] Destino alcanzado en " +
std::to_string((int)it->ruta.size()-1) + " saltos (" +
std::to_string((int)lat_total) + "ms total)",
EventoRed::INFO);
notificar("Traceroute completo: " +
std::to_string((int)it->ruta.size()-1) + " saltos",
0xFF4CAF50, 4.0f);
}
total_paquetes_entregados++;
it = paquetes_activos.erase(it);
continue;
ea.uso_actual_mbps += it->mbps / std::max(velocidad, 0.01f) * 0.1f;
}
// Calcular buffer en el nodo actual
if (estado.nodos.count(u)) {
estado.nodos[u].buffer_mb += it->tamaño_mb;
estado.nodos[u].paquetes_cola++;
}
// Buffer overflow check (si el buffer se llena, dropear)
if (estado.nodos.count(u) && estado.nodos[u].buffer_mb > estado.nodos[u].buffer_max_mb) {
if (uniformeF(0, 1) < 0.15f) { // 15% de dropear cuando esta lleno
total_paquetes_perdidos++;
it = paquetes_activos.erase(it);
continue;
}
}
it->progreso += dt / std::max(velocidad, 0.01f);
if (it->progreso >= 1.0f) {
it->progreso = 0.0f;
it->paso_actual++;
if (estado.nodos.count(v)) estado.nodos[v].paquetes_rx++;
if (estado.nodos.count(u)) estado.nodos[u].paquetes_tx++;
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
}
++it;
}
std::vector<MicroCorte> microcortes_pendientes;
};
#pragma once
#include <map>
#include <vector>
float memoria_uso = 0.0f; // 0.0 - 1.0
float paquetes_rx = 0.0f; // paquetes/segundo recibidos
float paquetes_tx = 0.0f; // paquetes/segundo enviados
float buffer_mb = 0.0f; // MB ocupados actualmente
float buffer_max_mb = 50.0f; // capacidad maxima del buffer
int paquetes_cola = 0; // paquetes esperando en este nodo
bool activo = true; // false = nodo caido (failover)
float uptime = 0.0f; // segundos desde inicio simulacion
float tiempo_caida = 0.0f; // si activo=false, cuantos seg lleva caido
enum Tipo { INSTANTE, SPIKE, CORTE, RESTAURACION, SOBRECARGA } tipo;
};
// --- Notificacion tipo "achievement" para feedback en canvas ---
struct Notificacion {
float tiempo_real; // ImGui::GetTime() real, no simulado
float duracion = 3.0f;
std::string mensaje;
uint32_t color; // ARGB
};
// Estado global de la simulacion
struct EstadoSimulacion {
bool activa = false;
float tiempo = 0.0f; // segundos desde inicio
float velocidad = 1.0f; // multiplicador de velocidad
std::map<int, EstadoNodo> nodos;
std::map<std::pair<int,int>, EstadoArista> aristas;
std::vector<FlujoTrafico> flujos;
std::deque<EventoRed> log_eventos; // ultimos 100 eventos
std::deque<TimelineEvent> timeline; // eventos para timeline grafico
EstadisticasRed stats;
std::vector<Notificacion> notificaciones;
void registrarEvento(float t, const std::string& msg,
EventoRed::Severidad sev = EventoRed::INFO) {
}
};
