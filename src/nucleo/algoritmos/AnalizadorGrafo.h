#pragma once

#include "../Grafo.h"
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

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

struct AnalisisCompleto {
    int num_nodos = 0;
    int num_aristas = 0;
    float densidad = 0.0f;
    float grado_promedio = 0.0f;
    int grado_max = 0;
    int grado_min = 0;
    int id_hub_max = -1;
    int id_hub_min = -1;
    float grado_desviacion = 0.0f;
    bool es_conexo = false;
    int num_componentes = 0;
    int tamano_componente_mas_grande = 0;
    float proporcion_conectada = 1.0f; // % de nodos en el componente gigante
    float diametro_aproximado = 0.0f;  // BFS desde 5 nodos aleatorios
    float coeficiente_clustering_global = 0.0f;
    float heterogeneidad = 0.0f; // (grado_max - grado_min) / grado_promedio
    std::string resumen;
    bool es_arbol = false;
    bool es_completo = false;
    bool es_bipartito = false;
    bool es_regular = false;
};

struct AnalizadorGrafo {

    static PropiedadesGrafo analizar(const Grafo& g) {
        PropiedadesGrafo r;
        int n = (int)g.nodos.size();
        if (n == 0) return r;

        int total_aristas = (int)g.aristas.size();

        // conexidad via BFS sobre ids reales
        std::unordered_set<int> visitado;
        std::queue<int> q;
        int semilla = g.nodos[0].id;
        q.push(semilla);
        visitado.insert(semilla);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int w : g.vecinos(v)) {
                if (!visitado.count(w)) {
                    visitado.insert(w);
                    q.push(w);
                }
            }
        }
        r.es_conexo = (visitado.size() == g.nodos.size());

        // euleriano: todos los grados pares
        if (r.es_conexo) {
            r.es_euleriano = true;
            for (const auto& nodo : g.nodos) {
                if (g.gradoNodo(nodo.id) % 2 != 0) {
                    r.es_euleriano = false;
                    break;
                }
            }
        }

        // arbol: conexo con n-1 aristas
        r.es_arbol = r.es_conexo && total_aristas == n - 1;

        // completo
        int max_posibles = n * (n - 1) / 2;
        r.es_completo = (total_aristas == max_posibles);

        // regular
        if (n > 0) {
            r.es_regular = true;
            int grado_ref = -1;
            for (const auto& nodo : g.nodos) {
                int grado = g.gradoNodo(nodo.id);
                if (grado_ref == -1) grado_ref = grado;
                else if (grado != grado_ref) { r.es_regular = false; break; }
            }
            r.grado_regular = r.es_regular ? grado_ref : -1;
        }

        // bipartito (BFS con coloreo 2-colores)
        std::unordered_map<int,int> color;
        for (const auto& nodo : g.nodos) color[nodo.id] = -1;
        bool bip = true;
        for (const auto& nodo : g.nodos) {
            if (!bip) break;
            if (color[nodo.id] != -1) continue;
            color[nodo.id] = 0;
            std::queue<int> bq;
            bq.push(nodo.id);
            while (!bq.empty() && bip) {
                int v = bq.front(); bq.pop();
                for (int w : g.vecinos(v)) {
                    if (color[w] == -1) {
                        color[w] = 1 - color[v];
                        bq.push(w);
                    } else if (color[w] == color[v]) {
                        bip = false;
                        break;
                    }
                }
            }
        }
        r.es_bipartito = bip;

        return r;
    }

    static AnalisisCompleto analisisDetallado(const Grafo& g) {
        AnalisisCompleto r;
        r.num_nodos = (int)g.nodos.size();
        r.num_aristas = (int)g.aristas.size();
        if (r.num_nodos == 0) return r;

        int max_id = g.rangoIds();
        if (max_id <= 0) return r;

        // Grados
        std::vector<int> grado(max_id, 0);
        for (const auto& a : g.aristas) {
            if (a.origen_id < max_id) grado[a.origen_id]++;
            if (a.destino_id < max_id && !a.es_dirigida) grado[a.destino_id]++;
            else if (a.es_dirigida && a.destino_id < max_id) grado[a.destino_id]++;
        }

        double suma_grados = 0;
        r.grado_max = 0;
        r.grado_min = 999999;
        r.id_hub_max = g.nodos[0].id;
        r.id_hub_min = g.nodos[0].id;
        std::vector<double> grados_vals;

        for (const auto& n : g.nodos) {
            int d = grado[n.id];
            suma_grados += d;
            grados_vals.push_back((double)d);
            if (d > r.grado_max) { r.grado_max = d; r.id_hub_max = n.id; }
            if (d < r.grado_min) { r.grado_min = d; r.id_hub_min = n.id; }
        }

        r.grado_promedio = (float)(suma_grados / r.num_nodos);

        // Desviación estándar del grado
        double varianza = 0;
        for (double d : grados_vals) varianza += (d - r.grado_promedio) * (d - r.grado_promedio);
        r.grado_desviacion = (float)sqrt(varianza / r.num_nodos);

        // Densidad
        if (r.num_nodos > 1) {
            r.densidad = (2.0f * r.num_aristas) / (r.num_nodos * (r.num_nodos - 1.0f));
        }

        // Heterogeneidad
        r.heterogeneidad = (r.grado_max - r.grado_min) / std::max(0.01f, r.grado_promedio);

        // Componentes conexas (BFS sobre todos los nodos)
        std::vector<int> componente(max_id, -1);
        int comp_actual = 0;
        int tam_max = 0;

        for (const auto& n : g.nodos) {
            if (componente[n.id] != -1) continue;
            // BFS desde este nodo
            std::queue<int> q;
            q.push(n.id);
            componente[n.id] = comp_actual;
            int tam = 1;

            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v : g.vecinos(u)) {
                    if (v < max_id && componente[v] == -1) {
                        componente[v] = comp_actual;
                        q.push(v);
                        tam++;
                    }
                }
            }

            if (tam > tam_max) tam_max = tam;
            comp_actual++;
        }

        r.num_componentes = comp_actual;
        r.tamano_componente_mas_grande = tam_max;
        r.proporcion_conectada = (float)tam_max / r.num_nodos;
        r.es_conexo = (comp_actual == 1);

        // Diámetro aproximado (BFS desde hasta 5 nodos, tomar el máximo de las excentricidades)
        int max_excentricidad = 0;
        int muestras = std::min(5, r.num_nodos);
        for (int s = 0; s < muestras; s++) {
            int semilla = g.nodos[s % g.nodos.size()].id;
            std::vector<int> dist(max_id, -1);
            std::queue<int> q;
            q.push(semilla);
            dist[semilla] = 0;
            int max_d = 0;

            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v : g.vecinos(u)) {
                    if (v < max_id && dist[v] == -1) {
                        dist[v] = dist[u] + 1;
                        q.push(v);
                        if (dist[v] > max_d) max_d = dist[v];
                    }
                }
            }
            if (max_d > max_excentricidad) max_excentricidad = max_d;
        }
        r.diametro_aproximado = (float)max_excentricidad;

        // Coeficiente de clustering global (transitividad)
        // Para cada nodo, cuantos pares de vecinos están conectados
        if (r.num_nodos > 2) {
            long long triangulos = 0;
            long long tripletes = 0;
            for (const auto& n : g.nodos) {
                int u = n.id;
                auto vecs = g.vecinos(u);
                int kv = (int)vecs.size();
                if (kv < 2) continue;
                tripletes += (long long)kv * (kv - 1) / 2;

                // Contar aristas entre vecinos
                int conex = 0;
                for (size_t i = 0; i < vecs.size(); i++) {
                    for (size_t j = i + 1; j < vecs.size(); j++) {
                        if (g.obtenerArista(vecs[i], vecs[j])) conex++;
                    }
                }
                triangulos += conex;
            }
            if (tripletes > 0) {
                r.coeficiente_clustering_global = (float)triangulos / (float)tripletes;
            }
        }

        // Rasgos cualitativos
        r.es_arbol = (r.es_conexo && r.num_aristas == r.num_nodos - 1);
        if (r.num_nodos > 1) {
            long long max_posibles = (long long)r.num_nodos * (r.num_nodos - 1) / 2;
            r.es_completo = (r.num_aristas >= max_posibles);
        }
        if (r.num_nodos > 0) {
            bool todos_iguales = true;
            for (double d : grados_vals) {
                if (std::abs(d - r.grado_promedio) > 0.01f) { todos_iguales = false; break; }
            }
            r.es_regular = todos_iguales;
        }

        // Resumen textual
        std::stringstream ss;
        ss << "|V|=" << r.num_nodos << " |E|=" << r.num_aristas;
        ss << " dens=" << (r.densidad * 100.0f) << "%";
        if (r.es_conexo) ss << " conexo";
        else ss << " " << r.num_componentes << " componentes";
        r.resumen = ss.str();

        return r;
    }
};

}
