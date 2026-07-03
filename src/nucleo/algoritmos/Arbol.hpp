#pragma once

#include <vector>
#include <map>
#include <string>

#include "../Grafo.hpp"

namespace Algoritmos {
namespace Arbol {

struct PropiedadesArbol {
    bool es_arbol = false;
    std::string razon_no_arbol;

    int raiz_id = -1;

    std::map<int, int>              nivel;
    std::map<int, int>              padre;
    std::map<int, std::vector<int>> hijos;
    std::map<int, std::vector<int>> ancestros;
    std::map<int, std::vector<int>> descendientes;

    int                  altura      = 0;
    int                  grado_arbol = 0;
    std::vector<int>     hojas;
    std::vector<int>     rama_mas_larga;
    std::vector<int>     rama_mas_corta;

    std::vector<std::pair<int,int>> primos;
    std::map<int, std::vector<int>> hermanos;
};

bool verificarEsArbol(const Grafo& g, std::string& razon);

PropiedadesArbol analizar(const Grafo& g, int raiz_id);

}
}
