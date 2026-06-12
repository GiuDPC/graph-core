#pragma once

#include "imgui.h"
#include <string>
#include <vector>

// estado completo de interfaz de usuario
struct EstadoUI {
    // -- modo de aplicacion --
    enum class ModoApp { Grafos, Redes };
    ModoApp modo_actual = ModoApp::Grafos;

    // -- categorias de herramientas (toolbar) --
    enum Categoria {
        CatGeneral,       // ningun tool seleccionado
        // grafos
        CatRutas,
        CatArbol,
        CatBusqueda,
        CatCiclos,
        CatColoreo,
        CatIsomorfismo,
        CatMatrices,
        CatFractales,
        CatEulerHamilton,
        // redes
        CatTopologia,
        CatSimular,
        CatStats,
        CatEventos,
        CatFallos
    };
    Categoria herramienta_activa = CatGeneral;

    // -- seleccion --
    int  nodo_seleccionado = -1;
    int  nodo_hover        = -1;
    bool arrastrando       = false;

    // -- panning --
    ImVec2 offset_lienzo = ImVec2(0, 0);

    // -- creacion pendiente --
    ImVec2 pos_click_derecho          = ImVec2(0, 0);
    int    pendiente_arista_origen    = -1;
    int    pendiente_arista_destino   = -1;
    float  pendiente_arista_peso      = 1.0f;
    char   buffer_nombre[64]          = {};

    // -- logs --
    std::vector<std::string> system_logs;

    void registrarLog(const std::string& msg) {
        system_logs.push_back("[SYS] " + msg);
        if (system_logs.size() > 100) system_logs.erase(system_logs.begin());
    }

    // -- fuentes --
    ImFont* fontMono = nullptr;
    
    // -- logo --
    unsigned int id_logo = 0;
    int width_logo = 0;
    int height_logo = 0;
    
    // -- flags de dialogo --
    bool mostrar_acerca_de = false;
};
