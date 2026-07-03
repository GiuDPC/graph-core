#pragma once

#include "interfaz/estado/EstadoUI.hpp"

struct Toolbar {
    static bool dibujar(EstadoUI& ui, bool en_modo_aero);

private:
    static bool botonCat(const char* id, const char* label_con_icono,
                         EstadoUI::Categoria cat, EstadoUI& ui);
};
