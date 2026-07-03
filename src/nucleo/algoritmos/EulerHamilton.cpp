#include "EulerHamilton.hpp"
#include <stack>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
// euler: todos los vertices con grado par, hierholzer O(V+E)
// hamilton: backtrack exponencial O(V!), heuristica vecino mas cercano O(V^2)

namespace Algoritmos {

std::vector<int> EulerHamilton::buscarCaminoEuleriano(Grafo& g, int origen_sugerido) {
    std::vector<int> camino;
    if (g.nodos.empty()) return camino;

    std::vector<int> grados(g.rangoIds(), 0);
    for (const auto& arista : g.aristas) {
        grados[arista.origen_id]++;
        grados[arista.destino_id]++;
    }

    int impares = 0;
    int nodo_impar = -1;
    for (size_t i = 0; i < grados.size(); i++) {
        if (grados[i] % 2 != 0) {
            impares++;
            nodo_impar = i;
        }
    }

    if (impares != 0 && impares != 2) {
        return camino;
    }

    int start_node = origen_sugerido >= 0 ? origen_sugerido : 0;
    if (impares == 2) {
        if (grados[start_node] % 2 == 0) {
            start_node = nodo_impar;
        }
    }

    std::unordered_map<int, std::vector<int>> adj;
    for (const auto& a : g.aristas) {
        adj[a.origen_id].push_back(a.destino_id);
        adj[a.destino_id].push_back(a.origen_id);
    }

    std::stack<int> curr_path;
    curr_path.push(start_node);
    int curr_v = start_node;

    while (!curr_path.empty()) {
        if (!adj[curr_v].empty()) {
            curr_path.push(curr_v);
            int next_v = adj[curr_v].back();
            adj[curr_v].pop_back();

            auto it = std::find(adj[next_v].begin(), adj[next_v].end(), curr_v);
            if (it != adj[next_v].end()) adj[next_v].erase(it);

            curr_v = next_v;
        } else {
            camino.push_back(curr_v);
            curr_v = curr_path.top();
            curr_path.pop();
        }
    }

    std::reverse(camino.begin(), camino.end());
    return camino;
}

std::vector<PasoAnimacion> EulerHamilton::generarPasosEuler(Grafo& g, int origen_sugerido) {
    std::vector<PasoAnimacion> pasos;
    auto camino = buscarCaminoEuleriano(g, origen_sugerido);
    if (camino.empty()) return pasos;

    for (size_t i = 1; i < camino.size(); i++) {
        PasoAnimacion paso1;
        paso1.accion = PasoAnimacion::VISITAR;
        paso1.arista_origen = camino[i-1];
        paso1.arista_destino = camino[i];
        paso1.descripcion = "Viajando (" + std::to_string(i) + "/" + std::to_string(camino.size()-1) + "): " + g.nombreNodo(camino[i-1]) + " -> " + g.nombreNodo(camino[i]);
        pasos.push_back(paso1);

        PasoAnimacion paso2;
        paso2.accion = PasoAnimacion::CONFIRMAR;
        paso2.arista_origen = camino[i-1];
        paso2.arista_destino = camino[i];
        paso2.descripcion = "Camino confirmado";
        pasos.push_back(paso2);
    }
    return pasos;
}

static bool hamiltonianoRecursivo(int v, int num_nodos, const std::unordered_map<int, std::vector<int>>& adj,
                                   std::vector<bool>& visitado, std::vector<int>& camino) {
    camino.push_back(v);
    if (camino.size() == num_nodos) {
        return true;
    }

    visitado[v] = true;
    auto it = adj.find(v);
    if (it != adj.end()) {
        for (int u : it->second) {
            if (!visitado[u]) {
                if (hamiltonianoRecursivo(u, num_nodos, adj, visitado, camino)) {
                    return true;
                }
            }
        }
    }

    visitado[v] = false;
    camino.pop_back();
    return false;
}

std::vector<int> EulerHamilton::buscarCaminoHamiltoniano(const Grafo& g) {
    std::vector<int> camino;
    if (g.nodos.empty()) return camino;

    if (g.nodos.size() > 20) {
        return buscarCaminoHamiltonianoHeuristico(g).ruta;
    }

    std::unordered_map<int, std::vector<int>> adj;
    for (const auto& a : g.aristas) {
        adj[a.origen_id].push_back(a.destino_id);
        adj[a.destino_id].push_back(a.origen_id);
    }

    int num_nodos = g.nodos.size();
    std::vector<bool> visitado(g.rangoIds(), false);

    for (const auto& n : g.nodos) {
        if (hamiltonianoRecursivo(n.id, num_nodos, adj, visitado, camino)) {
            return camino;
        }
    }

    return {};
}

EulerHamilton::ResultadoHamilton EulerHamilton::buscarCaminoHamiltonianoHeuristico(const Grafo& g, int origen_sugerido) {
    ResultadoHamilton res;
    res.distancia_total = 0;
    if (g.nodos.empty()) return res;

    int actual = origen_sugerido >= 0 ? origen_sugerido : (g.nodos.empty() ? 0 : g.nodos[0].id);
    res.ruta.push_back(actual);
    std::vector<bool> visitado(g.rangoIds(), false);
    visitado[actual] = true;

    for (size_t paso = 1; paso < g.nodos.size(); paso++) {
        int mejor = -1;
        float mejor_dist = 1e9f;
        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == actual && !visitado[a.destino_id]) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == actual && !visitado[a.origen_id]) v = a.origen_id;
            if (v >= 0 && a.peso < mejor_dist) { mejor_dist = a.peso; mejor = v; }
        }
        if (mejor < 0) break;
        res.ruta.push_back(mejor);
        visitado[mejor] = true;
        res.distancia_total += mejor_dist;
        actual = mejor;
    }

    for (const auto& a : g.aristas) {
        if ((a.origen_id == actual && a.destino_id == res.ruta[0]) ||
            (!a.es_dirigida && a.destino_id == actual && a.origen_id == res.ruta[0])) {
            res.distancia_total += a.peso;
            res.ruta.push_back(res.ruta[0]);
            break;
        }
    }
    return res;
}

std::vector<PasoAnimacion> EulerHamilton::generarPasosHamiltonHeuristico(const Grafo& g, int origen_sugerido) {
    std::vector<PasoAnimacion> pasos;
    auto res = buscarCaminoHamiltonianoHeuristico(g, origen_sugerido);
    if (res.ruta.empty()) return pasos;

    for (size_t i = 1; i < res.ruta.size(); i++) {
        PasoAnimacion paso1;
        paso1.accion = PasoAnimacion::VISITAR;
        paso1.arista_origen = res.ruta[i-1];
        paso1.arista_destino = res.ruta[i];
        paso1.descripcion = "Viajando (" + std::to_string(i) + "/" + std::to_string(res.ruta.size()-1) + "): " + g.nombreNodo(res.ruta[i-1]) + " -> " + g.nombreNodo(res.ruta[i]);
        pasos.push_back(paso1);

        PasoAnimacion paso2;
        paso2.accion = PasoAnimacion::CONFIRMAR;
        paso2.arista_origen = res.ruta[i-1];
        paso2.arista_destino = res.ruta[i];
        paso2.descripcion = "Camino confirmado";
        pasos.push_back(paso2);
    }
    return pasos;
}

}
