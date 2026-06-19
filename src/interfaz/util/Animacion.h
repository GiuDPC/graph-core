#pragma once

#include "imgui.h"
#include "Easing.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include <set>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>

// Particula animada que viaja entre nodos
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
    float       tiempo_entre_pasos = 0.8f; // segundos entre pasos automáticos (AeroGrafos)
    float       velocidad         = 1.0f;  // multiplicador de velocidad (AeroGrafos)
    float       velocidad_paso    = 0.5f;  // legacy: usado por modo Grafos original
    bool        activa            = false;
    bool        pausada           = false;
    bool        completa          = false; // true cuando todos los pasos se ejecutaron

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
    est.completa = false;
    est.velocidad = 1.0f;
    est.tiempo_entre_pasos = 0.8f;
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
    est.completa = false;
    est.pausada = false;
    est.paso_actual = -1;
    est.timer_paso = 0.0f;
}

// Aplicar un paso individual (adelante)
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
        case PasoAnimacion::COLOREAR:
            if (p.nodo_id >= 0) {
                est.visitados.insert(p.nodo_id);
                est.procesando.erase(p.nodo_id);
            }
            break;
    }
}

// Retroceder un paso (deshacer el último)
inline void deshacerPaso(EstadoAnimacion& est, const PasoAnimacion& p) {
    switch (p.accion) {
        case PasoAnimacion::VISITAR:
            if (p.nodo_id >= 0) {
                est.procesando.erase(p.nodo_id);
                est.visitados.erase(p.nodo_id);
                est.tiempo_visita_nodo.erase(p.nodo_id);
            }
            break;
        case PasoAnimacion::CONFIRMAR:
            if (p.nodo_id >= 0) {
                est.procesando.insert(p.nodo_id);
                est.visitados.erase(p.nodo_id);
            }
            if (p.arista_origen >= 0 && p.arista_destino >= 0) {
                est.confirmadas.erase({p.arista_origen, p.arista_destino});
                est.exploradas.insert({p.arista_origen, p.arista_destino});
            }
            break;
        case PasoAnimacion::EXPLORAR:
            if (p.arista_origen >= 0 && p.arista_destino >= 0)
                est.exploradas.erase({p.arista_origen, p.arista_destino});
            break;
        case PasoAnimacion::DESCARTAR:
            if (p.arista_origen >= 0 && p.arista_destino >= 0)
                est.descartadas.erase({p.arista_origen, p.arista_destino});
            break;
        case PasoAnimacion::COLOREAR:
            if (p.nodo_id >= 0) {
                est.visitados.erase(p.nodo_id);
            }
            break;
    }
}

// Avanzar un paso
inline void pasoAdelante(EstadoAnimacion& est) {
    if (!est.activa || est.completa) return;
    int siguiente = est.paso_actual + 1;
    if (siguiente >= (int)est.pasos.size()) {
        est.completa = true;
        est.pausada = true;
        return;
    }
    est.paso_actual = siguiente;
    aplicarPaso(est, est.pasos[siguiente]);
    est.timer_paso = 0.0f;
}

// Retroceder un paso
inline void pasoAtras(EstadoAnimacion& est) {
    if (!est.activa || est.paso_actual < 0) return;
    deshacerPaso(est, est.pasos[est.paso_actual]);
    est.paso_actual--;
    est.timer_paso = 0.0f;
    est.completa = false;
}

// Avanzar automáticamente según el tiempo transcurrido
inline void avanzarAuto(EstadoAnimacion& est, float dt) {
    if (!est.activa || est.pausada || est.completa) return;
    est.timer_paso += dt * est.velocidad;
    float intervalo = est.tiempo_entre_pasos;
    if (est.timer_paso >= intervalo) {
        pasoAdelante(est);
        est.timer_paso = 0.0f;
    }
}

// Obtener descripción del paso actual
inline std::string obtenerDescripcion(EstadoAnimacion& est) {
    if (est.paso_actual < 0 || est.paso_actual >= (int)est.pasos.size())
        return "";
    return est.pasos[est.paso_actual].descripcion;
}

// Obtener progreso 0..1
inline float obtenerProgreso(EstadoAnimacion& est) {
    if (est.pasos.empty()) return 0.0f;
    return (float)(est.paso_actual + 1) / (float)est.pasos.size();
}

// Pausar/reanudar animación
inline void pausar(EstadoAnimacion& est) {
    if (!est.activa || est.completa) return;
    est.pausada = !est.pausada;
}

// Saltar al final
inline void completar(EstadoAnimacion& est) {
    while (!est.completa) pasoAdelante(est);
}

} // namespace Animacion
