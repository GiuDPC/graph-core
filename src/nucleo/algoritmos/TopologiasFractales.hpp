#pragma once

#include "../Grafo.hpp"
#include <vector>
#include <cmath>
#include <string>

namespace Algoritmos {

struct TopologiasFractales {

    static Grafo generarSierpinski(int iteraciones, float x_centro = 0.0f, float y_centro = 0.0f, float lado = 600.0f);

    static Grafo generarMandala(int capas, int ramas, float x_centro = 0.0f, float y_centro = 0.0f, float radio_max = 300.0f);

    static Grafo generarArbolFractal(int niveles, float x_raiz = 0.0f, float y_raiz = 300.0f, float longitud_ini = 150.0f);

    static Grafo generarKoch(int iteraciones, float x_centro = 0.0f, float y_centro = 0.0f, float radio = 300.0f);

    static Grafo generarMallaHexagonal(int capas, float x_centro = 0.0f, float y_centro = 0.0f, float lado = 40.0f);
};

}
