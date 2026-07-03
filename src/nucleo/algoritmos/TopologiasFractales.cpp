#include "TopologiasFractales.hpp"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Algoritmos {

Grafo TopologiasFractales::generarSierpinski(int iteraciones, float x_centro, float y_centro, float lado) {
    Grafo g;

    float h = lado * std::sqrt(3.0f) / 2.0f;

    ImVec2 v0(x_centro, y_centro - h/2.0f);
    ImVec2 v1(x_centro - lado/2.0f, y_centro + h/2.0f);
    ImVec2 v2(x_centro + lado/2.0f, y_centro + h/2.0f);

    g.agregarNodo(v0); int n0 = g.contador_ids - 1; g.nodos.back().nombre = "V0";
    g.agregarNodo(v1); int n1 = g.contador_ids - 1; g.nodos.back().nombre = "V1";
    g.agregarNodo(v2); int n2 = g.contador_ids - 1; g.nodos.back().nombre = "V2";

    g.agregarArista(n0, n1, 1.0f);
    g.agregarArista(n1, n2, 1.0f);
    g.agregarArista(n2, n0, 1.0f);

    struct Triangulo { int a, b, c; };
    std::vector<Triangulo> triangulos = {{n0, n1, n2}};

    for (int it = 0; it < iteraciones; it++) {
        std::vector<Triangulo> nuevos_triangulos;
        std::vector<Arista> aristas_a_borrar;

        for (const auto& t : triangulos) {
            auto p_a = g.obtenerNodo(t.a)->posicion;
            auto p_b = g.obtenerNodo(t.b)->posicion;
            auto p_c = g.obtenerNodo(t.c)->posicion;

            ImVec2 m_ab((p_a.x + p_b.x)/2.0f, (p_a.y + p_b.y)/2.0f);
            ImVec2 m_bc((p_b.x + p_c.x)/2.0f, (p_b.y + p_c.y)/2.0f);
            ImVec2 m_ca((p_c.x + p_a.x)/2.0f, (p_c.y + p_a.y)/2.0f);

            g.agregarNodo(m_ab); int n_ab = g.contador_ids - 1;
            g.agregarNodo(m_bc); int n_bc = g.contador_ids - 1;
            g.agregarNodo(m_ca); int n_ca = g.contador_ids - 1;

            g.agregarArista(n_ab, n_bc, 1.0f);
            g.agregarArista(n_bc, n_ca, 1.0f);
            g.agregarArista(n_ca, n_ab, 1.0f);

            g.agregarArista(t.a, n_ab, 1.0f);
            g.agregarArista(t.a, n_ca, 1.0f);

            g.agregarArista(t.b, n_ab, 1.0f);
            g.agregarArista(t.b, n_bc, 1.0f);

            g.agregarArista(t.c, n_bc, 1.0f);
            g.agregarArista(t.c, n_ca, 1.0f);

            nuevos_triangulos.push_back({t.a, n_ab, n_ca});
            nuevos_triangulos.push_back({t.b, n_ab, n_bc});
            nuevos_triangulos.push_back({t.c, n_bc, n_ca});

            aristas_a_borrar.push_back({t.a, t.b, 0});
            aristas_a_borrar.push_back({t.b, t.c, 0});
            aristas_a_borrar.push_back({t.c, t.a, 0});
        }

        for (const auto& ab : aristas_a_borrar) {
            for (auto it = g.aristas.begin(); it != g.aristas.end(); ++it) {
                if ((it->origen_id == ab.origen_id && it->destino_id == ab.destino_id) ||
                    (it->origen_id == ab.destino_id && it->destino_id == ab.origen_id)) {
                    g.aristas.erase(it);
                    break;
                }
            }
        }

        triangulos = nuevos_triangulos;
    }

    for (size_t i = 0; i < g.nodos.size(); i++) {
        g.nodos[i].nombre = "F" + std::to_string(i+1);
        g.nodos[i].radio = std::max(4.0f, 20.0f - (iteraciones * 3.5f));
    }

    return g;
}

Grafo TopologiasFractales::generarMandala(int capas, int ramas, float x_centro, float y_centro, float radio_max) {
    Grafo g;
    g.agregarNodo(ImVec2(x_centro, y_centro));
    g.nodos.back().nombre = "M0";
    int centro = g.contador_ids - 1;

    float dr = radio_max / capas;
    std::vector<std::vector<int>> anillos(capas);

    for (int i = 0; i < capas; i++) {
        float r = (i + 1) * dr;
        for (int j = 0; j < ramas; j++) {
            float angulo = (2.0f * (float)M_PI * j) / ramas;
            angulo += (i * (float)M_PI / ramas);

            float x = x_centro + r * std::cos(angulo);
            float y = y_centro + r * std::sin(angulo);

            g.agregarNodo(ImVec2(x, y));
            g.nodos.back().nombre = "M" + std::to_string(i+1) + "-" + std::to_string(j);
            anillos[i].push_back(g.contador_ids - 1);
        }
    }

    for (int i = 0; i < capas; i++) {
        for (int j = 0; j < ramas; j++) {
            int u = anillos[i][j];
            int v_sig = anillos[i][(j + 1) % ramas];
            g.agregarArista(u, v_sig, 1.0f);

            if (i == 0) {
                g.agregarArista(centro, u, 1.0f);
            } else {
                int u_int = anillos[i-1][j];
                g.agregarArista(u, u_int, 1.0f);

                int v_int = anillos[i-1][(j + 1) % ramas];
                g.agregarArista(v_sig, u_int, 1.0f);
            }
        }
    }

    for (size_t i = 0; i < g.nodos.size(); i++) {
        g.nodos[i].radio = std::max(4.0f, 20.0f - (capas * 3.0f));
    }
    return g;
}

