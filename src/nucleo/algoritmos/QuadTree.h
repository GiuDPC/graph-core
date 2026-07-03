#pragma once
#include "../Grafo.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Algoritmos {

struct QuadNode {
    float x_min, y_min, x_max, y_max;
    float cm_x, cm_y;
    float mass;
    int nodo_id;
    int NW, NE, SW, SE; // Indices in the memory pool

    void init(float x1, float y1, float x2, float y2) {
        x_min = x1; y_min = y1; x_max = x2; y_max = y2;
        cm_x = 0; cm_y = 0; mass = 0; nodo_id = -1;
        NW = -1; NE = -1; SW = -1; SE = -1;
    }

    // Half-open interval [min, max) — a node exactly on the right/bottom
    // boundary goes to the adjacent quadrant, never to both.
    bool contains(float x, float y) const {
        return x >= x_min && x < x_max && y >= y_min && y < y_max;
    }
};

class QuadTree {
private:
    std::vector<QuadNode> pool;
    int root;
    int max_depth;

    int allocNode(float x1, float y1, float x2, float y2) {
        int idx = (int)pool.size();
        pool.emplace_back();
        pool[idx].init(x1, y1, x2, y2);
        return idx;
    }

    // Use ONLY int indices — never QuadNode& references — because
    // allocNode() may reallocate the vector and invalidate all refs.
    void insert_rec(int node_idx, float x, float y, float m, int id, int depth) {
        if (node_idx == -1) return;
        if (!pool[node_idx].contains(x, y)) return;

        if (pool[node_idx].mass == 0) {
            pool[node_idx].cm_x = x;
            pool[node_idx].cm_y = y;
            pool[node_idx].mass = m;
            pool[node_idx].nodo_id = id;
            return;
        }

        // Safety: max depth prevents stack overflow with coincident nodes
        if (depth >= max_depth) {
            // Merge masses instead of subdividing further
            float tm = pool[node_idx].mass + m;
            pool[node_idx].cm_x = (pool[node_idx].cm_x * pool[node_idx].mass + x * m) / tm;
            pool[node_idx].cm_y = (pool[node_idx].cm_y * pool[node_idx].mass + y * m) / tm;
            pool[node_idx].mass = tm;
            pool[node_idx].nodo_id = -1;
            return;
        }

        if (pool[node_idx].NW == -1) {
            float mx = (pool[node_idx].x_min + pool[node_idx].x_max) / 2.0f;
            float my = (pool[node_idx].y_min + pool[node_idx].y_max) / 2.0f;
            pool[node_idx].NW = allocNode(pool[node_idx].x_min, pool[node_idx].y_min, mx, my);
            pool[node_idx].NE = allocNode(mx, pool[node_idx].y_min, pool[node_idx].x_max, my);
            pool[node_idx].SW = allocNode(pool[node_idx].x_min, my, mx, pool[node_idx].y_max);
            pool[node_idx].SE = allocNode(mx, my, pool[node_idx].x_max, pool[node_idx].y_max);

            // Reinsert existing node (may be at same position — jitter to avoid boundary pileup)
            if (pool[node_idx].nodo_id != -1) {
                float old_x = pool[node_idx].cm_x;
                float old_y = pool[node_idx].cm_y;
                if (std::abs(old_x - x) < 0.1f && std::abs(old_y - y) < 0.1f) {
                    old_x += ((rand() % 100) - 50) * 0.01f;
                    old_y += ((rand() % 100) - 50) * 0.01f;
                }
                insert_rec(pool[node_idx].NW, old_x, old_y, pool[node_idx].mass, pool[node_idx].nodo_id, depth + 1);
                insert_rec(pool[node_idx].NE, old_x, old_y, pool[node_idx].mass, pool[node_idx].nodo_id, depth + 1);
                insert_rec(pool[node_idx].SW, old_x, old_y, pool[node_idx].mass, pool[node_idx].nodo_id, depth + 1);
                insert_rec(pool[node_idx].SE, old_x, old_y, pool[node_idx].mass, pool[node_idx].nodo_id, depth + 1);
                pool[node_idx].nodo_id = -1;
            }
        }

        insert_rec(pool[node_idx].NW, x, y, m, id, depth + 1);
        insert_rec(pool[node_idx].NE, x, y, m, id, depth + 1);
        insert_rec(pool[node_idx].SW, x, y, m, id, depth + 1);
        insert_rec(pool[node_idx].SE, x, y, m, id, depth + 1);

        // Re-aggregate center of mass
        float total_mass = pool[node_idx].mass + m;
        pool[node_idx].cm_x = (pool[node_idx].cm_x * pool[node_idx].mass + x * m) / total_mass;
        pool[node_idx].cm_y = (pool[node_idx].cm_y * pool[node_idx].mass + y * m) / total_mass;
        pool[node_idx].mass = total_mass;
    }

    void calcRepulsion_rec(int node_idx, float target_x, float target_y, float target_mass, float target_radio, 
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

public:
    QuadTree() {
        pool.reserve(16384);
        max_depth = 20;
    }
    
    void clear() {
        pool.clear();
    }

    void build(float min_x, float min_y, float max_x, float max_y, size_t expected_nodes) {
        pool.clear();
        pool.reserve(std::max((size_t)16384, expected_nodes * 8));
        root = allocNode(min_x, min_y, max_x, max_y);
    }

    void insert(float x, float y, float m, int id) {
        insert_rec(root, x, y, m, id, 0);
    }
    
    void calcRepulsion(float target_x, float target_y, float target_mass, float target_radio, 
                       float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id) {
        calcRepulsion_rec(root, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
    }
};

} // namespace Algoritmos
