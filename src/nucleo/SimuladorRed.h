#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <deque>
#include "Grafo.h"
#include "tipos/EstadoRed.h"
#include "algoritmos/Dijkstra.h"
#include "audio/Sonidos.h"

extern Sonidos g_sonidos;

// Motor de simulacion de red
// Mejoras: colas de paquetes, eventos automáticos, ruteo cacheado,
//           CPU/RAM reactiva, jitter con efecto real
class SimuladorRed {
public:
    EstadoSimulacion estado;

    // Configurable
    bool eventos_automaticos = true;
    float probabilidad_evento = 0.005f;  // por frame
    float probabilidad_spike  = 0.002f;

    void inicializar(const Grafo& g) {
        estado = {};
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
            ea.activa         = true;
            estado.aristas[key] = ea;
        }
        estado.activa = true;
        estado.tiempo = 0.0f;
        cache_rutas.clear();
        timer_eventos = 0.0f;
    }

    void tick(Grafo& g, float dt) {
        if (!estado.activa) return;
        float dt_sim = dt * estado.velocidad;
        estado.tiempo += dt_sim;

        actualizarNodos(g, dt_sim);
        actualizarAristas(g, dt_sim);
        procesarColas(g, dt_sim);

        if (eventos_automaticos) {
            generarEventos(g, dt_sim);
        }
    }

    // --- Fallos / Restauraciones ---

    void simularFalloNodo(int nodo_id, Grafo& g) {
        if (!estado.nodos.count(nodo_id)) return;
        estado.nodos[nodo_id].activo = false;
        estado.nodos[nodo_id].tiempo_caida = 0.0f;
        estado.registrarEvento(estado.tiempo,
            g.nombreNodo(nodo_id) + " CAIDO — recalculando rutas...",
            EventoRed::ERROR_RED);
        cache_rutas.clear(); // invalidar cache de rutas
        g_sonidos.reproducir(Sonidos::NODO_CAIDO);
    }

    void simularFalloArista(int origen, int destino, Grafo& g) {
        auto key  = std::make_pair(origen, destino);
        auto key2 = std::make_pair(destino, origen);
        if (estado.aristas.count(key))  estado.aristas[key].activa  = false;
        if (estado.aristas.count(key2)) estado.aristas[key2].activa = false;
        estado.registrarEvento(estado.tiempo,
            "Enlace " + g.nombreNodo(origen) + " <-> " + g.nombreNodo(destino) + " CAIDO",
            EventoRed::ERROR_RED);
        cache_rutas.clear();
        g_sonidos.reproducir(Sonidos::NODO_CAIDO);
    }

    void restaurarNodo(int nodo_id, Grafo& g) {
        if (!estado.nodos.count(nodo_id)) return;
        estado.nodos[nodo_id].activo = true;
        estado.registrarEvento(estado.tiempo,
            g.nombreNodo(nodo_id) + " restaurado", EventoRed::INFO);
        cache_rutas.clear();
        g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
    }

    void restaurarArista(int origen, int destino, Grafo& g) {
        auto key  = std::make_pair(origen, destino);
        auto key2 = std::make_pair(destino, origen);
        if (estado.aristas.count(key))  estado.aristas[key].activa  = true;
        if (estado.aristas.count(key2)) estado.aristas[key2].activa = true;
        estado.registrarEvento(estado.tiempo,
            "Enlace " + g.nombreNodo(origen) + " <-> " + g.nombreNodo(destino) + " restaurado",
            EventoRed::INFO);
        cache_rutas.clear();
    }

    // Paquete individual en transito (exposed para renderizado)
    struct Paquete {
        int origen_id;
        int destino_id;
        float mbps;
        std::string tipo;
        float progreso = 0.0f;
        float tamaño_mb;
        int paso_actual = 0;
        std::vector<int> ruta;
    };

    // Exponer paquetes activos para renderizado
    const std::vector<Paquete>& obtenerPaquetes() const { return paquetes_activos; }

    void enviarFlujo(int origen, int destino, float mbps,
                     const std::string& tipo, float duracion, Grafo& g) {
        FlujoTrafico f;
        f.origen_id       = origen;
        f.destino_id      = destino;
        f.mbps            = mbps;
        f.tipo            = tipo;
        f.tiempo_restante = duracion;
        estado.flujos.push_back(f);
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

    // Perdida de paquetes simulada en una arista (0.0 - 1.0)
    float perdidaEnArista(int origen, int destino) const {
        auto key = std::make_pair(origen, destino);
        if (!estado.aristas.count(key)) return 0.0f;
        const auto& ea = estado.aristas.at(key);
        return ea.packet_loss;
    }

    ImU32 colorUsoArista(float uso) const {
        if (uso < 0.5f) {
            float t = uso * 2.0f;
            return IM_COL32((int)(t * 255), 200, 0, 220);
        } else {
            float t = (uso - 0.5f) * 2.0f;
            return IM_COL32(255, (int)((1.0f - t) * 200), 0, 220);
        }
    }

private:
    std::mt19937 gen{42};

    // Cache de rutas: clave "origen:destino" -> ruta
    std::unordered_map<std::string, std::vector<int>> cache_rutas;

    // Temporizadores
    float timer_eventos = 0.0f;
    float timer_paquetes = 0.0f; // generacion de paquetes visuales

    float uniformeF(float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(gen);
    }

    // Obtener o calcular ruta cacheada
    std::vector<int> obtenerRuta(const Grafo& g, int origen, int destino) {
        std::string key = std::to_string(origen) + ":" + std::to_string(destino);
        auto it = cache_rutas.find(key);
        if (it != cache_rutas.end()) return it->second;

        auto res = Algoritmos::dijkstra(g, origen, destino);
        if (res.hay_ruta) {
            cache_rutas[key] = res.ruta;
            return res.ruta;
        }
        return {};
    }

    void actualizarNodos(Grafo& g, float dt) {
        for (const auto& n : g.nodos) {
            if (!estado.nodos.count(n.id)) continue;
            auto& en = estado.nodos[n.id];
            if (!en.activo) {
                en.tiempo_caida += dt;
                continue;
            }

            // CPU: base 5-15% + actividad de ruteo (depende de flujos que pasan)
            float cpu_base = 0.05f + sinf(en.uptime * 0.1f) * 0.03f;
            float cpu_ruteo = 0.0f;

            // Cuantos flujos pasan por este nodo?
            int flujos_pasan = 0;
            for (const auto& f : estado.flujos) {
                auto ruta = obtenerRuta(g, f.origen_id, f.destino_id);
                for (size_t i = 0; i < ruta.size(); i++) {
                    if (ruta[i] == n.id) { flujos_pasan++; break; }
                }
            }
            cpu_ruteo = flujos_pasan * 0.05f;

            // RAM: base 10-25% + almacenamiento de estado
            float ram_base = 0.10f + flujos_pasan * 0.02f;

            // Transicion suave
            en.cpu_uso     = std::clamp(en.cpu_uso * 0.9f + (cpu_base + cpu_ruteo) * 0.1f, 0.02f, 0.99f);
            en.memoria_uso = std::clamp(en.memoria_uso * 0.9f + ram_base * 0.1f, 0.05f, 0.99f);

            // Si CPU o RAM pasan 80%, generar evento de sobrecarga
            if (en.cpu_uso > 0.80f && uniformeF(0, 1) < 0.01f) {
                estado.registrarEvento(estado.tiempo,
                    g.nombreNodo(n.id) + " sobrecargado (CPU:" + std::to_string((int)(en.cpu_uso*100)) + "%)",
                    EventoRed::ADVERTENCIA);
                g_sonidos.reproducir(Sonidos::ARISTA_SATURADA);
            }
        }
    }

    void actualizarAristas(Grafo& g, float dt) {
        for (auto& [key, ea] : estado.aristas) {
            if (!ea.activa) continue;
            // Decaimiento natural del uso
            ea.uso_actual_mbps *= std::pow(0.90f, dt);
            if (ea.uso_actual_mbps < 0.01f) ea.uso_actual_mbps = 0.0f;

            // Jitter basado en uso
            float uso_normalizado = std::min(1.0f, ea.uso_actual_mbps / ea.bandwidth_mbps);
            ea.jitter_ms = uso_normalizado * ea.latencia_ms * 0.3f;

            // Packet loss basado en congestion
            if (uso_normalizado > 0.9f) {
                ea.packet_loss = (uso_normalizado - 0.9f) * 0.5f; // 0-5% loss
            } else {
                ea.packet_loss = 0.0f;
            }
        }
    }

    std::vector<Paquete> paquetes_activos;

    void procesarColas(Grafo& g, float dt) {
        // 1. Avanzar temporizador global de paquetes
        timer_paquetes += dt;

        // 2. Procesar flujos: crear paquetes periodicos para CADA flujo
        for (auto it = estado.flujos.begin(); it != estado.flujos.end(); ) {
            it->tiempo_restante -= dt;
            if (it->tiempo_restante <= 0) {
                it = estado.flujos.erase(it);
                continue;
            }

            // Encontrar ruta
            auto ruta = obtenerRuta(g, it->origen_id, it->destino_id);
            if (ruta.size() < 2) { ++it; continue; }

            // Crear paquete cada ~100ms para cada flujo
            float intervalo = 0.1f;
            if (timer_paquetes >= intervalo) {
                Paquete p;
                p.origen_id   = it->origen_id;
                p.destino_id  = it->destino_id;
                p.mbps        = it->mbps;
                p.tipo        = it->tipo;
                p.ruta        = ruta;
                p.paso_actual = 0;
                // Tamaño visual acotado: 0.5-4.0 MB segun el flujo
                p.tamaño_mb   = 0.5f + std::min(it->mbps * 0.01f, 3.5f);
                paquetes_activos.push_back(p);
            }
            ++it;
        }

        // Reset timer global (solo una vez por tick)
        if (timer_paquetes >= 0.1f) timer_paquetes = 0.0f;

        // 2. Avanzar paquetes activos
        for (auto it = paquetes_activos.begin(); it != paquetes_activos.end(); ) {
            if (it->paso_actual + 1 >= (int)it->ruta.size()) {
                // Llegó a destino!
                int dst = it->destino_id;
                if (estado.nodos.count(dst)) {
                    estado.nodos[dst].paquetes_rx++;
                }
                it = paquetes_activos.erase(it);
                continue;
            }

            int u = it->ruta[it->paso_actual];
            int v = it->ruta[it->paso_actual + 1];
            auto key = std::make_pair(u, v);

            // Avanzar en la arista
            float velocidad = 1.0f; // segundos para cruzar
            if (estado.aristas.count(key)) {
                const auto& ea = estado.aristas.at(key);
                if (!ea.activa) {
                    // Arista caída -> paquete perdido
                    it = paquetes_activos.erase(it);
                    continue;
                }
                velocidad = ea.latencia_ms / 1000.0f + ea.jitter_ms / 1000.0f;

                // Packet loss?
                if (uniformeF(0, 1) < ea.packet_loss) {
                    estado.registrarEvento(estado.tiempo,
                        "Paquete perdido en " + g.nombreNodo(u) + " -> " + g.nombreNodo(v) +
                        " (congestion " + std::to_string((int)(ea.uso_actual_mbps / ea.bandwidth_mbps * 100)) + "%)",
                        EventoRed::ADVERTENCIA);
                    it = paquetes_activos.erase(it);
                    continue;
                }
            }

            it->progreso += dt / std::max(velocidad, 0.01f);
            if (it->progreso >= 1.0f) {
                it->progreso = 0.0f;
                it->paso_actual++;
                if (estado.nodos.count(v)) {
                    estado.nodos[v].paquetes_rx++;
                }
                if (estado.nodos.count(u)) {
                    estado.nodos[u].paquetes_tx++;
                }
            }
            ++it;
        }
    }

    void generarEventos(Grafo& g, float dt) {
        timer_eventos += dt;

        // Evento aleatorio: spike de trafico (cada ~5-15s)
        if (uniformeF(0, 1) < probabilidad_spike * dt * 10) {
            if (!g.nodos.empty() && estado.flujos.size() < 6) {
                int intentos = 0;
                while (intentos++ < 20) {
                    int src = g.nodos[uniformeF(0, (float)g.nodos.size() - 1)].id;
                    int dst = g.nodos[uniformeF(0, (float)g.nodos.size() - 1)].id;
                    if (src != dst && estado.nodos[src].activo && estado.nodos[dst].activo) {
                        auto ruta = obtenerRuta(g, src, dst);
                        if (ruta.size() >= 2) {
                            float mbps_spike = 100 + uniformeF(0, 400);
                            std::string tipos[] = {"HTTP", "VIDEO", "PING", "DDOS"};
                            std::string tipo = tipos[(int)uniformeF(0, 3.99f)];
                            float dur = 3.0f + uniformeF(0, 7.0f);
                            enviarFlujo(src, dst, mbps_spike, tipo, dur, g);
                            estado.registrarEvento(estado.tiempo,
                                "[AUTO] Spike de trafico: " + tipo + " " + std::to_string((int)mbps_spike) + " Mbps",
                                EventoRed::ADVERTENCIA);
                            break;
                        }
                    }
                }
            }
        }

        // Evento aleatorio: microcorte (cada ~15-30s)
        if (uniformeF(0, 1) < 0.001f * dt * 20) {
            if (!g.aristas.empty()) {
                int idx = (int)uniformeF(0, (float)g.aristas.size() - 1);
                const auto& a = g.aristas[idx];
                simularFalloArista(a.origen_id, a.destino_id, g);
                estado.registrarEvento(estado.tiempo,
                    "[AUTO] Microcorte en enlace " + g.nombreNodo(a.origen_id) + " <-> " + g.nombreNodo(a.destino_id),
                    EventoRed::ERROR_RED);
                // Programar restauración automática después de unos segundos
                microcortes_pendientes.push_back({
                    (int)a.origen_id, (int)a.destino_id,
                    estado.tiempo + 2.0f + uniformeF(0, 3.0f)
                });
            }
        }

        // Restaurar microcortes
        for (auto it = microcortes_pendientes.begin(); it != microcortes_pendientes.end(); ) {
            if (estado.tiempo >= it->tiempo_restauracion) {
                restaurarArista(it->origen, it->destino, g);
                estado.registrarEvento(estado.tiempo,
                    "[AUTO] Enlace " + g.nombreNodo(it->origen) + " <-> " + g.nombreNodo(it->destino) + " restaurado",
                    EventoRed::INFO);
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
