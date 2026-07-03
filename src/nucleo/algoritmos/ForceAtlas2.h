#pragma once

#include "../Grafo.hpp"
#include "QuadTree.h"
#include "imgui.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace Algoritmos {

struct ParametrosFA2 {
    float scaling_ratio       = 10.0f;
    float gravity             = 1.0f;
    float edge_weight_influence = 1.0f;
    float jitter_tolerance    = 1.0f;
    bool  linlog_mode         = false;
    bool  dissuade_hubs       = false;
    bool  prevent_overlap     = false;
    bool  strong_gravity      = false;
};

struct ForceAtlas2 {
    struct DatosNodo {
        float dx = 0, dy = 0;
        float old_dx = 0, old_dy = 0;
        float masa = 1;
    };

    std::unordered_map<int, DatosNodo> datos;
    float speed            = 1.0f;
    float speed_efficiency = 1.0f;
    int   nodo_arrastrado  = -1;
    
    bool   gravedad_inicializada = false;
    ImVec2 ancla_gravedad = ImVec2(0, 0);
    QuadTree qtree;

    void reset() {
        datos.clear();
        speed            = 1.0f;
        speed_efficiency = 1.0f;
        gravedad_inicializada = false;
    }

    void step(Grafo& g, const ParametrosFA2& p) {
        int n_nodos = (int)g.nodos.size();
        if (n_nodos < 2) return;

        // 0. Preparar datos base (O(V)) y Ancla de Gravedad (Efecto Medusa Fix)
        ImVec2 centro_actual(0, 0);
        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            d.old_dx = d.dx;
            d.old_dy = d.dy;
            d.dx = 0;
            d.dy = 0;
            d.masa = 1.0f; // Inicializar masa
            centro_actual.x += n.posicion.x;
            centro_actual.y += n.posicion.y;
        }
        centro_actual.x /= n_nodos;
        centro_actual.y /= n_nodos;
        
        if (!gravedad_inicializada || g.nodos.size() != datos.size()) {
            ancla_gravedad = centro_actual;
            gravedad_inicializada = true;
        }

        // Calcular masa iterando sobre aristas (O(E)) en vez de O(V*E)
        for (const auto& a : g.aristas) {
            datos[a.origen_id].masa += 1.0f;
            datos[a.destino_id].masa += 1.0f;
        }

        // Constantes ajustadas para estabilidad
        float k_rep = p.scaling_ratio * 10.0f; 
        
        // 1. Repulsion y Prevencion de Overlap (Barnes-Hut O(V log V))
        // Encontrar límites del QuadTree
        float min_x = 9999999, min_y = 9999999;
        float max_x = -9999999, max_y = -9999999;
        for (const auto& n : g.nodos) {
            if (n.posicion.x < min_x) min_x = n.posicion.x;
            if (n.posicion.y < min_y) min_y = n.posicion.y;
            if (n.posicion.x > max_x) max_x = n.posicion.x;
            if (n.posicion.y > max_y) max_y = n.posicion.y;
        }
        // Margen extra para que nodos en el borde exacto no fallen
        min_x -= 10.0f; min_y -= 10.0f;
        max_x += 10.0f; max_y += 10.0f;

        // Threshold for Barnes-Hut vs Brute Force
        if (g.nodos.size() < 150) {
            // Fuerza bruta (más rápido para grafos pequeños)
            for (size_t i = 0; i < g.nodos.size(); i++) {
                auto& ni = g.nodos[i];
                auto& di = datos[ni.id];
                for (size_t j = i + 1; j < g.nodos.size(); j++) {
                    auto& nj = g.nodos[j];
                    auto& dj = datos[nj.id];
                    float dx = ni.posicion.x - nj.posicion.x;
                    float dy = ni.posicion.y - nj.posicion.y;
                    float dist = sqrtf(dx * dx + dy * dy);
                    if (dist < 0.01f) {
                        dx = (rand() % 100 - 50) * 0.01f;
                        dy = (rand() % 100 - 50) * 0.01f;
                        dist = sqrtf(dx * dx + dy * dy) + 0.01f;
                    }
                    float rep = k_rep * (di.masa * dj.masa) / dist;
                    if (p.prevent_overlap) {
                        float min_d = ni.radio + nj.radio + 2.0f;
                        if (dist < min_d) rep *= 10.0f;
                    }
                    float rep_x = (dx / dist) * rep;
                    float rep_y = (dy / dist) * rep;
                    di.dx += rep_x;
                    di.dy += rep_y;
                    dj.dx -= rep_x;
                    dj.dy -= rep_y;
                }
            }
        } else {
            // QuadTree / Barnes-Hut para grafos densos
            qtree.build(min_x, min_y, max_x, max_y, g.nodos.size());
            for (const auto& n : g.nodos) {
                qtree.insert(n.posicion.x, n.posicion.y, datos[n.id].masa, n.id);
            }

            float theta = 1.2f; // Precisión de Barnes-Hut
            
            for (auto& n : g.nodos) {
                auto& d = datos[n.id];
                float fx = 0.0f, fy = 0.0f;
                qtree.calcRepulsion(n.posicion.x, n.posicion.y, d.masa, n.radio, k_rep, theta, p.prevent_overlap, fx, fy, n.id);
                d.dx += fx;
                d.dy += fy;
            }
        }

