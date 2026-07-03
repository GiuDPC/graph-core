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
    int NW, NE, SW, SE;

    void init(float x1, float y1, float x2, float y2) {
        x_min = x1; y_min = y1; x_max = x2; y_max = y2;
        cm_x = 0; cm_y = 0; mass = 0; nodo_id = -1;
        NW = -1; NE = -1; SW = -1; SE = -1;
    }

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

    void insert_rec(int node_idx, float x, float y, float m, int id, int depth);
    void calcRepulsion_rec(int node_idx, float target_x, float target_y, float target_mass, float target_radio,
                           float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id);

public:
    QuadTree() {
        pool.reserve(16384);
        max_depth = 20;
    }

    void clear() {
        pool.clear();
    }

    void build(float min_x, float min_y, float max_x, float max_y, size_t expected_nodes);

    void insert(float x, float y, float m, int id) {
        insert_rec(root, x, y, m, id, 0);
    }

    void calcRepulsion(float target_x, float target_y, float target_mass, float target_radio,
                       float k_rep, float theta, bool prevent_overlap, float& out_fx, float& out_fy, int target_id) {
        calcRepulsion_rec(root, target_x, target_y, target_mass, target_radio, k_rep, theta, prevent_overlap, out_fx, out_fy, target_id);
    }
};

} // namespace Algoritmos
