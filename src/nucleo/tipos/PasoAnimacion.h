#pragma once
#include <string>

// Paso de animacion (resultado de un tick de algoritmo)
struct PasoAnimacion {
    enum Accion { VISITAR, EXPLORAR, CONFIRMAR, DESCARTAR, COLOREAR };

    Accion      accion;
    int         nodo_id      = -1;
    int         arista_origen  = -1;
    int         arista_destino = -1;
    std::string descripcion;
};
