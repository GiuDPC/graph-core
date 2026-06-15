#pragma once

#include "../Grafo.h"
#include <vector>
#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Algoritmos {

struct TopologiasFractales {

    // Generador de Grafo basado en el Triángulo de Sierpinski
    static Grafo generarSierpinski(int iteraciones, float x_centro = 0.0f, float y_centro = 0.0f, float lado = 600.0f) {
        Grafo g;
        
        // El triangulo inicial (iteracion 0)
        // Vertices del triangulo equilátero
        float h = lado * std::sqrt(3.0f) / 2.0f;
        
        // v0: arriba, v1: abajo izquierda, v2: abajo derecha
        ImVec2 v0(x_centro, y_centro - h/2.0f);
        ImVec2 v1(x_centro - lado/2.0f, y_centro + h/2.0f);
        ImVec2 v2(x_centro + lado/2.0f, y_centro + h/2.0f);
        
        g.agregarNodo(v0); int n0 = g.contador_ids - 1; g.nodos.back().nombre = "V0";
        g.agregarNodo(v1); int n1 = g.contador_ids - 1; g.nodos.back().nombre = "V1";
        g.agregarNodo(v2); int n2 = g.contador_ids - 1; g.nodos.back().nombre = "V2";
        
        g.agregarArista(n0, n1, 1.0f);
        g.agregarArista(n1, n2, 1.0f);
        g.agregarArista(n2, n0, 1.0f);

        // Subdivisión iterativa
        // En cada iteración, para cada triángulo (representado por 3 nodos conectados)
        // insertamos 3 nuevos nodos en el punto medio de sus aristas y los conectamos.
        // Pero en forma de grafo, Sierpinski graph nivel n:
        // Se puede generar construyendo 3 copias del grafo de nivel n-1 y conectándolas por las "esquinas".
        
        // Mejor enfoque algorítmico para grafos de Sierpinski (S_n):
        // Nivel 0: 1 nodo (S_0) ? No, típicamente S_1 es un triángulo K3.
        // Vamos a usar un enfoque de reemplazo iterativo de triángulos.
        
        struct Triangulo { int a, b, c; };
        std::vector<Triangulo> triangulos = {{n0, n1, n2}};

        for (int it = 0; it < iteraciones; it++) {
            std::vector<Triangulo> nuevos_triangulos;
            std::vector<Arista> aristas_a_borrar;
            
            for (const auto& t : triangulos) {
                // Encontrar puntos medios
                auto p_a = g.obtenerNodo(t.a)->posicion;
                auto p_b = g.obtenerNodo(t.b)->posicion;
                auto p_c = g.obtenerNodo(t.c)->posicion;
                
                ImVec2 m_ab((p_a.x + p_b.x)/2.0f, (p_a.y + p_b.y)/2.0f);
                ImVec2 m_bc((p_b.x + p_c.x)/2.0f, (p_b.y + p_c.y)/2.0f);
                ImVec2 m_ca((p_c.x + p_a.x)/2.0f, (p_c.y + p_a.y)/2.0f);
                
                // Agregar nodos medios
                g.agregarNodo(m_ab); int n_ab = g.contador_ids - 1;
                g.agregarNodo(m_bc); int n_bc = g.contador_ids - 1;
                g.agregarNodo(m_ca); int n_ca = g.contador_ids - 1;
                
                // Conectar nodos medios para formar el triángulo central invertido
                g.agregarArista(n_ab, n_bc, 1.0f);
                g.agregarArista(n_bc, n_ca, 1.0f);
                g.agregarArista(n_ca, n_ab, 1.0f);
                
                // Conectar las esquinas a los nodos medios
                g.agregarArista(t.a, n_ab, 1.0f);
                g.agregarArista(t.a, n_ca, 1.0f);
                
                g.agregarArista(t.b, n_ab, 1.0f);
                g.agregarArista(t.b, n_bc, 1.0f);
                
                g.agregarArista(t.c, n_bc, 1.0f);
                g.agregarArista(t.c, n_ca, 1.0f);
                
                // Registrar las 3 nuevas sub-estructuras para la próxima iteración
                nuevos_triangulos.push_back({t.a, n_ab, n_ca});
                nuevos_triangulos.push_back({t.b, n_ab, n_bc});
                nuevos_triangulos.push_back({t.c, n_bc, n_ca});
                
                // Eliminar las aristas exteriores viejas que ahora estan divididas
                aristas_a_borrar.push_back({t.a, t.b, 0});
                aristas_a_borrar.push_back({t.b, t.c, 0});
                aristas_a_borrar.push_back({t.c, t.a, 0});
            }
            
            // Eliminar aristas viejas subdivididas
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

        // Renombrar nodos secuencialmente para que quede limpio
        for (size_t i = 0; i < g.nodos.size(); i++) {
            g.nodos[i].nombre = "F" + std::to_string(i+1);
        }

        return g;
    }
    // generador mandala (planar, para 4 colores)
    static Grafo generarMandala(int capas, int ramas, float x_centro = 0.0f, float y_centro = 0.0f, float radio_max = 300.0f) {
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
                angulo += (i * (float)M_PI / ramas); // desfase en espiral
                
                float x = x_centro + r * std::cos(angulo);
                float y = y_centro + r * std::sin(angulo);
                
                g.agregarNodo(ImVec2(x, y));
                g.nodos.back().nombre = "M" + std::to_string(i+1) + "-" + std::to_string(j);
                anillos[i].push_back(g.contador_ids - 1);
            }
        }

