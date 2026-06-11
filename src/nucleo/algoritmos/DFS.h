#pragma once

#include <vector>
#include <string>

#include "../Grafo.h"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {
namespace DFS {

struct ResultadoDFS {
    std::vector<int>                       orden_visita;
    std::vector<std::pair<int,int>>        back_edges;    // aristas que crean ciclos
};

void dfsHelper(const Grafo& g, int u, int padre, std::vector<bool>& vis, ResultadoDFS& resultado) {
    vis[u] = true;
    resultado.orden_visita.push_back(u);

    for (const auto& a : g.aristas) {
        int v = -1;
        if (a.origen_id == u) v = a.destino_id;
        else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
        if (v == -1 || v >= g.rangoIds() || v == padre) continue; // NO marcar vuelta al padre como back-edge

        if (!vis[v]) {
            dfsHelper(g, v, u, vis, resultado);
        } else {
            resultado.back_edges.push_back({u, v}); // arista de vuelta = ciclo genuino
        }
    }
}

ResultadoDFS dfs(const Grafo& g, int inicio_id) {
    ResultadoDFS resultado;
    if (g.estaVacio() || !g.obtenerNodo(inicio_id)) return resultado;

    std::vector<bool> vis(g.rangoIds(), false);
    dfsHelper(g, inicio_id, -1, vis, resultado);
    return resultado;
}

void dfsPasosHelper(const Grafo& g, int u, int padre, std::vector<bool>& vis,
                     std::vector<PasoAnimacion>& pasos) {
    vis[u] = true;
    pasos.push_back({PasoAnimacion::VISITAR, u, -1, -1,
        "Visitando " + g.nombreNodo(u)});

    for (const auto& a : g.aristas) {
        int v = -1;
        if (a.origen_id == u) v = a.destino_id;
        else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
        if (v == -1 || v >= g.rangoIds() || v == padre) continue;

        pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
            "Explorando " + g.nombreNodo(u) + " -> " + g.nombreNodo(v)});

        if (!vis[v]) {
            dfsPasosHelper(g, v, u, vis, pasos);
        } else {
            pasos.push_back({PasoAnimacion::DESCARTAR, -1, u, v,
                "Back-edge: " + g.nombreNodo(v) + " ya fue visitado (posible ciclo)"});
        }
    }
    pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
        "Completado " + g.nombreNodo(u)});
}

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int inicio_id) {
    std::vector<PasoAnimacion> pasos;
    if (g.estaVacio() || !g.obtenerNodo(inicio_id)) return pasos;
    std::vector<bool> vis(g.rangoIds(), false);
    dfsPasosHelper(g, inicio_id, -1, vis, pasos);
    return pasos;
}

} // namespace DFS
} // namespace Algoritmos
