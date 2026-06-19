#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <deque>
#include <chrono>
#include "Grafo.h"
#include "tipos/EstadoRed.h"
#include "algoritmos/Dijkstra.h"
#include "audio/Sonidos.h"

extern Sonidos g_sonidos;

// ── Colores de protocolo (uint32_t ARGB) ────────────────────────────────────
namespace ColoresProtocolo {
    inline uint32_t paraTipo(const std::string& tipo) {
        if (tipo == "HTTP")    return 0xFF4CAF50; // verde
        if (tipo == "PING")    return 0xFF00BCD4; // cian
        if (tipo == "VIDEO")   return 0xFF9C27B0; // purpura
        if (tipo == "DDOS")    return 0xFFF44336; // rojo
        if (tipo == "VOIP")    return 0xFFFF9800; // naranja
        if (tipo == "DNS")     return 0xFF2196F3; // azul
        if (tipo == "FTP")     return 0xFF795548; // marron
        return 0xFFB0BEC5;                        // gris default
    }
    inline const char* icono(const std::string& tipo) {
        if (tipo == "HTTP")    return "H";
        if (tipo == "PING")    return "P";
        if (tipo == "VIDEO")   return "V";
        if (tipo == "DDOS")    return "D";
        if (tipo == "VOIP")    return "A";
        if (tipo == "DNS")     return "?";
        if (tipo == "FTP")     return "F";
        return "•";
    }
}

// --- Prioridad QoS (menor = mas prioritario) ---
inline int prioridadProtocolo(const std::string& tipo) {
    if (tipo == "VOIP")  return 1;
    if (tipo == "VIDEO") return 2;
    if (tipo == "HTTP")  return 3;
    if (tipo == "DNS")   return 4;
    if (tipo == "PING")  return 5;
    if (tipo == "DDOS")  return 6;
    return 5;
}

// Motor de simulacion de red mejorado
class SimuladorRed {
public:
    EstadoSimulacion estado;

    bool eventos_automaticos = true;
    float probabilidad_spike  = 0.002f;

    void inicializar(const Grafo& g) {
        estado = {};
        contador_paquetes = 0;
        total_paquetes_enviados = 0;
        total_paquetes_perdidos = 0;
        total_paquetes_entregados = 0;
        for (const auto& n : g.nodos) {
            EstadoNodo en;
            en.activo       = true;
            en.cpu_uso      = 0.05f + uniformeF(0.0f, 0.15f);
            en.memoria_uso  = 0.10f + uniformeF(0.0f, 0.20f);
            estado.nodos[n.id] = en;
        }
        for (const auto& a : g.aristas) {
            auto key = std::make_pair(a.origen_id, a.destino_id);
            EstadoArista ea;
            ea.bandwidth_mbps  = a.peso * 10.0f;
            ea.latencia_ms     = a.peso;
            ea.uso_actual_mbps = 0.0f;
            ea.jitter_ms       = 0.0f;
            ea.packet_loss     = 0.0f;
            ea.activa          = true;
            estado.aristas[key] = ea;
        }
        estado.activa = true;
        estado.tiempo = 0.0f;
        cache_rutas.clear();
        timer_stats = 0.0f;
    }

    void tick(Grafo& g, float dt) {
        if (!estado.activa) return;
        float dt_sim = dt * estado.velocidad;
        estado.tiempo += dt_sim;

        actualizarNodos(g, dt_sim);
        actualizarAristas(g, dt_sim);
        procesarColas(g, dt_sim);

        // Colectar hist. uso aristas + stats cada ~0.5s sim
        timer_stats += dt_sim;
        if (timer_stats >= 0.5f) {
            timer_stats = 0.0f;
            colectarEstadisticas(g);
        }

        if (eventos_automaticos) {
            generarEventos(g, dt_sim);
        }
    }

    // ── API publica ──────────────────────────────────────────────────────────

    // Obtener la tabla de enrutamiento para un nodo (destino -> siguiente salto)
    std::vector<std::pair<int, std::string>> tablaRuteo(const Grafo& g, int nodo_id) {
        std::vector<std::pair<int, std::string>> result;
        if (!estado.nodos.count(nodo_id) || !estado.nodos[nodo_id].activo) return result;
        for (const auto& n : g.nodos) {
            if (n.id == nodo_id) continue;
            auto ruta = obtenerRuta(g, nodo_id, n.id);
            if (ruta.size() >= 2) {
                int sig = ruta[1];
                result.push_back({n.id, g.nombreNodo(sig)});
            }
        }
        return result;
    }

