#pragma once

#include "imgui.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include <set>
#include <map>
#include <vector>
#include <string>


struct ParticulaAnimacion {
    bool   activa     = false;
    ImVec2 pos_inicio;
    ImVec2 pos_fin;
    float  progreso   = 0.0f;
    float  duracion   = 0.3f;
    ImU32  color      = 0;
    float  radio      = 6.0f;
};

namespace Animacion {

struct EstadoAnimacion {
    std::vector<PasoAnimacion> pasos;
    int         paso_actual       = -1;
    float       timer_paso        = 0.0f;
    float       tiempo_entre_pasos = 0.8f;
    float       velocidad         = 1.0f;
    float       velocidad_paso    = 0.5f;
    bool        activa            = false;
    bool        pausada           = false;
    bool        completa          = false;

    std::set<int> visitados;
    std::set<int> procesando;
    std::set<std::pair<int,int>> exploradas;
    std::set<std::pair<int,int>> confirmadas;
    std::set<std::pair<int,int>> descartadas;

    ParticulaAnimacion particula;
    std::map<int, float> tiempo_visita_nodo;
};

void reset(EstadoAnimacion& est);
void iniciar(EstadoAnimacion& est, std::vector<PasoAnimacion> pasos);
void aplicarPaso(EstadoAnimacion& est, const PasoAnimacion& p);
void deshacerPaso(EstadoAnimacion& est, const PasoAnimacion& p);
void pasoAdelante(EstadoAnimacion& est);
void pasoAtras(EstadoAnimacion& est);
void avanzarAuto(EstadoAnimacion& est, float dt);
std::string obtenerDescripcion(EstadoAnimacion& est);
float obtenerProgreso(EstadoAnimacion& est);
void pausar(EstadoAnimacion& est);
void completar(EstadoAnimacion& est);

} // namespace Animacion
