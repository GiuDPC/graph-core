#include "Kruskal.hpp"
#include "../UnionFind.hpp"
#include <algorithm>
// kruskal: ordenar E por peso O(E log E), union-find O(E a(V))
// mst: V-1 aristas de peso total minimo, avaro O(E log V)

namespace Algoritmos {
namespace Kruskal {

ResultadoKruskal kruskal(const Grafo& g) {
    ResultadoKruskal resultado;
    if (g.estaVacio()) return resultado;

    std::vector<Arista> ordenadas = g.aristas;
    std::sort(ordenadas.begin(), ordenadas.end(),
        [](const Arista& a, const Arista& b) { return a.peso < b.peso; });

    UnionFind uf(g.rangoIds());
    for (const auto& a : ordenadas) {
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
        pasos.push_back({PasoAnimacion::EXPLORAR, -1, a.origen_id, a.destino_id,
            "Evaluando ruta más corta disponible: " + g.nombreNodo(a.origen_id) + " -> " + g.nombreNodo(a.destino_id) +
            " (" + std::to_string((int)a.peso) + " km)", -1, a.peso, -1});

        if (uf.unir(a.origen_id, a.destino_id)) {
            pasos.push_back({PasoAnimacion::CONFIRMAR, -1, a.origen_id, a.destino_id,
                "Aceptada: Conecta nuevas áreas al esqueleto principal", -1, a.peso, -1});
            pasos.push_back({PasoAnimacion::CONFIRMAR, a.origen_id, -1, -1, "", -1, -1.0f, -1});
            pasos.push_back({PasoAnimacion::CONFIRMAR, a.destino_id, -1, -1, "", -1, -1.0f, -1});
            aceptadas++;
            if (aceptadas == (int)g.nodos.size() - 1) break;
        } else {
            pasos.push_back({PasoAnimacion::DESCARTAR, -1, a.origen_id, a.destino_id,
                "Rechazada: Formaría un ciclo redundante (ya están conectadas)", -1, a.peso, -1});
        }
    }
    return pasos;
}

}
}
