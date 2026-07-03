#pragma once

#include "../Grafo.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace Algoritmos {

// Coloreo con modulacion fractal 
struct ResultadoColorFractal {
    std::vector<int> colores_base;      //colores originales
    std::vector<float> modulacion_fractal; // 0.0-1.0 por nodo
    std::vector<float> morph_fase;      // fase para morphing animado
    int num_colores = 0;
};

struct ColorFractal {

    // mapeo de la posicion 2d al plano de mandelbrot
    static void mapearMandelbrot(const Grafo& g,
                                  float& min_x, float& max_x,
                                  float& min_y, float& max_y) {
        min_x = min_y = 1e9f;
        max_x = max_y = -1e9f;
        for (const auto& n : g.nodos) {
            if (n.posicion.x < min_x) min_x = n.posicion.x;
            if (n.posicion.x > max_x) max_x = n.posicion.x;
            if (n.posicion.y < min_y) min_y = n.posicion.y;
            if (n.posicion.y > max_y) max_y = n.posicion.y;
        }
        float cx = (min_x + max_x) * 0.5f;
        float cy = (min_y + max_y) * 0.5f;
        float r = std::max(max_x - min_x, max_y - min_y) * 0.5f;
        if (r < 1.0f) r = 1.0f;
        r *= 1.2f;
        min_x = cx - r; max_x = cx + r;
        min_y = cy - r; max_y = cy + r;
    }

    // iteraciones de mandelbrot para un punto
    static int iterarMandelbrot(double cx, double cy, int max_iter) {
        double zx = 0.0, zy = 0.0;
        int iter = 0;
        while (iter < max_iter) {
            double zx2 = zx * zx - zy * zy + cx;
            double zy2 = 2.0 * zx * zy + cy;
            zx = zx2;
            zy = zy2;
            if (zx * zx + zy * zy > 4.0) break;
            iter++;
        }
        return iter;
    }

    // calcular para todos los nodos del grafo
    static ResultadoColorFractal calcular(const Grafo& g,
                                           const std::vector<int>& colores_base,
                                           float tiempo = 0.0f) {
        ResultadoColorFractal res;
        res.colores_base = colores_base;
        int n = (int)g.nodos.size();
        res.modulacion_fractal.resize(n);
        res.morph_fase.resize(n);

        if (n == 0) return res;

        float min_x, max_x, min_y, max_y;
        mapearMandelbrot(g, min_x, max_x, min_y, max_y);

        double cx_offset = sin(tiempo * 0.1) * 0.3;
        double cy_offset = cos(tiempo * 0.13) * 0.3;

        int max_iter = 64;

        for (int i = 0; i < n; i++) {
            if (i >= (int)g.nodos.size()) break;
            const auto& nodo = g.nodos[i];

            // mapeo posicion del nodo al plano de mandelbrot (-2..1, -1.5..1.5)
            double cx = -0.7 + (nodo.posicion.x - min_x) / (max_x - min_x) * 2.5 - 0.5;
            double cy = (nodo.posicion.y - min_y) / (max_y - min_y) * 2.5 - 1.25;

            cx += cx_offset;
            cy += cy_offset;

            int iter = iterarMandelbrot(cx, cy, max_iter);

            float v = (float)iter / max_iter;
            v = v * v * (3.0f - 2.0f * v);  // smoothstep
            res.modulacion_fractal[i] = std::clamp(v, 0.0f, 1.0f);

            res.morph_fase[i] = fmod(tiempo * 0.05f + i * 0.1f, 1.0f);
        }

        // Contar colores unicos
        std::vector<int> unicos = colores_base;
        std::sort(unicos.begin(), unicos.end());
        unicos.erase(std::unique(unicos.begin(), unicos.end()), unicos.end());
        res.num_colores = (int)unicos.size();

        return res;
    }

    // Obtener color final para un nodo
    static uint32_t colorFusionado(int color_idx, float mod_fractal,
                                    float morph_fase, int num_colores_totales) {
        struct HSV { float h, s, v; };
        static const HSV paleta[] = {
            {0.00f, 0.75f, 0.90f},  // Rojo
            {0.33f, 0.70f, 0.80f},  // Verde
            {0.58f, 0.75f, 0.85f},  // Azul
            {0.12f, 0.80f, 0.95f},  // Amarillo
            {0.83f, 0.65f, 0.85f},  // Magenta
            {0.50f, 0.70f, 0.85f},  // Cyan
        };

        float h, s, v;
        if (color_idx < 6) {
            h = paleta[color_idx].h;
            s = paleta[color_idx].s;
            v = paleta[color_idx].v;
        } else {
            h = (color_idx % 12) / 12.0f;
            s = 0.7f;
            v = 0.8f;
        }

        float mod = mod_fractal;
        s = std::clamp(s * (0.6f + mod * 0.5f), 0.3f, 1.0f);
        v = std::clamp(v * (0.7f + mod * 0.4f), 0.3f, 1.0f);

        h = fmod(h + mod * 0.05f + morph_fase * 0.02f, 1.0f);

        // HSV to RGB
        float r, g, b;
        int hi = (int)(h * 6.0f);
        float f = h * 6.0f - hi;
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));
        switch (hi % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }

        return ((uint32_t)(255) << 24) |
               ((uint32_t)(r * 255) << 16) |
               ((uint32_t)(g * 255) << 8) |
               ((uint32_t)(b * 255));
    }
};

}
