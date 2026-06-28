#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include "../Grafo.h"

namespace Algoritmos {
namespace Isomorfismo {

struct ResultadoIsomorfismo {
    bool                            son_isomorfos = false;
    std::vector<std::pair<int,int>> mapeo;          // id_g1 id_g2
    std::string                     descripcion;
    // condiciones de verificacion rapida
    bool misma_cantidad_nodos   = false;
    bool misma_cantidad_aristas = false;
    bool misma_secuencia_grados = false;
};

// genera la secuencia de grados ordenada de mayor a menor
std::vector<int> secuenciaGrados(const Grafo& g) {
    std::vector<int> grados;
    for (const auto& n : g.nodos)
        grados.push_back(g.gradoNodo(n.id));
    std::sort(grados.rbegin(), grados.rend());
    return grados;
}

// backtracking para encontrar el mapeo isomorfo
bool backtrack(const Grafo& g1, const Grafo& g2,
               std::vector<int>& mapeo,          // mapeo[i] id en g2 para nodo i de g1
               std::vector<bool>& usado_g2,
               int idx)
{
    if (idx == (int)g1.nodos.size()) return true;

    int u1 = g1.nodos[idx].id;
    for (int j = 0; j < (int)g2.nodos.size(); j++) {
        if (usado_g2[j]) continue;
        int u2 = g2.nodos[j].id;

        // verificar consistencia para cada arista u1 v1 ya mapeada
        // debe existir u2 mapeo[v1] en g2
        bool consistente = true;
        for (int k = 0; k < idx && consistente; k++) {
            int v1 = g1.nodos[k].id;
            int v2_mapped = mapeo[k];
            // Verificamos conexión de ida (u -> v)
            bool arista_ida_g1 = (g1.obtenerArista(u1, v1) != nullptr);
            bool arista_ida_g2 = (g2.obtenerArista(u2, v2_mapped) != nullptr);
            if (arista_ida_g1 != arista_ida_g2) consistente = false;
            
            // Verificamos conexión de vuelta (v -> u)
            bool arista_vuelta_g1 = (g1.obtenerArista(v1, u1) != nullptr);
            bool arista_vuelta_g2 = (g2.obtenerArista(v2_mapped, u2) != nullptr);
            if (arista_vuelta_g1 != arista_vuelta_g2) consistente = false;
        }

        if (consistente) {
            mapeo[idx] = u2;
            usado_g2[j] = true;
            if (backtrack(g1, g2, mapeo, usado_g2, idx + 1)) return true;
            mapeo[idx] = -1;
            usado_g2[j] = false;
        }
    }
    return false;
}

ResultadoIsomorfismo verificar(const Grafo& g1, const Grafo& g2) {
    ResultadoIsomorfismo resultado;

    // ── CORTAFUEGOS ──
    if (g1.nodos.size() > 12 || g2.nodos.size() > 12) {
        resultado.son_isomorfos = false;
        resultado.descripcion = "Grafos demasiado grandes para verificacion topologica exacta (max 12 nodos). Use verificacion geometrica.";
        return resultado;
    }

    // condicion 1 mismo numero de nodos
    resultado.misma_cantidad_nodos = (g1.nodos.size() == g2.nodos.size());
    if (!resultado.misma_cantidad_nodos) {
        resultado.descripcion = "Distinto numero de nodos (" +
            std::to_string(g1.nodos.size()) + " vs " + std::to_string(g2.nodos.size()) + ")";
        return resultado;
    }

    // condicion 2 mismo numero de aristas
    resultado.misma_cantidad_aristas = (g1.aristas.size() == g2.aristas.size());
    if (!resultado.misma_cantidad_aristas) {
        resultado.descripcion = "Distinto numero de aristas (" +
            std::to_string(g1.aristas.size()) + " vs " + std::to_string(g2.aristas.size()) + ")";
        return resultado;
    }

    // condicion 3 misma secuencia de grados
    auto deg1 = secuenciaGrados(g1);
    auto deg2 = secuenciaGrados(g2);
    resultado.misma_secuencia_grados = (deg1 == deg2);
    if (!resultado.misma_secuencia_grados) {
        resultado.descripcion = "Distinta secuencia de grados";
        return resultado;
    }

    // backtracking para encontrar el mapeo
    int n = (int)g1.nodos.size();
    std::vector<int>  mapeo_ids(n, -1);
    std::vector<bool> usado(n, false);

    if (backtrack(g1, g2, mapeo_ids, usado, 0)) {
        resultado.son_isomorfos = true;
        for (int i = 0; i < n; i++) {
            resultado.mapeo.push_back({g1.nodos[i].id, mapeo_ids[i]});
        }
        resultado.descripcion = "Los grafos son isomorfos. Se encontro el mapeo de nodos.";
    } else {
        resultado.descripcion = "Las condiciones necesarias se cumplen pero no existe mapeo valido. "
                                "Los grafos NO son isomorfos.";
    }
    return resultado;
}

struct ResultadoIsoGeometrico {
    bool son_isomorfos = false;
    std::string descripcion;
    float error_maximo = 0.0f;
    std::vector<std::pair<int,int>> mapeo; // id_g1 id_g2
};

// O(V^2) - Compara la geometria y estructura de forma precisa.
inline ResultadoIsoGeometrico verificarGeometrico(const Grafo& g1, const Grafo& g2) {
    ResultadoIsoGeometrico res;
    
    if (g1.nodos.size() != g2.nodos.size() || g1.aristas.size() != g2.aristas.size()) {
        res.descripcion = "Distinto numero de nodos o aristas.";
        return res;
    }

    auto deg1 = secuenciaGrados(g1);
    auto deg2 = secuenciaGrados(g2);
    if (deg1 != deg2) {
        res.descripcion = "Distinta secuencia de grados.";
        return res;
    }

    // Normalizar posiciones (centro de masa)
    ImVec2 centro1(0,0), centro2(0,0);
    for (const auto& n : g1.nodos) { centro1.x += n.posicion.x; centro1.y += n.posicion.y; }
    for (const auto& n : g2.nodos) { centro2.x += n.posicion.x; centro2.y += n.posicion.y; }
    if (!g1.nodos.empty()) {
        centro1.x /= g1.nodos.size(); centro1.y /= g1.nodos.size();
        centro2.x /= g2.nodos.size(); centro2.y /= g2.nodos.size();
    }

    float max_dist = 0.0f;
    for (const auto& n : g1.nodos) {
        float d = std::hypot(n.posicion.x - centro1.x, n.posicion.y - centro1.y);
        if (d > max_dist) max_dist = d;
    }
    float max_dist2 = 0.0f;
    for (const auto& n : g2.nodos) {
        float d = std::hypot(n.posicion.x - centro2.x, n.posicion.y - centro2.y);
        if (d > max_dist2) max_dist2 = d;
    }
    
    float scale1 = (max_dist > 0) ? 1.0f / max_dist : 1.0f;
    float scale2 = (max_dist2 > 0) ? 1.0f / max_dist2 : 1.0f;

    // Buscar emparejamiento geometrico mas cercano
    std::vector<int> mapeo(g1.nodos.size(), -1);
    std::vector<bool> usado(g2.nodos.size(), false);
    
    float error_acumulado = 0.0f;
    for (size_t i = 0; i < g1.nodos.size(); i++) {
        ImVec2 p1 = g1.nodos[i].posicion;
        float nx1 = (p1.x - centro1.x) * scale1;
        float ny1 = (p1.y - centro1.y) * scale1;

        int best_match = -1;
        float best_dist = 999999.0f;

        for (size_t j = 0; j < g2.nodos.size(); j++) {
            if (usado[j]) continue;
            
            // Si el grado no es el mismo, no pueden ser el mismo nodo geometricamente
            if (g1.gradoNodo(g1.nodos[i].id) != g2.gradoNodo(g2.nodos[j].id)) continue;

            ImVec2 p2 = g2.nodos[j].posicion;
            float nx2 = (p2.x - centro2.x) * scale2;
            float ny2 = (p2.y - centro2.y) * scale2;

            float d = std::hypot(nx1 - nx2, ny1 - ny2);
            if (d < best_dist) {
                best_dist = d;
                best_match = j;
            }
        }
        
        // Tolerancia estricta (1%) 
        if (best_match != -1 && best_dist < 0.05f) {
            mapeo[i] = g2.nodos[best_match].id;
            usado[best_match] = true;
            if (best_dist > res.error_maximo) res.error_maximo = best_dist;
            error_acumulado += best_dist;
        } else {
            res.descripcion = "La figura no coincide geometricamente (error en nodo " + g1.nodos[i].nombre + ").";
            return res;
        }
    }

    // Verificar conexiones usando el mapeo
    for (const auto& a1 : g1.aristas) {
        int idx_o = -1, idx_d = -1;
        for (size_t i = 0; i < g1.nodos.size(); i++) {
            if (g1.nodos[i].id == a1.origen_id) idx_o = i;
            if (g1.nodos[i].id == a1.destino_id) idx_d = i;
        }
        if (idx_o == -1 || idx_d == -1) continue;

        int m_o = mapeo[idx_o];
        int m_d = mapeo[idx_d];
        
        bool found = false;
        for (const auto& a2 : g2.aristas) {
            if ((a2.origen_id == m_o && a2.destino_id == m_d) || 
                (!a1.es_dirigida && !a2.es_dirigida && a2.origen_id == m_d && a2.destino_id == m_o)) {
                found = true;
                break;
            }
        }
        if (!found) {
            res.descripcion = "Coinciden geometricamente pero las aristas no corresponden.";
            return res;
        }
    }

    res.son_isomorfos = true;
    res.descripcion = "Los grafos SON isomorfos geometricamente con un error de " + std::to_string(res.error_maximo * 100.0f) + "%.";
    for (size_t i = 0; i < g1.nodos.size(); i++) {
        res.mapeo.push_back({g1.nodos[i].id, mapeo[i]});
    }
    return res;
}

}
}