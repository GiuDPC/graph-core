#pragma once

#include <vector>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {
namespace Kruskal {

struct ResultadoKruskal {
    std::vector<Arista> aristas_mst;
    float               peso_total = 0.0f;
    int                 aristas_aceptadas = 0;
    int                 aristas_rechazadas = 0;
};

ResultadoKruskal kruskal(const Grafo& g);

std::vector<PasoAnimacion> generarPasos(const Grafo& g);

}
}