        // formar triangulacion planar
        for (int i = 0; i < capas; i++) {
            for (int j = 0; j < ramas; j++) {
                int u = anillos[i][j];
                int v_sig = anillos[i][(j + 1) % ramas];
                g.agregarArista(u, v_sig, 1.0f); // anillo

                if (i == 0) {
                    g.agregarArista(centro, u, 1.0f);
                } else {
                    int u_int = anillos[i-1][j];
                    g.agregarArista(u, u_int, 1.0f);
                    
                    int v_int = anillos[i-1][(j + 1) % ramas];
                    g.agregarArista(v_sig, u_int, 1.0f); // triangulacion cruzada
                }
            }
        }
        return g;
        return g;
    }

    // ── arbol fractal (arbol en y) ──────────────────────────────────────────
    static Grafo generarArbolFractal(int niveles, float x_raiz = 0.0f, float y_raiz = 300.0f, float longitud_ini = 150.0f) {
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

            // dos nuevas ramas
            ramas.push_back({ nuevo_id, x_fin, y_fin, r.angulo - 0.5f, r.longitud * 0.7f, r.nivel + 1 });
            ramas.push_back({ nuevo_id, x_fin, y_fin, r.angulo + 0.5f, r.longitud * 0.7f, r.nivel + 1 });
        }
        return g;
    }

    // ── copo de nieve de koch ───────────────────────────────────────────────
    static Grafo generarKoch(int iteraciones, float x_centro = 0.0f, float y_centro = 0.0f, float radio = 300.0f) {
        Grafo g;
        // triangulo inicial invertido
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

    // ── malla hexagonal (panal) ─────────────────────────────────────────────
    static Grafo generarMallaHexagonal(int capas, float x_centro = 0.0f, float y_centro = 0.0f, float lado = 40.0f) {
        Grafo g;
        float h = std::sqrt(3.0f) * lado;

        // para no repetir nodos
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

        for (int q = -capas; q <= capas; q++) {
            int r1 = std::max(-capas, -q - capas);
            int r2 = std::min(capas, -q + capas);
            for (int r = r1; r <= r2; r++) {
                int id_actual = get_id(q, r);
                // conectar con vecinos (solo 3 direcciones para evitar duplicados en grafo no dirigido)
                if (r < r2) g.agregarArista(id_actual, get_id(q, r + 1), 1.0f);
                if (q < capas && r > -q - capas) g.agregarArista(id_actual, get_id(q + 1, r - 1), 1.0f);
                if (q < capas && r < -q + capas) g.agregarArista(id_actual, get_id(q + 1, r), 1.0f);
            }
        }
        return g;
    }
};

} // namespace Algoritmos
