#include "Planaridad.hpp"
#include <set>
#include <algorithm>
#include <cmath>
// planaridad: formula de euler |E| <= 3|V| - 6 para grafos planares
// kuratowski: menores K5 y K3,3, deteccion de cruces O(V)

namespace Algoritmos {

bool Planaridad::sospecharK5(const Grafo& g) {
    int V = (int)g.nodos.size();
    int candidatos = 0;
    for (const auto& n : g.nodos) {
        if (g.gradoNodo(n.id) >= 4) candidatos++;
    }
    if (candidatos >= 5) return true;

    std::vector<int> ids;
    for (const auto& n : g.nodos) ids.push_back(n.id);
    if ((int)ids.size() < 5) return false;

    std::sort(ids.begin(), ids.end(), [&](int a, int b) {
        return g.gradoNodo(a) > g.gradoNodo(b);
    });
    ids.resize(5);
    int aristas_entre = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            auto vecinos = g.vecinos(ids[i]);
            if (std::find(vecinos.begin(), vecinos.end(), ids[j]) != vecinos.end())
                aristas_entre++;
        }
    }
    return aristas_entre >= 7;
}

bool Planaridad::aristasSeCruzan(const Grafo& g, int a1, int a2, int b1, int b2) {
    auto n_a1 = g.obtenerNodo(a1);
    auto n_a2 = g.obtenerNodo(a2);
    auto n_b1 = g.obtenerNodo(b1);
    auto n_b2 = g.obtenerNodo(b2);
    if (!n_a1 || !n_a2 || !n_b1 || !n_b2) return false;

    if (a1 == b1 || a1 == b2 || a2 == b1 || a2 == b2) return false;

    float x1 = n_a1->posicion.x, y1 = n_a1->posicion.y;
    float x2 = n_a2->posicion.x, y2 = n_a2->posicion.y;
    float x3 = n_b1->posicion.x, y3 = n_b1->posicion.y;
    float x4 = n_b2->posicion.x, y4 = n_b2->posicion.y;

    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (fabs(denom) < 0.001f) return false;

    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

    return (t > 0 && t < 1 && u > 0 && u < 1);
}

ResultadoPlanaridad Planaridad::analizar(const Grafo& g) {
    ResultadoPlanaridad res;
    int V = (int)g.nodos.size();
    int E = (int)g.aristas.size();

    res.pasa_cota_aristas = verificarCotaEuler(g);
    res.sospecha_k5 = sospecharK5(g);
    res.sospecha_k33 = sospecharK33(g);

    res.aristas_cruce.clear();
    for (size_t i = 0; i < g.aristas.size(); i++) {
        for (size_t j = i + 1; j < g.aristas.size(); j++) {
            const auto& a = g.aristas[i];
            const auto& b = g.aristas[j];
            if (aristasSeCruzan(g, a.origen_id, a.destino_id, b.origen_id, b.destino_id)) {
                res.aristas_cruce.push_back({(int)i, (int)j});
            }
        }
    }
    res.cruces_estimadas = (int)res.aristas_cruce.size();

    bool tiene_cruces = (res.cruces_estimadas > 0);
    bool pasa_euler = res.pasa_cota_aristas;

    if (!pasa_euler && tiene_cruces) {
        res.es_planar = false;
        res.descripcion = "El grafo NO es planar. Supera la cota de aristas de Euler "
            "(" + std::to_string(E) + " > 3*" + std::to_string(V) + " - 6 = " + std::to_string(3*V-6) + ") "
            "y tiene " + std::to_string(res.cruces_estimadas) + " cruces visibles.";
    } else if (!pasa_euler) {
        res.es_planar = false;
        res.descripcion = "El grafo supera la cota de aristas de Euler "
            "(" + std::to_string(E) + " > 3*" + std::to_string(V) + " - 6 = " + std::to_string(3*V-6) + "). "
            "No puede ser plano, aunque no se vean cruces en este dibujo.";
    } else if (tiene_cruces) {
        res.es_planar = false;
        res.descripcion = "El grafo podria ser planar (pasa cota de Euler), "
            "pero tiene " + std::to_string(res.cruces_estimadas) + " cruces en este dibujo. "
            "Quizas pueda redibujarse sin cruces — prueba con 'Distribuir nodos'.";
    } else {
        res.es_planar = true;
        std::string extra;
        if (res.sospecha_k5) extra += " Se detectaron posibles subdivisiones de K5 (5 nodos de alto grado).";
        if (res.sospecha_k33) extra += " Se detectaron posibles subdivisiones de K3,3 (6+ nodos de grado >=3).";
        if (!extra.empty()) extra = " (con advertencia:" + extra + ")";
        res.descripcion = "El grafo ES planar." + extra + " "
            "Por el teorema de los 4 colores, puede colorearse con solo 4 colores. "
            "|E|=" + std::to_string(E) + " <= 3·" + std::to_string(V) + "-6=" + std::to_string(3*V-6);
    }

    return res;
}

}
