#pragma once

#include <vector>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {

struct ResultadoColoreo {
    std::vector<int> colores;
    int              num_colores;
};

ResultadoColoreo coloreoGreedy(const Grafo& g);

ResultadoColoreo coloreoWelshPowell(const Grafo& g);

std::vector<PasoAnimacion> generarPasosColoreo(const Grafo& g);

}
