#pragma once

#include "imgui.h"
#include "Easing.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include <set>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>

// Particular animada que viaja entre nodos
struct ParticulaAnimacion {
    bool   activa     = false;
    ImVec2 pos_inicio;
    ImVec2 pos_fin;
    float  progreso   = 0.0f;
    float  duracion   = 0.3f;
    ImU32  color      = IM_COL32(0, 255, 200, 255);
    float  radio      = 6.0f;
};

namespace Animacion {

// Estado completo de animación
struct EstadoAnimacion {
    std::vector<PasoAnimacion> pasos;
    int         paso_actual       = -1;
    float       timer_paso        = 0.0f;
    float       velocidad_paso    = 0.5f;
    bool        activa            = false;
    bool        pausada           = false;

    std::set<int> visitados;
    std::set<int> procesando;
    std::set<std::pair<int,int>> exploradas;
    std::set<std::pair<int,int>> confirmadas;
    std::set<std::pair<int,int>> descartadas;

    ParticulaAnimacion particula;
    std::map<int, float> tiempo_visita_nodo;
};

// Resetear animación por completo
inline void reset(EstadoAnimacion& est) {
    est.pasos.clear();
    est.paso_actual = -1;
    est.timer_paso = 0.0f;
    est.activa = false;
    est.pausada = false;
    est.visitados.clear();
    est.procesando.clear();
    est.exploradas.clear();
    est.confirmadas.clear();
    est.descartadas.clear();
    est.particula.activa = false;
    est.tiempo_visita_nodo.clear();
}

// Iniciar una nueva animación
inline void iniciar(EstadoAnimacion& est, std::vector<PasoAnimacion> pasos) {
    reset(est);
    est.pasos = std::move(pasos);
    est.activa = true;
    est.pausada = false;
    est.paso_actual = -1;
    est.timer_paso = 0.0f;
}

// Aplicar un paso de animación
inline void aplicarPaso(EstadoAnimacion& est, const PasoAnimacion& p) {
    switch (p.accion) {
        case PasoAnimacion::VISITAR:
            if (p.nodo_id >= 0) {
                est.procesando.insert(p.nodo_id);
                est.visitados.insert(p.nodo_id);
                est.tiempo_visita_nodo[p.nodo_id] = (float)ImGui::GetTime();
            }
            break;
        case PasoAnimacion::CONFIRMAR:
            if (p.nodo_id >= 0) {
                est.procesando.erase(p.nodo_id);
                est.visitados.insert(p.nodo_id);
            }
            if (p.arista_origen >= 0 && p.arista_destino >= 0) {
                est.confirmadas.insert({p.arista_origen, p.arista_destino});
                est.exploradas.erase({p.arista_origen, p.arista_destino});
            }
            break;
        case PasoAnimacion::EXPLORAR:
            if (p.arista_origen >= 0 && p.arista_destino >= 0)
                est.exploradas.insert({p.arista_origen, p.arista_destino});
            break;
        case PasoAnimacion::DESCARTAR:
            if (p.arista_origen >= 0 && p.arista_destino >= 0)
                est.descartadas.insert({p.arista_origen, p.arista_destino});
            break;
    }
}

// Pausar/reanudar animación
inline void pausar(EstadoAnimacion& est) {
    est.pausada = !est.pausada;
}

} 
