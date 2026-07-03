#pragma once

#include "../Grafo.hpp"
#include "QuadTree.hpp"
#include "imgui.h"
#include <vector>
#include <cmath>
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

    void step(Grafo& g, const ParametrosFA2& p);
};

}
