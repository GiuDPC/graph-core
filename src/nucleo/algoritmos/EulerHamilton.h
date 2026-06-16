#pragma once

#include "../Grafo.h"
#include <vector>
#include <stack>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

namespace Algoritmos {

struct EulerHamilton {

    // Algoritmo de Fleury para Circuito/Camino Euleriano 
    // Devuelve los indices de los nodos en orden. Si esta vacio, no hay.
    static std::vector<int> buscarCaminoEuleriano(Grafo& g) {
        std::vector<int> camino;
        if (g.nodos.empty()) return camino;

        // Primero verificamos si es euleriano (todos grados pares) o si tiene camino (exactamente 2 impares)
        std::vector<int> grados(g.nodos.size(), 0);
        for (const auto& arista : g.aristas) {
            grados[arista.origen_id]++;
            grados[arista.destino_id]++;
        }

        int impares = 0;
        int start_node = 0; // Default start
        for (size_t i = 0; i < grados.size(); i++) {
            if (grados[i] % 2 != 0) {
                impares++;
                start_node = i; // Start at an odd degree node if it exists
            }
        }

        if (impares != 0 && impares != 2) {
            // No hay camino ni circuito euleriano
            return camino;
        }

        // Usamos Hierholzer algorithm (mas eficiente que Fleury)
        std::unordered_map<int, std::vector<int>> adj;
        for (const auto& a : g.aristas) {
            adj[a.origen_id].push_back(a.destino_id);
            adj[a.destino_id].push_back(a.origen_id); // Grafo no dirigido
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

    // --- Backtracking para Camino Hamiltoniano ---
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

    static std::vector<int> buscarCaminoHamiltoniano(const Grafo& g) {
        std::vector<int> camino;
        if (g.nodos.empty()) return camino;

        std::unordered_map<int, std::vector<int>> adj;
        for (const auto& a : g.aristas) {
            adj[a.origen_id].push_back(a.destino_id);
            adj[a.destino_id].push_back(a.origen_id);
        }

        int num_nodos = g.nodos.size();
        std::vector<bool> visitado(num_nodos, false);

        for (int i = 0; i < num_nodos; i++) {
            if (hamiltonianoRecursivo(i, num_nodos, adj, visitado, camino)) {
                return camino;
            }
        }

        return {};
    }
};

} 