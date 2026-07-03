#pragma once

#include <vector>
#include <map>
#include <string>
#include "../Grafo.hpp"

namespace Algoritmos {
namespace Isomorfismo {

struct ResultadoIsomorfismo {
    bool                            son_isomorfos = false;
    std::vector<std::pair<int,int>> mapeo;
    std::string                     descripcion;
    int                             nivel_confianza = 0;
    bool misma_cantidad_nodos   = false;
    bool misma_cantidad_aristas = false;
    bool misma_secuencia_grados = false;
};

struct ResultadoIsoGeometrico {
    bool son_isomorfos = false;
    std::string descripcion;
    float error_maximo = 0.0f;
    std::vector<std::pair<int,int>> mapeo;
};

std::vector<int> secuenciaGrados(const Grafo& g);

ResultadoIsomorfismo verificarPorFirma(const Grafo& g1, const Grafo& g2);

ResultadoIsomorfismo verificarExacto(const Grafo& g1, const Grafo& g2);

ResultadoIsomorfismo verificar(const Grafo& g1, const Grafo& g2);

ResultadoIsoGeometrico verificarGeometrico(const Grafo& g1, const Grafo& g2);

}
}
