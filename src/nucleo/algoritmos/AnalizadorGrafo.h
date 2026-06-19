#pragma once

#include "../Grafo.h"
#include <vector>
#include <queue>

namespace Algoritmos {

struct PropiedadesGrafo {
    bool es_conexo = false;
    bool es_arbol = false;
    bool es_completo = false;
    bool es_bipartito = false;
    bool es_regular = false;
    bool es_euleriano = false;
    int grado_regular = -1;
};

struct AnalizadorGrafo {

    static PropiedadesGrafo analizar(const Grafo& g) {
        PropiedadesGrafo props;
        int num_nodos = (int)g.nodos.size();
        int num_aristas = (int)g.aristas.size();

        if (num_nodos == 0) return props;

        // deteccion de conexo usando bfs
        std::vector<bool> visitado(num_nodos, false);
        int visitados_count = 0;
        
        std::queue<int> q;
        q.push(0);
        visitado[0] = true;
        visitados_count++;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            for (int v : g.vecinos(u)) {
                if (!visitado[v]) {
                    visitado[v] = true;
                    visitados_count++;
                    q.push(v);
                }
            }
        }
        
        props.es_conexo = (visitados_count == num_nodos);

        // deteccion de arbol conexo y sin ciclos e v 1
        if (props.es_conexo && num_aristas == num_nodos - 1) {
            props.es_arbol = true;
        }

        //  deteccion de completo kn
        // numero maximo de aristas en grafo no dirigido sin bucles nn1/2
        int max_aristas = num_nodos * (num_nodos - 1) / 2;
        if (num_aristas == max_aristas && num_nodos > 1) {
            props.es_completo = true;
        }

        // deteccion de regular todos tienen el mismo grado
        if (num_nodos > 0) {
            int grado_base = (int)g.vecinos(0).size();
            bool todos_iguales = true;
            bool todos_pares = (grado_base % 2 == 0);
            for (int i = 1; i < num_nodos; i++) {
                int grado_actual = (int)g.vecinos(i).size();
                if (grado_actual != grado_base) todos_iguales = false;
                if (grado_actual % 2 != 0) todos_pares = false;
            }
            if (todos_iguales) {
                props.es_regular = true;
                props.grado_regular = grado_base;
            }

            // deteccion de euleriano conexo y todos los grados son pares
            if (props.es_conexo && todos_pares && num_aristas > 0) {
                props.es_euleriano = true;
            }
        }

        // deteccion de bipartito 2coloreable
        // bfs para intentar colorear con 2 colores 0 y 1
        if (num_nodos > 0) {
            std::vector<int> color(num_nodos, -1);
            bool es_bipartito = true;

            for (int start = 0; start < num_nodos; start++) {
                if (color[start] == -1) {
                    std::queue<int> bq;
                    bq.push(start);
                    color[start] = 0;

                    while (!bq.empty() && es_bipartito) {
                        int u = bq.front();
                        bq.pop();

                        for (int v : g.vecinos(u)) {
                            if (color[v] == -1) {
                                color[v] = 1 - color[u];
                                bq.push(v);
                            } else if (color[v] == color[u]) {
                                es_bipartito = false;
                                break;
                            }
                        }
                    }
                }
            }
            props.es_bipartito = es_bipartito && (num_aristas > 0);
        }

        return props;
    }
};

}
