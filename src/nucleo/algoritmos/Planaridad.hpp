#pragma once

#include "../Grafo.hpp"
#include <string>
#include <vector>

namespace Algoritmos {

struct ResultadoPlanaridad {
    bool es_planar                = false;
    bool pasa_cota_aristas        = false;
    int  cruces_estimadas         = 0;
    std::string descripcion;
    std::vector<std::pair<int,int>> aristas_cruce;
    bool sospecha_k5              = false;
    bool sospecha_k33             = false;
};

struct Planaridad {
    static bool verificarCotaEuler(const Grafo& g) {
        int V = (int)g.nodos.size();
        int E = (int)g.aristas.size();
        if (V < 3) return true;
        return E <= 3 * V - 6;
    }

    static bool sospecharK33(const Grafo& g) {
        int grado3 = 0;
        for (const auto& n : g.nodos) {
            if (g.gradoNodo(n.id) >= 3) grado3++;
        }
        return grado3 >= 6;
    }

    static bool sospecharK5(const Grafo& g);
    static bool aristasSeCruzan(const Grafo& g, int a1, int a2, int b1, int b2);
    static ResultadoPlanaridad analizar(const Grafo& g);
};

}
