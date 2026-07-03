#pragma once

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.hpp"
#include <vector>

namespace Algoritmos {

struct EulerHamilton {

    static std::vector<int> buscarCaminoEuleriano(Grafo& g, int origen_sugerido = 0);

    static std::vector<PasoAnimacion> generarPasosEuler(Grafo& g, int origen_sugerido = 0);

    struct ResultadoHamilton {
        std::vector<int> ruta;
        float distancia_total;
    };

    static std::vector<int> buscarCaminoHamiltoniano(const Grafo& g);

    static ResultadoHamilton buscarCaminoHamiltonianoHeuristico(const Grafo& g, int origen_sugerido = 0);

    static std::vector<PasoAnimacion> generarPasosHamiltonHeuristico(const Grafo& g, int origen_sugerido = 0);
};

}
