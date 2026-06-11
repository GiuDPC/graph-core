#pragma once

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <string>

#include "../Grafo.h"
#include "../tipos/PasoAnimacion.h"
#include "../tipos/TipoHardware.h"

// Algoritmo de dijkstra
namespace Algoritmos {

struct ResultadoDijkstra {
    std::vector<int>   ruta;           // secuencia de IDs de nodos
    std::vector<float> distancias;     // dist[id] = distancia minima desde origen
    bool               hay_ruta = false;
    float              costo_total = 0.0f;
    int                saltos = 0;
};

ResultadoDijkstra dijkstra(const Grafo& g, int id_origen, int id_destino,
                            bool aplicar_latencia = false) {
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

            float costo = a.peso_actual;
            if (aplicar_latencia) {
                const Nodo* nv = g.obtenerNodo(v);
                if (nv) costo += latenciaHardware(nv->tipo);
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

    // Reconstruir ruta
    for (int at = id_destino; at != -1; at = prev[at])
        resultado.ruta.push_back(at);
    std::reverse(resultado.ruta.begin(), resultado.ruta.end());

    resultado.hay_ruta    = true;
    resultado.costo_total = dist[id_destino];
    resultado.saltos      = (int)resultado.ruta.size() - 1;
    return resultado;
}

// Genera los pasos para animacion
std::vector<PasoAnimacion> generarPasos(const Grafo& g, int id_origen, int id_destino,
                                         bool aplicar_latencia = false) {
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
        "Inicio en " + nombre(id_origen) + " (dist=0)"});

    while (!cola.empty()) {
        auto [d, u] = cola.top(); cola.pop();
        if (d > dist[u]) continue;

        pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
            "Confirmado " + nombre(u) + " (dist=" + std::to_string((int)dist[u]) + ")"});

        if (u == id_destino) break;

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1 || v >= rango) continue;

            float costo = a.peso_actual;
            if (aplicar_latencia) {
                const Nodo* nv = g.obtenerNodo(v);
                if (nv) costo += latenciaHardware(nv->tipo);
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
                    "Mejorado " + nombre(v) + " (dist=" + std::to_string((int)nd) + ")"});
            }
        }
    }

    // Resaltar ruta final
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

} // namespace Algoritmos
