#include "QuadTree.hpp"

namespace Algoritmos {

void QuadTree::build(float min_x, float min_y, float max_x, float max_y, size_t expected_nodes) {
    pool.clear();
    pool.reserve(std::max((size_t)16384, expected_nodes * 8));
    root = allocNode(min_x, min_y, max_x, max_y);
}

void QuadTree::insert_rec(int node_idx, float x, float y, float m, int id, int depth) {
    if (node_idx == -1) return;
    if (!pool[node_idx].contains(x, y)) return;

    if (pool[node_idx].mass == 0) {
        pool[node_idx].cm_x = x;
        pool[node_idx].cm_y = y;
        pool[node_idx].mass = m;
        pool[node_idx].nodo_id = id;
        return;
    }

    if (depth >= max_depth) {
        float tm = pool[node_idx].mass + m;
        pool[node_idx].cm_x = (pool[node_idx].cm_x * pool[node_idx].mass + x * m) / tm;
        pool[node_idx].cm_y = (pool[node_idx].cm_y * pool[node_idx].mass + y * m) / tm;
        pool[node_idx].mass = tm;
        pool[node_idx].nodo_id = -1;
        return;
    }

    if (pool[node_idx].NW == -1) {
        float mx = (pool[node_idx].x_min + pool[node_idx].x_max) * 0.5f;
        float my = (pool[node_idx].y_min + pool[node_idx].y_max) * 0.5f;
        pool[node_idx].NW = allocNode(pool[node_idx].x_min, pool[node_idx].y_min, mx, my);
        pool[node_idx].NE = allocNode(mx, pool[node_idx].y_min, pool[node_idx].x_max, my);
        pool[node_idx].SW = allocNode(pool[node_idx].x_min, my, mx, pool[node_idx].y_max);
        pool[node_idx].SE = allocNode(mx, my, pool[node_idx].x_max, pool[node_idx].y_max);

        if (pool[node_idx].nodo_id != -1) {
            float old_x = pool[node_idx].cm_x;
            float old_y = pool[node_idx].cm_y;
            if (std::abs(old_x - x) < 0.1f && std::abs(old_y - y) < 0.1f) {
                old_x += ((rand() % 100) - 50) * 0.01f;
                old_y += ((rand() % 100) - 50) * 0.01f;
            }
            // ponytail: solo recursar en el cuadrante correcto, no en los 4
            float omx = (pool[node_idx].x_min + pool[node_idx].x_max) * 0.5f;
            float omy = (pool[node_idx].y_min + pool[node_idx].y_max) * 0.5f;
            int o_child = (old_x < omx) ? ((old_y < omy) ? pool[node_idx].NW : pool[node_idx].SW)
                                        : ((old_y < omy) ? pool[node_idx].NE : pool[node_idx].SE);
            insert_rec(o_child, old_x, old_y, pool[node_idx].mass, pool[node_idx].nodo_id, depth + 1);
            pool[node_idx].nodo_id = -1;
        }
    }

    { // ponytail: solo recursar en el cuadrante correcto
        float mx2 = (pool[node_idx].x_min + pool[node_idx].x_max) * 0.5f;
        float my2 = (pool[node_idx].y_min + pool[node_idx].y_max) * 0.5f;
        int child = (x < mx2) ? ((y < my2) ? pool[node_idx].NW : pool[node_idx].SW)
                              : ((y < my2) ? pool[node_idx].NE : pool[node_idx].SE);
        insert_rec(child, x, y, m, id, depth + 1);
    }

    float total_mass = pool[node_idx].mass + m;
    pool[node_idx].cm_x = (pool[node_idx].cm_x * pool[node_idx].mass + x * m) / total_mass;
    pool[node_idx].cm_y = (pool[node_idx].cm_y * pool[node_idx].mass + y * m) / total_mass;
    pool[node_idx].mass = total_mass;
}

void QuadTree::calcRepulsion_rec(int node_idx, float target_x, float target_y, float target_mass, float target_radio,
                                  float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id) {
    if (node_idx == -1) return;
    if (pool[node_idx].mass == 0) return;

    float dx = target_x - pool[node_idx].cm_x;
    float dy = target_y - pool[node_idx].cm_y;
    float dist = sqrtf(dx*dx + dy*dy);

    if (dist < 0.01f) {
        dx = (rand() % 100 - 50) * 0.01f;
        dy = (rand() % 100 - 50) * 0.01f;
        dist = sqrtf(dx*dx + dy*dy) + 0.01f;
    }

    float s = std::max(pool[node_idx].x_max - pool[node_idx].x_min,
                       pool[node_idx].y_max - pool[node_idx].y_min);

    if (pool[node_idx].NW == -1 || (s / dist < theta)) {
        if (pool[node_idx].nodo_id != target_id) {
            float rep = k_rep * (target_mass * pool[node_idx].mass) / dist;
            if (prevent_overlap) {
                float min_d = target_radio * 2.0f + 2.0f;
                if (dist < min_d) {
                    rep *= 10.0f;
                }
            }
            out_fx += (dx / dist) * rep;
            out_fy += (dy / dist) * rep;
        }
    } else {
        calcRepulsion_rec(pool[node_idx].NW, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
        calcRepulsion_rec(pool[node_idx].NE, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
        calcRepulsion_rec(pool[node_idx].SW, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
        calcRepulsion_rec(pool[node_idx].SE, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
    }
}

} // namespace Algoritmos
