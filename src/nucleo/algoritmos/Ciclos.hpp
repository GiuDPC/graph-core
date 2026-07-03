#pragma once

#include <vector>
#include <string>

#include "../Grafo.hpp"

namespace Algoritmos {

struct ResultadoCiclos {
    bool                            tiene_ciclo = false;
    std::vector<std::pair<int,int>> aristas_ciclo;
    std::string                     descripcion;
};

ResultadoCiclos detectarCiclos(const Grafo& g);

ResultadoCiclos detectarCiclosDirigidos(const Grafo& g);

}
