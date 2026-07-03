#include "Dijkstra.hpp"
#include "../tipos/TipoHardware.hpp"
#include <queue>
#include <limits>
#include <algorithm>

namespace Algoritmos {

ResultadoDijkstra dijkstra(const Grafo& g, int id_origen, int id_destino,
                           bool aplicar_latencia,
                           bool aplicar_escala,
                           std::function<bool(int)> nodo_valido,
                           std::function<bool(int, int)> arista_valida) {
    ResultadoDijkstra resultado;
    if (id_origen == id_destino || g.estaVacio()) return resultado;

    const float INF = std::numeric_limits<float>::infinity();
    int rango = g.rangoIds();
    std::vector<float> dist(rango, INF);
    std::vector<int>   prev(rango, -1);
    dist[id_origen] = 0.0f;

    using Par = std::pair<float, int>;
    std::priority_queue<Par, std::vector<Par>, std::greater<Par>> cola;
    cola.push({0.0f, id_origen});

    while (!cola.empty()) {
        auto [d, u] = cola.top(); cola.pop();
        if (u == id_destino) break;
        if (d > dist[u]) continue;

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1 || v >= rango) continue;

            if (nodo_valido && !nodo_valido(v)) continue;
            if (arista_valida && !arista_valida(u, v)) continue;

            const float COSTO_ESCALA_KM = 1500.0f;
            float costo = a.peso_actual;
            if (aplicar_latencia) {
                const Nodo* nv = g.obtenerNodo(v);
                if (nv) costo += latenciaHardware(nv->tipo);
            }
            if (aplicar_escala && u != id_origen) {
                costo += COSTO_ESCALA_KM;
            }
            float nd = d + costo;
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                cola.push({nd, v});
            }
        }
    }

    resultado.distancias = dist;
    if (dist[id_destino] == INF) return resultado;

    for (int at = id_destino; at != -1; at = prev[at])
        resultado.ruta.push_back(at);
    std::reverse(resultado.ruta.begin(), resultado.ruta.end());

    resultado.hay_ruta    = true;
    resultado.costo_total = dist[id_destino];
    resultado.saltos      = (int)resultado.ruta.size() - 1;
    return resultado;
}

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int id_origen, int id_destino,
                                         bool aplicar_latencia,
                                         bool aplicar_escala,
                                         std::function<bool(int)> nodo_valido,
                                         std::function<bool(int, int)> arista_valida) {
    std::vector<PasoAnimacion> pasos;
    if (id_origen == id_destino || g.estaVacio()) return pasos;

    const float INF = std::numeric_limits<float>::infinity();
    int rango = g.rangoIds();
    std::vector<float> dist(rango, INF);
    std::vector<int>   prev(rango, -1);
    dist[id_origen] = 0.0f;

    auto nombre = [&](int id) { return g.nombreNodo(id); };

    using Par = std::pair<float, int>;
    std::priority_queue<Par, std::vector<Par>, std::greater<Par>> cola;
    cola.push({0.0f, id_origen});

    pasos.push_back({PasoAnimacion::VISITAR, id_origen, -1, -1,
        "Inicio en " + nombre(id_origen) + " (dist=0)", -1, 0.0f});

    while (!cola.empty()) {
        auto [d, u] = cola.top(); cola.pop();
        if (d > dist[u]) continue;

        pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
            "Confirmado " + nombre(u) + " (dist=" + std::to_string((int)dist[u]) + ")", -1, dist[u]});

        if (u == id_destino) break;

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1 || v >= rango) continue;

            if (nodo_valido && !nodo_valido(v)) continue;
            if (arista_valida && !arista_valida(u, v)) continue;

            const float COSTO_ESCALA_KM = 1500.0f;
            float costo = a.peso_actual;
            if (aplicar_latencia) {
                const Nodo* nv = g.obtenerNodo(v);
                if (nv) costo += latenciaHardware(nv->tipo);
            }
            if (aplicar_escala && u != id_origen) {
                costo += COSTO_ESCALA_KM;
            }
            pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
                "Evaluando " + nombre(u) + " -> " + nombre(v) +
                " (costo=" + std::to_string((int)costo) + ")"});

            float nd = d + costo;
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                cola.push({nd, v});
                pasos.push_back({PasoAnimacion::VISITAR, v, -1, -1,
                    "Mejorado " + nombre(v) + " (dist=" + std::to_string((int)nd) + ")", -1, nd});
            }
        }
    }

    if (dist[id_destino] < INF) {
        std::vector<int> ruta;
        for (int at = id_destino; at != -1; at = prev[at]) ruta.push_back(at);
        std::reverse(ruta.begin(), ruta.end());
        for (size_t i = 0; i + 1 < ruta.size(); i++) {
            pasos.push_back({PasoAnimacion::CONFIRMAR, -1, ruta[i], ruta[i + 1],
                "Ruta optima: " + nombre(ruta[i]) + " -> " + nombre(ruta[i + 1])});
        }
    }
    return pasos;
}

}
