#pragma once

#include <vector>
#include <string>
#include "../Grafo.h"
#include "../UnionFind.h"

namespace Algoritmos {

struct ResultadoCiclos {
    bool                            tiene_ciclo = false;
    std::vector<std::pair<int,int>> aristas_ciclo;  // aristas que forman parte del ciclo
    std::string                     descripcion;
};

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

} // namespace Algoritmos