        // 2. Atraccion (aristas)
        for (const auto& a : g.aristas) {
            auto* no = g.obtenerNodo(a.origen_id);
            auto* nd = g.obtenerNodo(a.destino_id);
            if (!no || !nd || no == nd) continue;

            auto& do_ = datos[a.origen_id];
            auto& dd  = datos[a.destino_id];

            float dx   = no->posicion.x - nd->posicion.x;
            float dy   = no->posicion.y - nd->posicion.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 0.01f) continue;

            float weight = powf(std::max(0.01f, a.peso_actual), p.edge_weight_influence);
            float atr = 0;
            
            if (p.linlog_mode) {
                // LinLog real: log(1 + dist)
                atr = logf(1.0f + dist) * weight;
            } else {
                // Estandar FA2
                atr = dist * weight;
            }

            if (p.dissuade_hubs) {
                atr /= do_.masa;
            }

            // Mitigación de gravedad doble para aristas paralelas no-dirigidas
            if (!a.es_dirigida) {
                for (const auto& otra : g.aristas) {
                    if (&otra != &a && otra.origen_id == a.destino_id && otra.destino_id == a.origen_id) {
                        atr *= 0.5f;
                        break;
                    }
                }
            }

            float atr_x = (dx / dist) * atr;
            float atr_y = (dy / dist) * atr;

            do_.dx -= atr_x;
            do_.dy -= atr_y;
            dd.dx  += atr_x;
            dd.dy  += atr_y;
        }

        // 3. Gravedad anclada (Fix Efecto Medusa)
        for (auto& n : g.nodos) {
            auto& d  = datos[n.id];
            float dx = n.posicion.x - ancla_gravedad.x;
            float dy = n.posicion.y - ancla_gravedad.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 0.01f) {
                // Gravedad estándar: constante pero con un suave factor restaurador si se alejan mucho
                // Esto permite la forma de anillo de Gephi, pero evita que escapen al infinito
                float grav = p.gravity * d.masa * 1.5f; 
                
                if (p.strong_gravity) {
                    grav = p.gravity * d.masa * 0.1f * dist; // Strong Gravity = proporcional a la distancia
                } else {
                    // Soft boundary: si escapan más allá de 2000 px, la gravedad se vuelve elástica para retenerlos
                    if (dist > 2000.0f) {
                        grav += (dist - 2000.0f) * 0.05f;
                    }
                }
                
                d.dx -= (dx / dist) * grav;
                d.dy -= (dy / dist) * grav;
            }
        }

        // 4. Adaptacion de velocidad (Global Swing/Traction)
        float global_swing   = 0.0f;
        float global_traction = 0.0f;
        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            float swing   = sqrtf((d.dx - d.old_dx) * (d.dx - d.old_dx) +
                                  (d.dy - d.old_dy) * (d.dy - d.old_dy));
            float traction = sqrtf((d.dx + d.old_dx) * (d.dx + d.old_dx) +
                                   (d.dy + d.old_dy) * (d.dy + d.old_dy)) * 0.5f;
            global_swing   += d.masa * swing;
            global_traction += d.masa * traction;
        }

        if (global_swing > 0.001f) {
            float target = p.jitter_tolerance * global_traction / global_swing;
            speed = speed * 0.9f + target * 0.1f; // Suavizado de speed
            speed = std::min(speed, 10.0f); // Cap maximo estricto para que no explote
            speed = std::max(speed, 0.01f);
        }

        // 5. Aplicar fuerzas
        for (auto& n : g.nodos) {
            if (n.id == nodo_arrastrado) continue;

            auto& d = datos[n.id];
            float len = sqrtf(d.dx * d.dx + d.dy * d.dy);
            if (len < 0.001f) continue;

            // Limitar el movimiento maximo por paso localmente (Clamp)
            float max_move = speed / (1.0f + speed * sqrtf(global_swing));
            float move = std::min(len * max_move, 15.0f); // cap estabilizado

            float nx = n.posicion.x + (d.dx / len) * move;
            float ny = n.posicion.y + (d.dy / len) * move;
            
            // Sanitización extrema contra NaNs e Infinitos que destruyen el FPS en ImGui
            if (std::isnan(nx) || std::isinf(nx)) nx = (rand() % 100 - 50);
            if (std::isnan(ny) || std::isinf(ny)) ny = (rand() % 100 - 50);
            
            // Limitar el universo (Forma circular para estetica natural y evitar bugs de ImGui)
            float max_r = 10000.0f;
            float dist_c = sqrtf(nx*nx + ny*ny);
            if (dist_c > max_r) {
                nx = (nx / dist_c) * max_r;
                ny = (ny / dist_c) * max_r;
            }
            n.posicion.x = nx;
            n.posicion.y = ny;
        }


    }
};

}
