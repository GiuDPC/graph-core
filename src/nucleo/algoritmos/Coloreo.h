#pragma once

#include <vector>
#include <string>
#include "../Grafo.h"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {

struct ResultadoColoreo {
    std::vector<int> colores;
    int              num_colores;    
};

// greedy estandar orden de nodos
ResultadoColoreo coloreoGreedy(const Grafo& g) {
    ResultadoColoreo resultado;
    int rango = g.rangoIds();
    resultado.colores.assign(rango, -1);
    resultado.num_colores = 0;

    if (g.estaVacio()) return resultado;

    resultado.colores[g.nodos[0].id] = 0;
    resultado.num_colores = 1;

    for (size_t i = 1; i < g.nodos.size(); i++) {
        int u = g.nodos[i].id;
        std::vector<bool> usado(rango, false);

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v != -1 && v < rango && resultado.colores[v] != -1)
                usado[resultado.colores[v]] = true;
        }

        int c = 0;
        while (c < rango && usado[c]) c++;
        resultado.colores[u] = c;
        if (c + 1 > resultado.num_colores) resultado.num_colores = c + 1;
    }
    return resultado;
}

ResultadoColoreo coloreoWelshPowell(const Grafo& g) {
    ResultadoColoreo resultado;
    int rango = g.rangoIds();
    resultado.colores.assign(rango, -1);
    resultado.num_colores = 0;

    if (g.estaVacio()) return resultado;

    // ordenar nodos por grado descendente
    std::vector<int> orden;
    for (const auto& n : g.nodos) orden.push_back(n.id);
    std::sort(orden.begin(), orden.end(), [&](int a, int b) {
        return g.gradoNodo(a) > g.gradoNodo(b);
    });

    for (int u : orden) {
        if (resultado.colores[u] != -1) continue;
        resultado.colores[u] = resultado.num_colores;

        // asignar el mismo color a todos los noadyacentes no coloreados
        for (int v : orden) {
            if (resultado.colores[v] != -1) continue;
            bool adyacente = false;
            for (const auto& a : g.aristas) {
                if ((a.origen_id == u && a.destino_id == v) ||
                    (a.origen_id == v && a.destino_id == u) ||
                    (!a.es_dirigida && ((a.origen_id == u && a.destino_id == v) || (a.origen_id == v && a.destino_id == u)))) {
                    adyacente = true; break;
                }
                
                if (!a.es_dirigida && a.origen_id == v && resultado.colores[a.destino_id] == resultado.num_colores) {
                    adyacente = true; break;
                }
                if (!a.es_dirigida && a.destino_id == v && resultado.colores[a.origen_id] == resultado.num_colores) {
                    adyacente = true; break;
                }
            }
            if (!adyacente) {
                resultado.colores[v] = resultado.num_colores;
            }
        }
        resultado.num_colores++;
    }
    return resultado;
}

inline std::vector<PasoAnimacion> generarPasosColoreo(const Grafo& g) {
    std::vector<PasoAnimacion> pasos;
    int rango = g.rangoIds();
    std::vector<int> colores(rango, -1);

    if (g.estaVacio()) return pasos;

    // primer nodo
    int primero = g.nodos[0].id;
    colores[primero] = 0;
    pasos.push_back({PasoAnimacion::COLOREAR, primero, -1, -1,
        "color 0 -> " + g.nombreNodo(primero), 0});

    for (size_t i = 1; i < g.nodos.size(); i++) {
        int u = g.nodos[i].id;
        std::vector<bool> usado(rango + 1, false);

        for (const auto& a : g.aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v != -1 && v < rango && colores[v] != -1 && colores[v] < rango + 1)
                usado[colores[v]] = true;
        }

        int c = 0;
        while (c < rango + 1 && usado[c]) c++;
        colores[u] = c;

        pasos.push_back({PasoAnimacion::COLOREAR, u, -1, -1,
            "color " + std::to_string(c) + " -> " + g.nombreNodo(u), c});
    }
    return pasos;
}

}
