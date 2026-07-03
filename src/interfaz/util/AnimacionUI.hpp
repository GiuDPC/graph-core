#pragma once

#include <vector>

class Interfaz;
struct PasoAnimacion;

namespace AnimacionUI {
    void iniciar(Interfaz& self, std::vector<PasoAnimacion> pasos);
    void finalizar(Interfaz& self);
    void aplicarPaso(Interfaz& self, const PasoAnimacion& p);
    void reset(Interfaz& self);
}
