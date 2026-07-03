#include "ForceAtlas2.hpp"
#include <algorithm>
#include <cstdlib>
#include <unordered_set>

namespace Algoritmos {

void ForceAtlas2::step(Grafo& g, const ParametrosFA2& p) {
    int n_nodos = (int)g.nodos.size();
    if (n_nodos < 2) return;

    ImVec2 centro_actual(0, 0);
    for (auto& n : g.nodos) {
        auto& d = datos[n.id];
        d.old_dx = d.dx;
        d.old_dy = d.dy;
        d.dx = 0;
        d.dy = 0;
        d.masa = 1.0f;
        centro_actual.x += n.posicion.x;
        centro_actual.y += n.posicion.y;
    }
    centro_actual.x /= n_nodos;
    centro_actual.y /= n_nodos;

    if (!gravedad_inicializada || g.nodos.size() != datos.size()) {
        ancla_gravedad = centro_actual;
        gravedad_inicializada = true;
    }

    for (const auto& a : g.aristas) {
        datos[a.origen_id].masa += 1.0f;
        datos[a.destino_id].masa += 1.0f;
    }

    float k_rep = p.scaling_ratio * 10.0f;

    float min_x = 9999999, min_y = 9999999;
    float max_x = -9999999, max_y = -9999999;
    for (const auto& n : g.nodos) {
        if (n.posicion.x < min_x) min_x = n.posicion.x;
        if (n.posicion.y < min_y) min_y = n.posicion.y;
        if (n.posicion.x > max_x) max_x = n.posicion.x;
        if (n.posicion.y > max_y) max_y = n.posicion.y;
    }
    min_x -= 10.0f; min_y -= 10.0f;
    max_x += 10.0f; max_y += 10.0f;

    if (g.nodos.size() < 150) {
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
        qtree.build(min_x, min_y, max_x, max_y, g.nodos.size());
        for (const auto& n : g.nodos) {
            qtree.insert(n.posicion.x, n.posicion.y, datos[n.id].masa, n.id);
        }

        float theta = 1.2f;

        for (auto& n : g.nodos) {
            auto& d = datos[n.id];
            float fx = 0.0f, fy = 0.0f;
            qtree.calcRepulsion(n.posicion.x, n.posicion.y, d.masa, n.radio, k_rep, theta, p.prevent_overlap, fx, fy, n.id);
            d.dx += fx;
            d.dy += fy;
        }
    }

    // Pre-build reverse edge lookup: O(1) per edge, elimina el O(m²) de la busqueda de dobles
    std::unordered_set<uint64_t> edge_dir;
    edge_dir.reserve(g.aristas.size());
    for (const auto& ae : g.aristas)
        edge_dir.insert(((uint64_t)ae.origen_id << 32) | (uint32_t)ae.destino_id);

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
            atr = logf(1.0f + dist) * weight;
        } else {
            atr = dist * weight;
        }

        if (p.dissuade_hubs) {
            atr /= do_.masa;
        }

        if (!a.es_dirigida) {
            uint64_t rev_key = ((uint64_t)a.destino_id << 32) | (uint32_t)a.origen_id;
            if (edge_dir.count(rev_key)) atr *= 0.5f;
        }

        float atr_x = (dx / dist) * atr;
        float atr_y = (dy / dist) * atr;

        do_.dx -= atr_x;
        do_.dy -= atr_y;
        dd.dx  += atr_x;
        dd.dy  += atr_y;
    }

    for (auto& n : g.nodos) {
        auto& d  = datos[n.id];
        float dx = n.posicion.x - ancla_gravedad.x;
        float dy = n.posicion.y - ancla_gravedad.y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist > 0.01f) {
            float grav = p.gravity * d.masa * 1.5f;

            if (p.strong_gravity) {
                grav = p.gravity * d.masa * 0.1f * dist;
            } else {
                if (dist > 2000.0f) {
                    grav += (dist - 2000.0f) * 0.05f;
                }
            }

            d.dx -= (dx / dist) * grav;
            d.dy -= (dy / dist) * grav;
        }
    }

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
        speed = speed * 0.9f + target * 0.1f;
        speed = std::min(speed, 10.0f);
        speed = std::max(speed, 0.01f);
    }

    for (auto& n : g.nodos) {
        if (n.id == nodo_arrastrado) continue;

        auto& d = datos[n.id];
        float len = sqrtf(d.dx * d.dx + d.dy * d.dy);
        if (len < 0.001f) continue;

        float max_move = speed / (1.0f + speed * sqrtf(global_swing));
        float move = std::min(len * max_move, 15.0f);

        float nx = n.posicion.x + (d.dx / len) * move;
        float ny = n.posicion.y + (d.dy / len) * move;

        if (std::isnan(nx) || std::isinf(nx)) nx = (rand() % 100 - 50);
        if (std::isnan(ny) || std::isinf(ny)) ny = (rand() % 100 - 50);

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

}
