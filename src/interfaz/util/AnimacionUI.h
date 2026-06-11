#pragma once

#include "audio/Sonidos.h"
#include "interfaz/util/Animacion.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include <vector>

class Interfaz;

// Wrappers de animacion que agregan efectos secundarios (log, sonido)
// sobre las funciones puras de Animacion::.
namespace AnimacionUI {

inline void iniciar(Interfaz& self, std::vector<PasoAnimacion> pasos) {
    Animacion::iniciar(self.anim_estado, std::move(pasos));
    if (!self.anim_estado.pasos.empty()) {
        self.registrarLog("Animacion iniciada: " + std::to_string(self.anim_estado.pasos.size()) + " pasos");
        g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
    }
}

inline void aplicarPaso(Interfaz& self, const PasoAnimacion& p) {
    Animacion::aplicarPaso(self.anim_estado, p);
    switch (p.accion) {
        case PasoAnimacion::VISITAR:   g_sonidos.reproducir(Sonidos::VISITAR_NODO); break;
        case PasoAnimacion::CONFIRMAR: g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA); break;
        case PasoAnimacion::EXPLORAR:  g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO); break;
        case PasoAnimacion::DESCARTAR: g_sonidos.reproducir(Sonidos::DESCARTAR); break;
    }
    if (!p.descripcion.empty()) self.registrarLog(p.descripcion);
}

inline void reset(Interfaz& self) {
    Animacion::reset(self.anim_estado);
}

} // namespace AnimacionUI
