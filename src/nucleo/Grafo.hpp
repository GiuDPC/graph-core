#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <random>

#include "tipos/Nodo.h"
#include "tipos/Arista.h"

// Grafo: vector-of-vertices, vector-of-edges (no adjacency lists)
// Nodo.h pulls in ImVec2 via imgui.h — TODO: extract Posicion2D
class Grafo {
public:
    std::vector<Nodo>   nodos;
    std::vector<Arista> aristas;
    int contador_ids = 0;

    // ── Inline helpers (trivial getters, keeps .cpp lean) ───────────────

    std::string nombreNodo(int id) const {
        const Nodo* n = obtenerNodo(id);
        return n ? n->nombre : "?";
    }

    bool estaVacio() const { return nodos.empty(); }

    Nodo* obtenerNodo(int id) {
        for (auto& n : nodos) if (n.id == id) return &n;
        return nullptr;
    }

    const Nodo* obtenerNodo(int id) const {
        for (const auto& n : nodos) if (n.id == id) return &n;
        return nullptr;
    }

    void limpiar() {
        nodos.clear();
        aristas.clear();
        contador_ids = 0;
    }

    void resetearPesos() {
        for (auto& a : aristas) a.peso_actual = a.peso;
    }

    static std::mt19937& obtenerGeneradorAleatorio() {
        static std::mt19937 gen(42);
        return gen;
    }

    // ── .cpp (non-trivial implementations) ──────────────────────────────

    int rangoIds() const;

    void agregarNodo(ImVec2 posicion, TipoHardware tipo = TipoHardware::Servidor);

    void eliminarNodo(int id);

    void agregarArista(int id1, int id2, float peso, bool dirigida);
    void agregarArista(int id1, int id2, float peso = 1.0f);

    void aplicarJitter(float porcentaje);

    int gradoNodo(int id) const;
    std::vector<int> vecinos(int id) const;
    const Arista* obtenerArista(int id1, int id2) const;
};
