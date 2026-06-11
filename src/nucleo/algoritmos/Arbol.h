#pragma once

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <queue>
#include <limits>

#include "../Grafo.h"

namespace Algoritmos {
namespace Arbol {

// Resultado del analisis de arbol
struct PropiedadesArbol {
    bool es_arbol = false;          // ¿el grafo cumple las condiciones?
    std::string razon_no_arbol;     // por que no es arbol (si aplica)

    int raiz_id = -1;

    // Por nodo
    std::map<int, int>              nivel;          // nodo_id → nivel
    std::map<int, int>              padre;          // nodo_id → padre (-1 si raiz)
    std::map<int, std::vector<int>> hijos;          // nodo_id → lista de hijos
    std::map<int, std::vector<int>> ancestros;      // nodo_id → lista de ancestros (raiz al ultimo)
    std::map<int, std::vector<int>> descendientes;  // nodo_id → lista de descendientes

    // Globales
    int                  altura      = 0;           // max nivel
    int                  grado_arbol = 0;           // max grado de un nodo
    std::vector<int>     hojas;                     // IDs de nodos hoja
    std::vector<int>     rama_mas_larga;            // secuencia de nodos (raiz a hoja mas lejana)
    std::vector<int>     rama_mas_corta;            // raiz a hoja mas cercana

    // Primos: pares de nodos en el mismo nivel con distinto padre
    std::vector<std::pair<int,int>> primos;
    // Hermanos: agrupados por padre
    std::map<int, std::vector<int>> hermanos;       // padre_id → lista de hijos (= hermanos entre si)
};

// ── Helpers internos ──────────────────────────────────────────────────────────

// Verifica si el grafo es un arbol (conexo + aciclico)
inline bool verificarEsArbol(const Grafo& g, std::string& razon) {
    if (g.nodos.empty()) { razon = "El grafo esta vacio"; return false; }

    int n = (int)g.nodos.size();
    int e = (int)g.aristas.size();

    if (e != n - 1) {
        razon = "Un arbol de " + std::to_string(n) + " nodos debe tener exactamente " +
                std::to_string(n - 1) + " aristas. Este tiene " + std::to_string(e) + ".";
        return false;
    }

    // Verificar conexidad con BFS
    std::vector<bool> visitado(g.rangoIds(), false);
    std::queue<int> cola;
    cola.push(g.nodos[0].id);
    visitado[g.nodos[0].id] = true;
    int visitados = 0;

    while (!cola.empty()) {
        int u = cola.front(); cola.pop();
        visitados++;
        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (a.destino_id == u) v = a.origen_id;
            if (v != -1 && v < g.rangoIds() && !visitado[v]) {
                visitado[v] = true;
                cola.push(v);
            }
        }
    }

    if (visitados != n) {
        razon = "El grafo no es conexo. Solo se alcanzan " +
                std::to_string(visitados) + " de " + std::to_string(n) + " nodos.";
        return false;
    }
    return true;
}

// BFS desde la raiz para construir la estructura del arbol
inline void construirDesdeRaiz(const Grafo& g, PropiedadesArbol& props) {
    int raiz = props.raiz_id;
    std::queue<int> cola;
    cola.push(raiz);
    props.nivel[raiz]  = 0;
    props.padre[raiz]  = -1;
    props.hijos[raiz]  = {};

    std::set<int> visitado;
    visitado.insert(raiz);
    props.altura = 0;

    while (!cola.empty()) {
        int u = cola.front(); cola.pop();

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (a.destino_id == u) v = a.origen_id;
            if (v == -1 || visitado.count(v)) continue;

            visitado.insert(v);
            cola.push(v);

            props.nivel[v]  = props.nivel[u] + 1;
            props.padre[v]  = u;
            props.hijos[u].push_back(v);
            props.hijos[v]  = {};

            if (props.nivel[v] > props.altura)
                props.altura = props.nivel[v];
        }
    }
}