Grafo TopologiasFractales::generarArbolFractal(int niveles, float x_raiz, float y_raiz, float longitud_ini) {
    Grafo g;
    g.agregarNodo(ImVec2(x_raiz, y_raiz));
    g.nodos.back().nombre = "R";

    struct Rama { int id_padre; float x, y, angulo, longitud; int nivel; };
    std::vector<Rama> ramas = {{ 0, x_raiz, y_raiz, -(float)M_PI / 2.0f, longitud_ini, 0 }};

    while (!ramas.empty()) {
        Rama r = ramas.back();
        ramas.pop_back();

        if (r.nivel >= niveles) continue;

        float x_fin = r.x + std::cos(r.angulo) * r.longitud;
        float y_fin = r.y + std::sin(r.angulo) * r.longitud;

        g.agregarNodo(ImVec2(x_fin, y_fin));
        int nuevo_id = g.contador_ids - 1;
        g.nodos.back().nombre = "N" + std::to_string(nuevo_id);
        g.agregarArista(r.id_padre, nuevo_id, 1.0f);

        ramas.push_back({ nuevo_id, x_fin, y_fin, r.angulo - 0.5f, r.longitud * 0.7f, r.nivel + 1 });
        ramas.push_back({ nuevo_id, x_fin, y_fin, r.angulo + 0.5f, r.longitud * 0.7f, r.nivel + 1 });
    }

    for (size_t i = 0; i < g.nodos.size(); i++) {
        g.nodos[i].radio = std::max(4.0f, 20.0f - (niveles * 3.0f));
    }
    return g;
}

Grafo TopologiasFractales::generarKoch(int iteraciones, float x_centro, float y_centro, float radio) {
    Grafo g;

    for (int i = 0; i < 3; i++) {
        float a = (float)M_PI / 2.0f + i * (2.0f * (float)M_PI / 3.0f);
        g.agregarNodo(ImVec2(x_centro + std::cos(a) * radio, y_centro + std::sin(a) * radio));
        g.nodos.back().nombre = "K" + std::to_string(i);
    }

    struct Segmento { int a, b; };
    std::vector<Segmento> segs = {{0, 1}, {1, 2}, {2, 0}};

    for (int it = 0; it < iteraciones; it++) {
        std::vector<Segmento> nuevos_segs;
        for (const auto& s : segs) {
            ImVec2 pA = g.obtenerNodo(s.a)->posicion;
            ImVec2 pB = g.obtenerNodo(s.b)->posicion;

            ImVec2 p1(pA.x + (pB.x - pA.x) / 3.0f, pA.y + (pB.y - pA.y) / 3.0f);
            ImVec2 p2(pA.x + 2.0f * (pB.x - pA.x) / 3.0f, pA.y + 2.0f * (pB.y - pA.y) / 3.0f);

            float angulo = std::atan2(pB.y - pA.y, pB.x - pA.x) - (float)M_PI / 3.0f;
            float d = std::sqrt((pB.x - pA.x) * (pB.x - pA.x) + (pB.y - pA.y) * (pB.y - pA.y)) / 3.0f;
            ImVec2 pm(p1.x + std::cos(angulo) * d, p1.y + std::sin(angulo) * d);

            g.agregarNodo(p1); int n1 = g.contador_ids - 1;
            g.agregarNodo(pm); int nm = g.contador_ids - 1;
            g.agregarNodo(p2); int n2 = g.contador_ids - 1;

            g.nodos[n1].nombre = "K" + std::to_string(n1);
            g.nodos[nm].nombre = "K" + std::to_string(nm);
            g.nodos[n2].nombre = "K" + std::to_string(n2);

            nuevos_segs.push_back({s.a, n1});
            nuevos_segs.push_back({n1, nm});
            nuevos_segs.push_back({nm, n2});
            nuevos_segs.push_back({n2, s.b});
        }
        segs = nuevos_segs;
    }

    for (const auto& s : segs) {
        g.agregarArista(s.a, s.b, 1.0f);
    }
    return g;
}

Grafo TopologiasFractales::generarMallaHexagonal(int capas, float x_centro, float y_centro, float lado) {
    Grafo g;
    float h = std::sqrt(3.0f) * lado;

    auto get_id = [&](int q, int r) -> int {
        float x = x_centro + lado * 3.0f / 2.0f * q;
        float y = y_centro + h * (r + q / 2.0f);
        for (const auto& n : g.nodos) {
            if (std::abs(n.posicion.x - x) < 1.0f && std::abs(n.posicion.y - y) < 1.0f)
                return n.id;
        }
        g.agregarNodo(ImVec2(x, y));
        g.nodos.back().nombre = "H" + std::to_string(g.contador_ids - 1);
        return g.contador_ids - 1;
    };

    auto en_limites = [&](int nq, int nr) {
        return nq >= -capas && nq <= capas &&
               nr >= -capas && nr <= capas &&
               (nq + nr) >= -capas && (nq + nr) <= capas;
    };

    for (int q = -capas; q <= capas; q++) {
        int r1 = std::max(-capas, -q - capas);
        int r2 = std::min(capas, -q + capas);
        for (int r = r1; r <= r2; r++) {
            int id_actual = get_id(q, r);
            if (en_limites(q, r + 1)) g.agregarArista(id_actual, get_id(q, r + 1), 1.0f);
            if (en_limites(q + 1, r - 1)) g.agregarArista(id_actual, get_id(q + 1, r - 1), 1.0f);
            if (en_limites(q + 1, r)) g.agregarArista(id_actual, get_id(q + 1, r), 1.0f);
        }
    }
    return g;
}

}
