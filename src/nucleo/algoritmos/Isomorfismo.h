#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include "../Grafo.h"

namespace Algoritmos {
namespace Isomorfismo {

struct ResultadoIsomorfismo {
    bool                            son_isomorfos = false;
    std::vector<std::pair<int,int>> mapeo;          // (id_g1, id_g2)
    std::string                     descripcion;
    // Condiciones de verificacion rapida
    bool misma_cantidad_nodos   = false;
    bool misma_cantidad_aristas = false;
    bool misma_secuencia_grados = false;
};

// Genera la secuencia de grados ordenada de mayor a menor
std::vector<int> secuenciaGrados(const Grafo& g) {
    std::vector<int> grados;
    for (const auto& n : g.nodos)
        grados.push_back(g.gradoNodo(n.id));
    std::sort(grados.rbegin(), grados.rend());
    return grados;
}

// Backtracking para encontrar el mapeo isomorfo
bool backtrack(const Grafo& g1, const Grafo& g2,
               std::vector<int>& mapeo,          // mapeo[i] = id en g2 para nodo i de g1
               std::vector<bool>& usado_g2,
               int idx)
{
    if (idx == (int)g1.nodos.size()) return true;

    int u1 = g1.nodos[idx].id;
    for (int j = 0; j < (int)g2.nodos.size(); j++) {
        if (usado_g2[j]) continue;
        int u2 = g2.nodos[j].id;

        // Verificar consistencia: para cada arista (u1, v1) ya mapeada,
        // debe existir (u2, mapeo[v1]) en g2
        bool consistente = true;
        for (int k = 0; k < idx && consistente; k++) {
            int v1 = g1.nodos[k].id;
            int v2_mapped = mapeo[k];
            bool hay_arista_g1 = (g1.obtenerArista(u1, v1) != nullptr ||
                                   g1.obtenerArista(v1, u1) != nullptr);
            bool hay_arista_g2 = (g2.obtenerArista(u2, v2_mapped) != nullptr ||
                                   g2.obtenerArista(v2_mapped, u2) != nullptr);
            if (hay_arista_g1 != hay_arista_g2) consistente = false;
        }

        if (consistente) {
            mapeo[idx] = u2;
            usado_g2[j] = true;
            if (backtrack(g1, g2, mapeo, usado_g2, idx + 1)) return true;
            mapeo[idx] = -1;
            usado_g2[j] = false;
        }
    }
    return false;
}

ResultadoIsomorfismo verificar(const Grafo& g1, const Grafo& g2) {
    ResultadoIsomorfismo resultado;

    // Condicion 1: mismo numero de nodos
    resultado.misma_cantidad_nodos = (g1.nodos.size() == g2.nodos.size());
    if (!resultado.misma_cantidad_nodos) {
        resultado.descripcion = "Distinto numero de nodos (" +
            std::to_string(g1.nodos.size()) + " vs " + std::to_string(g2.nodos.size()) + ")";
        return resultado;
    }

    // Condicion 2: mismo numero de aristas
    resultado.misma_cantidad_aristas = (g1.aristas.size() == g2.aristas.size());
    if (!resultado.misma_cantidad_aristas) {
        resultado.descripcion = "Distinto numero de aristas (" +
            std::to_string(g1.aristas.size()) + " vs " + std::to_string(g2.aristas.size()) + ")";
        return resultado;
    }

    // Condicion 3: misma secuencia de grados
    auto deg1 = secuenciaGrados(g1);
    auto deg2 = secuenciaGrados(g2);
    resultado.misma_secuencia_grados = (deg1 == deg2);
    if (!resultado.misma_secuencia_grados) {
        resultado.descripcion = "Distinta secuencia de grados";
        return resultado;
    }

    // Backtracking para encontrar el mapeo
    int n = (int)g1.nodos.size();
    std::vector<int>  mapeo_ids(n, -1);
    std::vector<bool> usado(n, false);

    if (backtrack(g1, g2, mapeo_ids, usado, 0)) {
        resultado.son_isomorfos = true;
        for (int i = 0; i < n; i++) {
            resultado.mapeo.push_back({g1.nodos[i].id, mapeo_ids[i]});
        }
        resultado.descripcion = "Los grafos son isomorfos. Se encontro el mapeo de nodos.";
    } else {
        resultado.descripcion = "Las condiciones necesarias se cumplen pero no existe mapeo valido. "
                                "Los grafos NO son isomorfos.";
    }
    return resultado;
}

} // namespace Isomorfismo
} // namespace Algoritmos
