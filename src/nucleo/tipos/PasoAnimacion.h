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
    int         color_asignado = -1;
    
    // Datos extendidos para animaciones ricas en proyector
    float       distancia_acumulada = -1.0f; // Para Dijkstra (carteles de km)
    int         nivel_profundidad   = -1;    // Para BFS/DFS (ondas/niveles)
};
