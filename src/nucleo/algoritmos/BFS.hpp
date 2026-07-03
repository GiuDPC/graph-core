#pragma once

#include <vector>
#include <map>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.h"

namespace Algoritmos {
namespace BFS {

struct ResultadoBFS {
    std::vector<int>    orden_visita;
    std::map<int, int>  nivel;
    std::map<int, int>  padre;
};

ResultadoBFS bfs(const Grafo& g, int inicio_id);

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int inicio_id);

}
}
