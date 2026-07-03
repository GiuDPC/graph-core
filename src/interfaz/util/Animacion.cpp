#include "interfaz/util/Animacion.hpp"
#include "imgui.h"

namespace Animacion {

void reset(EstadoAnimacion& est) {
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

void iniciar(EstadoAnimacion& est, std::vector<PasoAnimacion> pasos) {
    reset(est);
    est.pasos = std::move(pasos);
    est.activa = true;
    est.completa = false;
    est.pausada = false;
    est.paso_actual = -1;
    est.timer_paso = 0.0f;
}

void aplicarPaso(EstadoAnimacion& est, const PasoAnimacion& p) {
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

void deshacerPaso(EstadoAnimacion& est, const PasoAnimacion& p) {
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

void pasoAdelante(EstadoAnimacion& est) {
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

void pasoAtras(EstadoAnimacion& est) {
    if (!est.activa || est.paso_actual < 0) return;
    deshacerPaso(est, est.pasos[est.paso_actual]);
    est.paso_actual--;
    est.timer_paso = 0.0f;
    est.completa = false;
}

void avanzarAuto(EstadoAnimacion& est, float dt) {
    if (!est.activa || est.pausada || est.completa) return;
    est.timer_paso += dt * est.velocidad;
    float intervalo = est.tiempo_entre_pasos;
    if (est.timer_paso >= intervalo) {
        pasoAdelante(est);
        est.timer_paso = 0.0f;
    }
}

std::string obtenerDescripcion(EstadoAnimacion& est) {
    if (est.paso_actual < 0 || est.paso_actual >= (int)est.pasos.size())
        return "";
    return est.pasos[est.paso_actual].descripcion;
}

float obtenerProgreso(EstadoAnimacion& est) {
    if (est.pasos.empty()) return 0.0f;
    return (float)(est.paso_actual + 1) / (float)est.pasos.size();
}

void pausar(EstadoAnimacion& est) {
    if (!est.activa || est.completa) return;
    est.pausada = !est.pausada;
}

void completar(EstadoAnimacion& est) {
    while (!est.completa) pasoAdelante(est);
}

} // namespace Animacion
