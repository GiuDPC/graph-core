#pragma once

#include <vector>
#include <functional>

#include "../Grafo.hpp"
#include "../tipos/PasoAnimacion.hpp"

namespace Algoritmos {

struct ResultadoDijkstra {
    std::vector<int>   ruta;
    std::vector<float> distancias;
    bool               hay_ruta = false;
    float              costo_total = 0.0f;
    int                saltos = 0;
};

ResultadoDijkstra dijkstra(const Grafo& g, int id_origen, int id_destino,
                           bool aplicar_latencia = false,
                           bool aplicar_escala = false,
                           std::function<bool(int)> nodo_valido = nullptr,
                           std::function<bool(int, int)> arista_valida = nullptr);

std::vector<PasoAnimacion> generarPasos(const Grafo& g, int id_origen, int id_destino,
                                         bool aplicar_latencia = false,
                                         bool aplicar_escala = false,
                                         std::function<bool(int)> nodo_valido = nullptr,
                                         std::function<bool(int, int)> arista_valida = nullptr);

}
