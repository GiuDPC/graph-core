#pragma once

#include "imgui.h"

class Interfaz;
class Grafo;

// Panel de matrices de adyacencia e incidencia
namespace Matrices {
    void dibujar(Grafo& red, Interfaz& self);
}
