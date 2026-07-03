#pragma once

#include "../Grafo.hpp"
#include <vector>

namespace Algoritmos {

struct ResultadoColorFractal {
    std::vector<int> colores_base;
    std::vector<float> modulacion_fractal;
    std::vector<float> morph_fase;
    int num_colores = 0;
};

struct ColorFractal {

    static void mapearMandelbrot(const Grafo& g,
                                  float& min_x, float& max_x,
                                  float& min_y, float& max_y);

    static int iterarMandelbrot(double cx, double cy, int max_iter);

    static ResultadoColorFractal calcular(const Grafo& g,
                                           const std::vector<int>& colores_base,
                                           float tiempo = 0.0f);

    static uint32_t colorFusionado(int color_idx, float mod_fractal,
                                    float morph_fase, int num_colores_totales);
};

}
