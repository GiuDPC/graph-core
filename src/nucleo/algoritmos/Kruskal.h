#pragma once

#include <vector>
#include <algorithm>
#include <string>

#include "../Grafo.h"
#include "../UnionFind.h"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {
namespace Kruskal { // Nested namespace to avoid generarPasos conflicts, actually the doc said to use namespace alias or nested namespaces

struct ResultadoKruskal {
    std::vector<Arista> aristas_mst;
    float               peso_total = 0.0f;
    int                 aristas_aceptadas = 0;
    int                 aristas_rechazadas = 0;
};

ResultadoKruskal kruskal(const Grafo& g) {
    ResultadoKruskal resultado;
    if (g.estaVacio()) return resultado;

    std::vector<Arista> ordenadas = g.aristas;
    std::sort(ordenadas.begin(), ordenadas.end(),
        [](const Arista& a, const Arista& b) { return a.peso < b.peso; });

    UnionFind uf(g.rangoIds());
    for (const auto& a : ordenadas) {
        // Kruskal solo funciona para grafos no-dirigidos (MST no tiene sentido en digrafos).
        // Las aristas dirigidas se omiten.
        if (a.es_dirigida) { resultado.aristas_rechazadas++; continue; }
        if (uf.unir(a.origen_id, a.destino_id)) {
            resultado.aristas_mst.push_back(a);
            resultado.peso_total += a.peso;
            resultado.aristas_aceptadas++;
            if ((int)resultado.aristas_mst.size() == (int)g.nodos.size() - 1) break;
        } else {
            resultado.aristas_rechazadas++;
        }
    }
    return resultado;
}

std::vector<PasoAnimacion> generarPasos(const Grafo& g) {
    std::vector<PasoAnimacion> pasos;
    if (g.estaVacio()) return pasos;

    std::vector<Arista> ordenadas = g.aristas;
    std::sort(ordenadas.begin(), ordenadas.end(),
        [](const Arista& a, const Arista& b) { return a.peso < b.peso; });

    UnionFind uf(g.rangoIds());
    int aceptadas = 0;

    for (const auto& a : ordenadas) {
        // Kruskal solo funciona para grafos no-dirigidos (MST no tiene sentido en digrafos).
        // Las aristas dirigidas se omiten.
        if (a.es_dirigida) continue;
        pasos.push_back({PasoAnimacion::EXPLORAR, -1, a.origen_id, a.destino_id,
            "Evaluando " + g.nombreNodo(a.origen_id) + " - " + g.nombreNodo(a.destino_id) +
            " (peso=" + std::to_string((int)a.peso) + ")"});

        if (uf.unir(a.origen_id, a.destino_id)) {
            pasos.push_back({PasoAnimacion::CONFIRMAR, -1, a.origen_id, a.destino_id,
                "Aceptada: conecta componentes distintos"});
            pasos.push_back({PasoAnimacion::CONFIRMAR, a.origen_id, -1, -1, ""});
            pasos.push_back({PasoAnimacion::CONFIRMAR, a.destino_id, -1, -1, ""});
            aceptadas++;
            if (aceptadas == (int)g.nodos.size() - 1) break;
        } else {
            pasos.push_back({PasoAnimacion::DESCARTAR, -1, a.origen_id, a.destino_id,
                "Rechazada: crearia ciclo"});
        }
    }
    return pasos;
}

} // namespace Kruskal
} // namespace Algoritmos
