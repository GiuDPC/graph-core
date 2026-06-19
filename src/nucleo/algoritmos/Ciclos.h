#pragma once

#include <vector>
#include <string>
#include <functional>
#include "../Grafo.h"
#include "../UnionFind.h"

namespace Algoritmos {

struct ResultadoCiclos {
    bool                            tiene_ciclo = false;
    std::vector<std::pair<int,int>> aristas_ciclo;  // aristas que forman parte del ciclo
    std::string                     descripcion;
};

// detecta ciclos en grafos no dirigidos usando unionfind legacy
ResultadoCiclos detectarCiclos(const Grafo& g) {
    ResultadoCiclos resultado;
    if (g.estaVacio()) return resultado;

    UnionFind uf(g.rangoIds());
    for (const auto& a : g.aristas) {
        if (!uf.unir(a.origen_id, a.destino_id)) {
            resultado.tiene_ciclo = true;
            resultado.aristas_ciclo.push_back({a.origen_id, a.destino_id});
            resultado.descripcion =
                "Ciclo detectado en arista " + g.nombreNodo(a.origen_id) +
                " - " + g.nombreNodo(a.destino_id);
        }
    }
    if (!resultado.tiene_ciclo) resultado.descripcion = "El grafo es aciclico (es un bosque o arbol)";
    return resultado;
}

// detecta ciclos con dfs 3 colores si toca gris hay ciclo
ResultadoCiclos detectarCiclosDirigidos(const Grafo& g) {
    ResultadoCiclos resultado;
    if (g.estaVacio()) return resultado;

    const int WHITE = 0;
    const int GRAY  = 1;
    const int BLACK = 2;

    int rango = g.rangoIds();
    std::vector<int> color(rango, WHITE);

    std::vector<std::pair<int,int>> pila_aristas;

    std::function<bool(int)> dfs = [&](int u) -> bool {
        color[u] = GRAY;
        for (const auto& a : g.aristas) {
            // sigue direccion correcta origen destino o ambas
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1 || v >= rango) continue;

            pila_aristas.push_back({a.origen_id, a.destino_id});

            if (color[v] == GRAY) {
                resultado.tiene_ciclo = true;
                resultado.aristas_ciclo.push_back({a.origen_id, a.destino_id});
                resultado.descripcion = "Ciclo dirigido detectado";
                return true;
            }

            if (color[v] == WHITE) {
                if (dfs(v)) return true;
            }

            pila_aristas.pop_back();
        }
        color[u] = BLACK;
        return false;
    };

    for (int i = 0; i < rango; i++) {
        if (color[i] == WHITE && g.obtenerNodo(i)) {
            if (dfs(i)) break;
        }
    }

    if (!resultado.tiene_ciclo) {
        resultado.descripcion = "El digrafo es aciclico (DAG)";
    }

    return resultado;
}

} 
