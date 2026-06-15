#pragma once

#include "audio/Sonidos.h"
#include "interfaz/util/Animacion.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include <vector>

class Interfaz;

// Wrappers de animación que agregan sonido y log y efectos visuales
namespace AnimacionUI {

inline void iniciar(Interfaz& self, std::vector<PasoAnimacion> pasos) {
    Animacion::iniciar(self.estado_grafos.anim_estado, std::move(pasos));
    if (!self.estado_grafos.anim_estado.pasos.empty()) {
        self.registrarLog("🎬 Animación iniciada: " +
            std::to_string(self.estado_grafos.anim_estado.pasos.size()) + " pasos");
        g_sonidos.reproducir(Sonidos::CLICK_MENU);
    }
}

inline void finalizar(Interfaz& self) {
    if (self.estado_ui.herramienta_activa == EstadoUI::CatRutas)
        g_sonidos.reproducir(Sonidos::TRIUNFO_DIJKSTRA);
    else
        g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
    self.registrarLog("✅ Algoritmo completado (" +
        std::to_string(self.estado_grafos.anim_estado.pasos.size()) + " pasos)");
}

inline void aplicarPaso(Interfaz& self, const PasoAnimacion& p) {
    Animacion::aplicarPaso(self.estado_grafos.anim_estado, p);

    if (self.estado_grafos.anim_estado.velocidad_paso < 0.15f) {
        if (p.accion == PasoAnimacion::CONFIRMAR)
            g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
        else if (p.accion == PasoAnimacion::DESCARTAR)
            g_sonidos.reproducir(Sonidos::DESCARTAR);
    } else {
        switch (p.accion) {
            case PasoAnimacion::VISITAR:
                g_sonidos.reproducir(Sonidos::VISITAR_NODO);
                break;
            case PasoAnimacion::CONFIRMAR:
                g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                break;
            case PasoAnimacion::EXPLORAR:
                if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes)
                    g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
                else
                    g_sonidos.reproducir(Sonidos::VISITAR_NODO);
                break;
            case PasoAnimacion::DESCARTAR:
                g_sonidos.reproducir(Sonidos::DESCARTAR);
                break;
            case PasoAnimacion::COLOREAR:
                if (p.nodo_id >= 0 && p.nodo_id < (int)self.estado_grafos.resultado_coloreo.colores.size()) {
                    if (self.estado_grafos.colores_nodos.size() < self.estado_grafos.resultado_coloreo.colores.size())
                        self.estado_grafos.colores_nodos.resize(self.estado_grafos.resultado_coloreo.colores.size(), -1);
                    self.estado_grafos.colores_nodos[p.nodo_id] = self.estado_grafos.resultado_coloreo.colores[p.nodo_id];
                }
                g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                break;
        }
    }

    if (!p.descripcion.empty()) self.registrarLog(p.descripcion);
}

inline void reset(Interfaz& self) {
    Animacion::reset(self.estado_grafos.anim_estado);
    self.registrarLog("⏹ Animación reiniciada");
}

}
