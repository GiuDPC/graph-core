#pragma once
#include "../Grafo.h"
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

    bool contains(float x, float y) const {
        return x >= x_min && x <= x_max && y >= y_min && y <= y_max;
    }
};

class QuadTree {
private:
    std::vector<QuadNode> pool;
    int root;

    int allocNode(float x1, float y1, float x2, float y2) {
        int idx = pool.size();
        pool.emplace_back();
        pool[idx].init(x1, y1, x2, y2);
        return idx;
    }

    void insert_rec(int node_idx, float x, float y, float m, int id) {
        QuadNode& node = pool[node_idx];
        if (!node.contains(x, y)) return;

        if (node.mass == 0) {
            node.cm_x = x;
            node.cm_y = y;
            node.mass = m;
            node.nodo_id = id;
            return;
        }

        if (node.NW == -1) {
            float mx = (node.x_min + node.x_max) / 2.0f;
            float my = (node.y_min + node.y_max) / 2.0f;
            node.NW = allocNode(node.x_min, node.y_min, mx, my);
            node.NE = allocNode(mx, node.y_min, node.x_max, my);
            node.SW = allocNode(node.x_min, my, mx, node.y_max);
            node.SE = allocNode(mx, my, node.x_max, node.y_max);
            
            // Refetch reference after allocNode (std::vector might reallocate)
            QuadNode& current = pool[node_idx];

            if (current.nodo_id != -1) {
                float old_x = current.cm_x, old_y = current.cm_y;
                if (std::abs(old_x - x) < 0.1f && std::abs(old_y - y) < 0.1f) {
                    old_x += ((rand() % 100) - 50) * 0.01f;
                    old_y += ((rand() % 100) - 50) * 0.01f;
                }
                insert_rec(current.NW, old_x, old_y, current.mass, current.nodo_id);
                insert_rec(current.NE, old_x, old_y, current.mass, current.nodo_id);
                insert_rec(current.SW, old_x, old_y, current.mass, current.nodo_id);
                insert_rec(current.SE, old_x, old_y, current.mass, current.nodo_id);
                
                // Re-fetch since insert_rec can alloc
                pool[node_idx].nodo_id = -1;
            }
        }

        QuadNode& current = pool[node_idx];
        insert_rec(current.NW, x, y, m, id);
        insert_rec(current.NE, x, y, m, id);
        insert_rec(current.SW, x, y, m, id);
        insert_rec(current.SE, x, y, m, id);

        // Re-fetch current again
        QuadNode& updated = pool[node_idx];
        float total_mass = updated.mass + m;
        updated.cm_x = (updated.cm_x * updated.mass + x * m) / total_mass;
        updated.cm_y = (updated.cm_y * updated.mass + y * m) / total_mass;
        updated.mass = total_mass;
    }

    void calcRepulsion_rec(int node_idx, float target_x, float target_y, float target_mass, float target_radio, 
                       float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id) {
        if (node_idx == -1) return;
        const QuadNode& node = pool[node_idx];
        if (node.mass == 0) return;
        
        float dx = target_x - node.cm_x;
        float dy = target_y - node.cm_y;
        float dist = sqrtf(dx*dx + dy*dy);
        
        if (dist < 0.01f) {
            dx = (rand() % 100 - 50) * 0.01f;
            dy = (rand() % 100 - 50) * 0.01f;
            dist = sqrtf(dx*dx + dy*dy) + 0.01f;
        }

        float s = std::max(node.x_max - node.x_min, node.y_max - node.y_min);
        
        if (node.NW == -1 || (s / dist < theta)) {
            if (node.nodo_id != target_id) {
                float rep = k_rep * (target_mass * node.mass) / dist;
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
            calcRepulsion_rec(node.NW, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
            calcRepulsion_rec(node.NE, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
            calcRepulsion_rec(node.SW, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
            calcRepulsion_rec(node.SE, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
        }
    }

public:
    QuadTree() {
        pool.reserve(4096);
    }
    
    void clear() {
        pool.clear();
    }

    void build(float min_x, float min_y, float max_x, float max_y, size_t expected_nodes) {
        pool.clear();
        // Reserve extra space to avoid reallocation during insert
        pool.reserve(expected_nodes * 4);
        root = allocNode(min_x, min_y, max_x, max_y);
    }

    void insert(float x, float y, float m, int id) {
        insert_rec(root, x, y, m, id);
    }
    
    void calcRepulsion(float target_x, float target_y, float target_mass, float target_radio, 
                       float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id) {
        calcRepulsion_rec(root, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
    }
};

} // namespace Algoritmos
