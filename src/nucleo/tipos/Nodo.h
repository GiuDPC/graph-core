#pragma once
#include <string>
#include "imgui.h"          // ImVec2 (miembro de datos: necesita definición completa, no solo forward decl)
                             // TODO(refactor): extraer Posicion2D propia del dominio para eliminar dependencia de ImGui
#include "TipoHardware.h"

// Nodo del grafo
struct Nodo {
    int         id;
    std::string nombre;
    ImVec2      posicion;
    float       radio  = 24.0f;
    TipoHardware tipo  = TipoHardware::Servidor;

    Nodo(int _id, ImVec2 _pos, TipoHardware _tipo = TipoHardware::Servidor)
        : id(_id), posicion(_pos), tipo(_tipo)
    {
        radio  = 24.0f;
        nombre = std::string(prefijoHardware(_tipo)) + std::to_string(_id);
    }
};