    // Paquete individual en transito
    struct Paquete {
        int id = 0;                     // unico para inspector
        int origen_id;
        int destino_id;
        float mbps;
        std::string tipo;
        float progreso = 0.0f;
        float tamaño_mb;
        int paso_actual = 0;
        std::vector<int> ruta;
        int prioridad = 5;              // QoS: menor = mas prioritario
    };

    const std::vector<Paquete>& obtenerPaquetes() const { return paquetes_activos; }

    // Datos de estadisticas
    int totalPaquetesEnviados() const { return total_paquetes_enviados; }
    int totalPaquetesPerdidos() const { return total_paquetes_perdidos; }
    int totalPaquetesEntregados() const { return total_paquetes_entregados; }

    void enviarFlujo(int origen, int destino, float mbps,
                     const std::string& tipo, float duracion, Grafo& g) {
        FlujoTrafico f;
        f.origen_id       = origen;
        f.destino_id      = destino;
        f.mbps            = mbps;
        f.tipo            = tipo;
        f.tiempo_restante = duracion;
        estado.flujos.push_back(f);

        uint32_t col = ColoresProtocolo::paraTipo(tipo);
        estado.registrarTimeline(estado.tiempo, tipo + " " + std::to_string((int)mbps) + "Mbps",
            col, TimelineEvent::SPIKE, duracion);

        estado.registrarEvento(estado.tiempo,
            "Flujo " + tipo + ": " + g.nombreNodo(origen) + " -> " + g.nombreNodo(destino) +
            " (" + std::to_string((int)mbps) + " Mbps)", EventoRed::INFO);
        g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
    }

    float usoArista(int origen, int destino) const {
        auto key = std::make_pair(origen, destino);
        if (!estado.aristas.count(key)) return 0.0f;
        const auto& ea = estado.aristas.at(key);
        if (ea.bandwidth_mbps <= 0) return 0.0f;
        return std::min(1.0f, ea.uso_actual_mbps / ea.bandwidth_mbps);
    }

    float perdidaEnArista(int origen, int destino) const {
        auto key = std::make_pair(origen, destino);
        if (!estado.aristas.count(key)) return 0.0f;
        return estado.aristas.at(key).packet_loss;
    }

