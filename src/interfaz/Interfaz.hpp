#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.hpp"
#include "nucleo/tipos/PasoAnimacion.h"
#include "estado/EstadoGrafos.hpp"
#include "estado/EstadoRedes.hpp"
#include "estado/EstadoAeroGrafos.hpp"
#include "estado/EstadoUI.hpp"
#include "nucleo/HistorialGrafos.hpp"
#include "audio/Sonidos.h"
#include "interfaz/util/Animacion.h"
#include <GLFW/glfw3.h>

extern Sonidos g_sonidos;

class Interfaz {
public:
    using ModoPanel = EstadoGrafos::ModoPanel;
    using ModoApp   = EstadoUI::ModoApp;

    EstadoGrafos      estado_grafos;
    EstadoRedes       estado_redes;
    EstadoAeroGrafos  estado_aerografos;
    EstadoUI          estado_ui;
    HistorialGrafos   historial;

    ModoApp ultimo_modo_workspace = ModoApp::Grafos;

    void dibujar(Grafo& red, GLFWwindow* ventana);
    void construirLayout(ImGuiID dock_id, ImVec2 tamano);

    void registrarLog(const std::string& msg) {
        estado_ui.registrarLog(msg);
    }

    void resetGrafoIsomorfismo() {
        estado_grafos.resetGrafoIsomorfismo();
    }
};
