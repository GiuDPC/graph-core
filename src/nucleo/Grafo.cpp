#include "Grafo.hpp"

#include <cmath>

// Rango de IDs

int Grafo::rangoIds() const {
    int max_id = 0;
    for (const auto& n : nodos) {
        if (n.id > max_id) max_id = n.id;
    }
    return nodos.empty() ? 0 : max_id + 1;
}

// CRUD nodos

void Grafo::agregarNodo(ImVec2 posicion, TipoHardware tipo) {
    nodos.push_back(Nodo(contador_ids, posicion, tipo));
    contador_ids++;
}

void Grafo::eliminarNodo(int id) {
    aristas.erase(
        std::remove_if(aristas.begin(), aristas.end(),
            [id](const Arista& a) { return a.origen_id == id || a.destino_id == id; }),
        aristas.end());
    nodos.erase(
        std::remove_if(nodos.begin(), nodos.end(),
            [id](const Nodo& n) { return n.id == id; }),
        nodos.end());
    if (nodos.empty()) contador_ids = 0;
}

// CRUD aristas

void Grafo::agregarArista(int id1, int id2, float peso, bool dirigida) {
    if (!dirigida && id1 == id2) return;
    for (const auto& a : aristas) {
        if (a.origen_id == id1 && a.destino_id == id2) return;
    }
    aristas.push_back(Arista(id1, id2, peso, dirigida));
}

void Grafo::agregarArista(int id1, int id2, float peso) {
    agregarArista(id1, id2, peso, false);
}

// Queries estructurales

int Grafo::gradoNodo(int id) const {
    int grado = 0;
    for (const auto& a : aristas) {
        if (a.origen_id == id || (!a.es_dirigida && a.destino_id == id)) grado++;
        else if (a.es_dirigida && a.destino_id == id) grado++;
    }
    return grado;
}

std::vector<int> Grafo::vecinos(int id) const {
    std::vector<int> result;
    for (const auto& a : aristas) {
        if (a.origen_id == id) result.push_back(a.destino_id);
        else if (!a.es_dirigida && a.destino_id == id) result.push_back(a.origen_id);
    }
    return result;
}

const Arista* Grafo::obtenerArista(int id1, int id2) const {
    for (const auto& a : aristas) {
        if ((a.origen_id == id1 && a.destino_id == id2) ||
            (!a.es_dirigida && a.origen_id == id2 && a.destino_id == id1))
            return &a;
    }
    return nullptr;
}