    // ── Fallos / Restauraciones con timeline ────────────────────────────────
    // --- Lanzar notificacion visual en canvas ---
    static float tiempoReal() {
        static auto t0 = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(std::chrono::steady_clock::now() - t0).count();
    }
    void notificar(const std::string& msg, uint32_t color, float duracion = 3.0f) {
        estado.notificaciones.push_back({tiempoReal(), duracion, msg, color});
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

    void simularFalloArista(int origen, int destino, Grafo& g) {
        auto key = std::make_pair(origen, destino);
        if (estado.aristas.count(key)) estado.aristas[key].activa = false;
        // NO desactivar key2 (destino, origen) — en modo dirigido son enlaces separados
        estado.registrarTimeline(estado.tiempo,
            g.nombreNodo(origen) + "->" + g.nombreNodo(destino) + " caido",
            0xFFFF3333, TimelineEvent::CORTE);
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

    void restaurarArista(int origen, int destino, Grafo& g) {
        auto key = std::make_pair(origen, destino);
        if (estado.aristas.count(key)) estado.aristas[key].activa = true;
        // NO restaurar key2 — en modo dirigido son enlaces separados
        estado.registrarTimeline(estado.tiempo,
            g.nombreNodo(origen) + "->" + g.nombreNodo(destino) + " restaurado",
            0xFF4CAF50, TimelineEvent::RESTAURACION);
        estado.registrarEvento(estado.tiempo,
            "Enlace " + g.nombreNodo(origen) + " -> " + g.nombreNodo(destino) + " restaurado",
            EventoRed::INFO);
        cache_rutas.clear();
    }

    void enviarFlujoPreset(Grafo& g, int preset) {
        if (g.nodos.size() < 2) return;
        // 0=Personalizado, 1=Videollamada, 2=Empresarial, 3=DDoS, 4=Supervivencia
        struct PresetFlujo {
            int   src_idx, dst_idx;
            float mbps;
            const char* tipo;
            float duracion;
        };

        std::vector<PresetFlujo> preset_flujos;
        int n = (int)g.nodos.size();

        switch (preset) {
            case 1: { // Videollamada: VoIP bidireccional + senializacion
                preset_flujos = {
                    {0, n-1, 0.1f, "PING", 30.0f},
                    {n-1, 0, 0.1f, "PING", 30.0f},
                    {0, n-1, 0.5f, "VOIP", 30.0f},
                    {n-1, 0, 0.5f, "VOIP", 30.0f},
                    {0, n-1, 3.0f, "HTTP", 30.0f},
                };
                break;
            }
            case 2: { // Empresarial: HTTP + DNS + FTP + PING monitoreo
                int server = 0;
                int dns = (n > 2) ? 1 : 0;
                int fw = (n > 3) ? 2 : 0;
                preset_flujos = {
                    {server, n-1, 2.0f, "HTTP", 20.0f},
                    {dns, server, 0.1f, "DNS", 30.0f},
                    {fw, n-1, 5.0f, "FTP", 15.0f},
                    {n-1, server, 0.1f, "PING", 30.0f},
                };
                break;
            }
            case 3: { // DDoS: multiples flujos agresivos
                int target = 0;
                for (int i = 1; i < std::min(n, 6); i++) {
                    preset_flujos.push_back({i, target, 300.0f + uniformeF(0, 200), "DDOS", 8.0f});
                }
                // contra-trafico legitimo
                preset_flujos.push_back({target, n-1, 1.0f, "HTTP", 8.0f});
                break;
            }
            default: // Supervivencia: topologia mesh con monitoreo
                for (int i = 0; i < std::min(n, 8); i++) {
                    int dst = (i + 1 + rand() % (n-1)) % n;
                    float mb = 1.0f + uniformeF(0, 5.0f);
                    const char* tipos[] = {"HTTP", "DNS", "PING"};
                    preset_flujos.push_back({i, dst, mb, tipos[i%3], 15.0f + uniformeF(0, 15.0f)});
                }
                break;
        }

        for (const auto& pf : preset_flujos) {
            if (pf.src_idx < n && pf.dst_idx < n && pf.src_idx != pf.dst_idx) {
                enviarFlujo(g.nodos[pf.src_idx].id, g.nodos[pf.dst_idx].id,
                    pf.mbps, pf.tipo, pf.duracion, g);
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
    float timer_paquetes = 0.0f;
    float timer_stats = 0.0f;
    int contador_paquetes = 0;
    int total_paquetes_enviados = 0;
    int total_paquetes_perdidos = 0;
    int total_paquetes_entregados = 0;

    float uniformeF(float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(gen);
    }

    std::vector<int> obtenerRuta(const Grafo& g, int origen, int destino) {
        std::string key = std::to_string(origen) + ":" + std::to_string(destino);
        auto it = cache_rutas.find(key);
        if (it != cache_rutas.end()) return it->second;

        auto filtro_nodo = [&](int id) {
            return estado.nodos.count(id) && estado.nodos.at(id).activo;
        };
        auto filtro_arista = [&](int u, int v) {
            auto k1 = std::make_pair(u, v);
            if (estado.aristas.count(k1) && !estado.aristas.at(k1).activa) return false;
            // NO checkear k2 — en modo dirigido la direccion opuesta es un enlace separado
            return true;
        };

        // Nota: Dijkstra ya respeta es_dirigida internamente (Dijkstra.h line 51)
        auto res = Algoritmos::dijkstra(g, origen, destino, false, false, filtro_nodo, filtro_arista);
        if (res.hay_ruta) {
            cache_rutas[key] = res.ruta;
            return res.ruta;
        }
        return {};
    }

    void colectarEstadisticas(Grafo& g) {
        // Throughput total
        float tp_total = 0.0f;
        float lat_total = 0.0f;
        float jit_total = 0.0f;
        int aristas_act = 0;
        for (auto& [key, ea] : estado.aristas) {
            if (!ea.activa) continue;
            tp_total += ea.uso_actual_mbps;
            lat_total += ea.latencia_ms;
            jit_total += ea.jitter_ms;
            aristas_act++;
            // Historial por arista
            float uso_norm = (ea.bandwidth_mbps > 0) ? std::min(1.0f, ea.uso_actual_mbps / ea.bandwidth_mbps) : 0.0f;
            ea.hist_uso.agregar(uso_norm);
        }
        if (aristas_act > 0) {
            estado.stats.latencia_promedio_ms = lat_total / aristas_act;
            estado.stats.jitter_promedio_ms = jit_total / aristas_act;
        }
        estado.stats.throughput_total_mbps = tp_total;
        estado.stats.hist_throughput.agregar(tp_total);

        // Packet loss rate
        float perdida_rate = (total_paquetes_enviados > 0)
            ? (float)total_paquetes_perdidos / total_paquetes_enviados : 0.0f;
        estado.stats.paquetes_perdidos_total = (float)total_paquetes_perdidos;
        estado.stats.hist_perdida.agregar(perdida_rate);

        // Historial por nodo
        for (auto& [id, en] : estado.nodos) {
            en.hist_cpu.agregar(en.cpu_uso);
            en.hist_ram.agregar(en.memoria_uso);
        }
    }

    void actualizarNodos(Grafo& g, float dt) {
        for (const auto& n : g.nodos) {
            if (!estado.nodos.count(n.id)) continue;
            auto& en = estado.nodos[n.id];
            if (!en.activo) {
                en.tiempo_caida += dt;
                continue;
            }

            en.uptime += dt;

            float cpu_base = 0.05f + sinf(en.uptime * 0.1f) * 0.03f;
            float cpu_ruteo = 0.0f;

            int flujos_pasan = 0;
            for (const auto& f : estado.flujos) {
                auto ruta = obtenerRuta(g, f.origen_id, f.destino_id);
                for (size_t i = 0; i < ruta.size(); i++) {
                    if (ruta[i] == n.id) { flujos_pasan++; break; }
                }
            }
            cpu_ruteo = flujos_pasan * 0.05f;

            float ram_base = 0.10f + flujos_pasan * 0.02f;

            en.cpu_uso     = std::clamp(en.cpu_uso * 0.9f + (cpu_base + cpu_ruteo) * 0.1f, 0.02f, 0.99f);
            en.memoria_uso = std::clamp(en.memoria_uso * 0.9f + ram_base * 0.1f, 0.05f, 0.99f);

            if (en.cpu_uso > 0.80f && uniformeF(0, 1) < 0.01f) {
                estado.registrarEvento(estado.tiempo,
                    g.nombreNodo(n.id) + " sobrecargado (CPU:" + std::to_string((int)(en.cpu_uso*100)) + "%)",
                    EventoRed::ADVERTENCIA);
                estado.registrarTimeline(estado.tiempo, g.nombreNodo(n.id) + " sobrecargado",
                    0xFFFF9800, TimelineEvent::SOBRECARGA);
                notificar(g.nombreNodo(n.id) + " SOBRECARGADO", 0xFFFF9800);
                g_sonidos.reproducir(Sonidos::ARISTA_SATURADA);
            }
        }
    }

    void actualizarAristas(Grafo& g, float dt) {
        for (auto& [key, ea] : estado.aristas) {
            if (!ea.activa) continue;
            ea.uso_actual_mbps *= std::pow(0.90f, dt);
            if (ea.uso_actual_mbps < 0.01f) ea.uso_actual_mbps = 0.0f;

            float uso_normalizado = std::min(1.0f, ea.uso_actual_mbps / ea.bandwidth_mbps);
            ea.jitter_ms = uso_normalizado * ea.latencia_ms * 0.3f;

            if (uso_normalizado > 0.9f) {
                ea.packet_loss = (uso_normalizado - 0.9f) * 0.5f;
            } else {
                ea.packet_loss = 0.0f;
            }
        }
    }

    std::vector<Paquete> paquetes_activos;

    void procesarColas(Grafo& g, float dt) {
        timer_paquetes += dt;

        // 1. Crear paquetes desde flujos
        for (auto it = estado.flujos.begin(); it != estado.flujos.end(); ) {
            it->tiempo_restante -= dt;
            if (it->tiempo_restante <= 0) {
                it = estado.flujos.erase(it);
                continue;
            }

            auto ruta = obtenerRuta(g, it->origen_id, it->destino_id);
            if (ruta.size() < 2) { ++it; continue; }

            float intervalo = 0.1f;
            if (timer_paquetes >= intervalo) {
                Paquete p;
                p.id = contador_paquetes++;
                p.origen_id   = it->origen_id;
                p.destino_id  = it->destino_id;
                p.mbps        = it->mbps;
                p.tipo        = it->tipo;
                p.ruta        = ruta;
                p.paso_actual = 0;
                p.tamaño_mb   = 0.5f + std::min(it->mbps * 0.01f, 3.5f);
                p.prioridad   = prioridadProtocolo(p.tipo);
                paquetes_activos.push_back(p);
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
            }

            int u = it->ruta[it->paso_actual];
            int v = it->ruta[it->paso_actual + 1];
            
            if (estado.nodos.count(u) && !estado.nodos.at(u).activo) {
                total_paquetes_perdidos++;
                it = paquetes_activos.erase(it);
                continue;
            }
            if (estado.nodos.count(v) && !estado.nodos.at(v).activo) {
                total_paquetes_perdidos++;
                it = paquetes_activos.erase(it);
                continue;
            }

            auto key = std::make_pair(u, v);

            float velocidad = 1.0f;
            if (estado.aristas.count(key)) {
                auto& ea = estado.aristas.at(key);
                if (!ea.activa) {
                    total_paquetes_perdidos++;
                    it = paquetes_activos.erase(it);
                    continue;
                }
                velocidad = ea.latencia_ms / 1000.0f + ea.jitter_ms / 1000.0f;

                if (uniformeF(0, 1) < ea.packet_loss) {
                    estado.registrarEvento(estado.tiempo,
                        "Paquete perdido en " + g.nombreNodo(u) + " -> " + g.nombreNodo(v) +
                        " (congestion " + std::to_string((int)(ea.uso_actual_mbps / ea.bandwidth_mbps * 100)) + "%)",
                        EventoRed::ADVERTENCIA);
                    total_paquetes_perdidos++;
                    it = paquetes_activos.erase(it);
                    continue;
                }

                // Acumular uso
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
    }

    void generarEventos(Grafo& g, float dt) {
        if (uniformeF(0, 1) < probabilidad_spike * dt * 10) {
            if (!g.nodos.empty() && estado.flujos.size() < 6) {
                int intentos = 0;
                while (intentos++ < 20) {
                    int src = g.nodos[uniformeF(0, (float)g.nodos.size() - 1)].id;
                    int dst = g.nodos[uniformeF(0, (float)g.nodos.size() - 1)].id;
                    if (src != dst &&
                        estado.nodos.count(src) && estado.nodos[src].activo &&
                        estado.nodos.count(dst) && estado.nodos[dst].activo) {
                        auto ruta = obtenerRuta(g, src, dst);
                        if (ruta.size() >= 2) {
                            float mbps_spike = 100 + uniformeF(0, 400);
                            std::string tipos[] = {"HTTP", "VIDEO", "PING", "DDOS"};
                            std::string tipo = tipos[(int)uniformeF(0, 3.99f)];
                            float dur = 3.0f + uniformeF(0, 7.0f);
                            enviarFlujo(src, dst, mbps_spike, tipo, dur, g);
                            break;
                        }
                    }
                }
            }
        }

        if (uniformeF(0, 1) < 0.001f * dt * 20) {
            if (!g.aristas.empty()) {
                int idx = (int)uniformeF(0, (float)g.aristas.size() - 1);
                const auto& a = g.aristas[idx];
                simularFalloArista(a.origen_id, a.destino_id, g);
                microcortes_pendientes.push_back({
                    (int)a.origen_id, (int)a.destino_id,
                    estado.tiempo + 2.0f + uniformeF(0, 3.0f)
                });
            }
        }

        for (auto it = microcortes_pendientes.begin(); it != microcortes_pendientes.end(); ) {
            if (estado.tiempo >= it->tiempo_restauracion) {
                restaurarArista(it->origen, it->destino, g);
                it = microcortes_pendientes.erase(it);
            } else {
                ++it;
            }
        }
    }

    struct MicroCorte {
        int origen, destino;
        float tiempo_restauracion;
    };
    std::vector<MicroCorte> microcortes_pendientes;
};
