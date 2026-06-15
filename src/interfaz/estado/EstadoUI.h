#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <unordered_map>

struct EstadoUI {
    enum class ModoApp { Grafos, Redes };
    ModoApp modo_actual = ModoApp::Grafos;

    enum Categoria {
        CatGeneral,
        CatRutas,
        CatArbol,
        CatBusqueda,
        CatCiclos,
        CatColoreo,
        CatIsomorfismo,
        CatMatrices,
        CatFractales,
        CatEulerHamilton
    };
    Categoria herramienta_activa = CatGeneral;

    //  seleccion 
    int  nodo_seleccionado = -1;
    int  nodo_hover        = -1;
    bool arrastrando       = false;

    //  panning y zoom 
    ImVec2 offset_lienzo = ImVec2(0, 0);
    ImVec2 pan_velocity  = ImVec2(0, 0);
    float  zoom_velocity = 0.0f;

    //  creacion pendiente 
    bool   creando_arista_drag        = false;
    int    drag_arista_origen         = -1;
    ImVec2 pos_click_derecho          = ImVec2(0, 0);
    int    pendiente_arista_origen    = -1;
    int    pendiente_arista_destino   = -1;
    float  pendiente_arista_peso      = 1.0f;
    bool   pendiente_arista_dirigida  = false;
    char   buffer_nombre[64]          = {};

    //  logs 
    std::vector<std::string> system_logs;

    void registrarLog(const std::string& msg) {
        system_logs.push_back("[SYS] " + msg);
        if (system_logs.size() > 100) system_logs.erase(system_logs.begin());
    }

    // enciclopedia / ayuda 
    bool mostrar_ventana_ayuda = false;
    int seccion_ayuda_actual = 0;

    bool  fisicas_activas = false;
    bool  fisicas_estado_cambiado = false;
    std::unordered_map<int, ImVec2> fisicas_posiciones_guardadas;

    bool aristas_dirigidas = false;  // toggle global: true = nuevas aristas se crean dirigidas

    // fuentes
    ImFont* fontMono = nullptr;
    
    // logo 
    unsigned int id_logo = 0;
    int width_logo = 0;
    int height_logo = 0;
    
    // flags de dialogo 
    bool mostrar_acerca_de = false;
};
