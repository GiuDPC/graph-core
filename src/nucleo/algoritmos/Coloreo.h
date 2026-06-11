#pragma once

#include <vector>
#include <string>
#include "../Grafo.h"

namespace Algoritmos {

struct ResultadoColoreo {
    std::vector<int> colores;        // colores[id_nodo] = color asignado
    int              num_colores;    // numero cromatico greedy
};

ResultadoColoreo coloreoGreedy(const Grafo& g) {
    ResultadoColoreo resultado;
    int rango = g.rangoIds();
    resultado.colores.assign(rango, -1);
    resultado.num_colores = 0;

    if (g.estaVacio()) return resultado;

    resultado.colores[g.nodos[0].id] = 0;
    resultado.num_colores = 1;

    for (size_t i = 1; i < g.nodos.size(); i++) {
        int u = g.nodos[i].id;
        std::vector<bool> usado(rango, false);

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v != -1 && v < rango && resultado.colores[v] != -1)
                usado[resultado.colores[v]] = true;
        }

        int c = 0;
        while (c < rango && usado[c]) c++;
        resultado.colores[u] = c;
        if (c + 1 > resultado.num_colores) resultado.num_colores = c + 1;
    }
    return resultado;
}

} // namespace Algoritmos
