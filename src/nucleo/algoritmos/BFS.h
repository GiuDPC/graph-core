#pragma once

#include <vector>
#include <queue>
#include <map>
#include <string>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {
namespace BFS {

struct ResultadoBFS {
    std::vector<int>    orden_visita;
    std::map<int, int>  nivel;     
    std::map<int, int>  padre;     
};

ResultadoBFS bfs(const Grafo& g, int inicio_id) {
    ResultadoBFS resultado;
    if (g.estaVacio() || !g.obtenerNodo(inicio_id)) return resultado;

    std::vector<bool> visitado(g.rangoIds(), false);
    std::queue<int> cola;
    visitado[inicio_id] = true;
    cola.push(inicio_id);
    resultado.nivel[inicio_id]  = 0;
    resultado.padre[inicio_id]  = -1;

    while (!cola.empty()) {
        int u = cola.front(); cola.pop();
        resultado.orden_visita.push_back(u);

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1 || v >= g.rangoIds() || visitado[v]) continue;

            visitado[v] = true;
            cola.push(v);
            resultado.nivel[v]  = resultado.nivel[u] + 1;
            resultado.padre[v]  = u;
        }
    }
    return resultado;
}

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int inicio_id) {
    std::vector<PasoAnimacion> pasos;
    if (g.estaVacio() || !g.obtenerNodo(inicio_id)) return pasos;

    std::vector<bool> visitado(g.rangoIds(), false);
    std::queue<int> cola;
    visitado[inicio_id] = true;
    cola.push(inicio_id);

    pasos.push_back({PasoAnimacion::VISITAR, inicio_id, -1, -1,
        "Inicio BFS en " + g.nombreNodo(inicio_id) + " (nivel 0)", -1, -1.0f, 0});

    std::map<int, int> nivel;
    nivel[inicio_id] = 0;

    while (!cola.empty()) {
        int u = cola.front(); cola.pop();
        pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
            "Procesando " + g.nombreNodo(u) + " (nivel " + std::to_string(nivel[u]) + ")", -1, -1.0f, nivel[u]});

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1) continue;

            pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
                "Explorando " + g.nombreNodo(u) + " -> " + g.nombreNodo(v)});

            if (v < g.rangoIds() && !visitado[v]) {
                visitado[v] = true;
                cola.push(v);
                nivel[v] = nivel[u] + 1;
                pasos.push_back({PasoAnimacion::VISITAR, v, -1, -1,
                    "Descubierto " + g.nombreNodo(v) + " (nivel " + std::to_string(nivel[v]) + ")", -1, -1.0f, nivel[v]});
            }
        }
    }
    return pasos;
}

}
} 
