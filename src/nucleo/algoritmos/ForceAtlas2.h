#pragma once

#include "../Grafo.h"
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

    void reset() {
        datos.clear();
        speed            = 1.0f;
        speed_efficiency = 1.0f;
    }

    void step(Grafo& g, const ParametrosFA2& p) {
        int n_nodos = (int)g.nodos.size();
        if (n_nodos < 2) return;

        // El centro debe ser el origen (0,0) para evitar que la gravedad 
        // cause aceleración neta y el grafo se vaya a la deriva.
        ImVec2 centro(0, 0);

        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            d.old_dx = d.dx;
            d.old_dy = d.dy;
            d.dx = 0;
            d.dy = 0;
            d.masa = 1.0f + (float)g.gradoNodo(n.id);
        }

        // Constantes ajustadas para estabilidad
        float k_rep = p.scaling_ratio * 10.0f; 
        
        // 1. Repulsion y Prevencion de Overlap
        for (size_t i = 0; i < g.nodos.size(); i++) {
            auto& ni = g.nodos[i];
            auto& di = datos[ni.id];

            for (size_t j = i + 1; j < g.nodos.size(); j++) {
                auto& nj = g.nodos[j];
                auto& dj = datos[nj.id];

                float dx   = ni.posicion.x - nj.posicion.x;
                float dy   = ni.posicion.y - nj.posicion.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < 0.01f) {
                    dx = (rand() % 100 - 50) * 0.01f;
                    dy = (rand() % 100 - 50) * 0.01f;
                    dist = sqrtf(dx * dx + dy * dy) + 0.01f;
                }

                // Repulsion estándar FA2
                float rep = k_rep * (di.masa * dj.masa) / dist;
                
                // Integración suave de Prevent Overlap
                if (p.prevent_overlap) {
                    float min_d = ni.radio + nj.radio + 2.0f;
                    if (dist < min_d) {
                        rep *= 10.0f; // Multiplicador fuerte pero estable en vez de suma explosiva
                    }
                }

                float rep_x = (dx / dist) * rep;
                float rep_y = (dy / dist) * rep;

                di.dx += rep_x;
                di.dy += rep_y;
                dj.dx -= rep_x;
                dj.dy -= rep_y;
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

            float atr_x = (dx / dist) * atr;
            float atr_y = (dy / dist) * atr;

            do_.dx -= atr_x;
            do_.dy -= atr_y;
            dd.dx  += atr_x;
            dd.dy  += atr_y;
        }

        // 3. Gravedad
        for (auto& n : g.nodos) {
            auto& d  = datos[n.id];
            float dx = n.posicion.x - centro.x;
            float dy = n.posicion.y - centro.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 0.01f) {
                // Gravedad proporcional a la distancia por defecto en FA2
                float grav = p.gravity * d.masa * 0.05f; 
                if (p.strong_gravity) grav = p.gravity * d.masa * 0.5f; // Mayor fuerza pero sin ser destructiva
                
                d.dx -= (dx / dist) * grav * dist;
                d.dy -= (dy / dist) * grav * dist;
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
            float move = std::min(len * max_move, 10.0f); // cap de desplazamiento ultra estricto por frame

            n.posicion.x += (d.dx / len) * move;
            n.posicion.y += (d.dy / len) * move;
        }


    }
};

} // namespace Algoritmos