// Calcula descendientes recursivamente (DFS desde cada nodo)
inline void calcularDescendientes(int u, PropiedadesArbol& props) {
    props.descendientes[u] = {};
    for (int hijo : props.hijos[u]) {
        calcularDescendientes(hijo, props);
        props.descendientes[u].push_back(hijo);
        for (int desc : props.descendientes[hijo])
            props.descendientes[u].push_back(desc);
    }
}

// Calcula ancestros de cada nodo (camino hacia la raiz)
inline void calcularAncestros(int u, const PropiedadesArbol& props_const, PropiedadesArbol& props) {
    props.ancestros[u] = {};
    if (!props_const.padre.count(u)) return;
    int p = props_const.padre.at(u);
    while (p != -1) {
        props.ancestros[u].push_back(p);
        p = props_const.padre.count(p) ? props_const.padre.at(p) : -1;
    }
}

// ── Funcion principal ─────────────────────────────────────────────────────────

inline PropiedadesArbol analizar(const Grafo& g, int raiz_id) {
    PropiedadesArbol props;
    props.raiz_id = raiz_id;

    // Verificar que es arbol
    props.es_arbol = verificarEsArbol(g, props.razon_no_arbol);
    if (!props.es_arbol) return props;

    // Si la raiz no existe, usar el primer nodo
    if (!g.obtenerNodo(raiz_id) && !g.nodos.empty())
        props.raiz_id = g.nodos[0].id;

    // Construir jerarquia desde la raiz
    construirDesdeRaiz(g, props);

    // Grado del arbol (max hijos)
    props.grado_arbol = 0;
    for (const auto& [nid, hijos] : props.hijos) {
        if ((int)hijos.size() > props.grado_arbol)
            props.grado_arbol = (int)hijos.size();
    }

    // Hojas
    for (const auto& n : g.nodos) {
        if (props.hijos.count(n.id) && props.hijos[n.id].empty())
            props.hojas.push_back(n.id);
    }

    // Descendientes y ancestros
    calcularDescendientes(props.raiz_id, props);
    for (const auto& n : g.nodos)
        calcularAncestros(n.id, props, props);

    // Hermanos (agrupados por padre)
    for (const auto& [pid, hijos] : props.hijos) {
        if (!hijos.empty()) props.hermanos[pid] = hijos;
    }

    // Primos (mismo nivel, distinto padre)
    std::map<int, std::vector<int>> nodos_por_nivel;
    for (const auto& [nid, niv] : props.nivel)
        nodos_por_nivel[niv].push_back(nid);

    for (const auto& [niv, nids] : nodos_por_nivel) {
        for (int i = 0; i < (int)nids.size(); i++) {
            for (int j = i + 1; j < (int)nids.size(); j++) {
                int a = nids[i], b = nids[j];
                if (props.padre[a] != props.padre[b])
                    props.primos.push_back({a, b});
            }
        }
    }

    // Rama mas larga (raiz a la hoja con mayor nivel)
    if (!props.hojas.empty()) {
        int hoja_lejana = *std::max_element(props.hojas.begin(), props.hojas.end(),
            [&](int a, int b) { return props.nivel[a] < props.nivel[b]; });
        int at = hoja_lejana;
        while (at != -1) {
            props.rama_mas_larga.push_back(at);
            at = props.padre.count(at) ? props.padre[at] : -1;
        }
        std::reverse(props.rama_mas_larga.begin(), props.rama_mas_larga.end());

        // Rama mas corta (raiz a la hoja con menor nivel)
        int hoja_cercana = *std::min_element(props.hojas.begin(), props.hojas.end(),
            [&](int a, int b) { return props.nivel[a] < props.nivel[b]; });
        at = hoja_cercana;
        while (at != -1) {
            props.rama_mas_corta.push_back(at);
            at = props.padre.count(at) ? props.padre[at] : -1;
        }
        std::reverse(props.rama_mas_corta.begin(), props.rama_mas_corta.end());
    }

    return props;
}

} // namespace Arbol
} // namespace Algoritmos
