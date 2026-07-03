#pragma once

#include <vector>
#include <string>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.hpp"

namespace Algoritmos {
namespace DFS {

struct ResultadoDFS {
    std::vector<int>                       orden_visita;
    std::vector<std::pair<int,int>>        back_edges;
};

ResultadoDFS dfs(const Grafo& g, int inicio_id);

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int inicio_id);

}
}
