#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <random>

#include "tipos/Nodo.h"
#include "tipos/Arista.h"

// Clase grafo — solo estructura y operaciones basicas
class Grafo {
public:
    std::vector<Nodo>   nodos;
    std::vector<Arista> aristas;
    int contador_ids = 0;

    // ── Helpers ──────────────────────────────────────────────────────────────

    std::string nombreNodo(int id) const {
        const Nodo* n = obtenerNodo(id);
        return n ? n->nombre : "?";
    }

    // Retorna el ID maximo + 1 (para crear vectores de algoritmos de forma segura)
    int rangoIds() const {
        int max_id = 0;
        for (const auto& n : nodos) {
            if (n.id > max_id) max_id = n.id;
        }
        return nodos.empty() ? 0 : max_id + 1;
    }

    bool estaVacio() const { return nodos.empty(); }

    // ── CRUD nodos ────────────────────────────────────────────────────────────

    void agregarNodo(ImVec2 posicion, TipoHardware tipo = TipoHardware::Servidor) {
        nodos.push_back(Nodo(contador_ids, posicion, tipo));
        contador_ids++;
    }

    void eliminarNodo(int id) {
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

    // ── CRUD aristas ──────────────────────────────────────────────────────────

    // Overload con flag dirigida (nuevo)
    // dirigida=true:  permite self-loops, A→B y B→A NO son duplicados
    // dirigida=false: rechaza self-loops, A→B y B→A SÍ son duplicados (legacy)
    void agregarArista(int id1, int id2, float peso, bool dirigida) {
        if (!dirigida && id1 == id2) return;
        for (const auto& a : aristas) {
            if (a.origen_id == id1 && a.destino_id == id2) return; // duplicado exacto
            // Eliminado el check de duplicado simétrico para permitir aristas paralelas
            // en grafos no dirigidos (se renderizarán como curvas en el lienzo)
        }
        aristas.push_back(Arista(id1, id2, peso, dirigida));
    }

    // Overload legacy — delega al nuevo con dirigida=false (backward compat)
    void agregarArista(int id1, int id2, float peso = 1.0f) {
        agregarArista(id1, id2, peso, false);
    }

    // ── Simulacion ────────────────────────────────────────────────────────────

    void aplicarJitter(float porcentaje) {
        // Usar referencia a generador global (unico por programa)
        // definido al final de este archivo
        static std::mt19937& gen = obtenerGeneradorAleatorio();
        for (auto& a : aristas) {
            std::uniform_real_distribution<float> dist(-porcentaje, porcentaje);
            a.peso_actual = a.peso * (1.0f + dist(gen));
            if (a.peso_actual < 0.1f) a.peso_actual = 0.1f;
        }
    }

    void resetearPesos() {
        for (auto& a : aristas) a.peso_actual = a.peso;
    }

    // ── Queries estructurales ─────────────────────────────────────────────────

    // Grado de un nodo (numero de aristas conectadas)
    int gradoNodo(int id) const {
        int grado = 0;
        for (const auto& a : aristas) {
            if (a.origen_id == id || (!a.es_dirigida && a.destino_id == id)) grado++;
            if (a.es_dirigida && a.destino_id == id) grado++;
        }
        return grado;
    }

    // Vecinos de un nodo
    std::vector<int> vecinos(int id) const {
        std::vector<int> result;
        for (const auto& a : aristas) {
            if (a.origen_id == id) result.push_back(a.destino_id);
            else if (!a.es_dirigida && a.destino_id == id) result.push_back(a.origen_id);
        }
        return result;
    }

    // Arista entre dos nodos (nullptr si no existe)
    const Arista* obtenerArista(int id1, int id2) const {
        for (const auto& a : aristas) {
            if ((a.origen_id == id1 && a.destino_id == id2) ||
                (!a.es_dirigida && a.origen_id == id2 && a.destino_id == id1))
                return &a;
        }
        return nullptr;
    }

    // ── Generador aleatorio global (unico por programa) ───────────────────
    static std::mt19937& obtenerGeneradorAleatorio() {
        static std::mt19937 gen(42);
        return gen;
    }
};
