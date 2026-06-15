#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "nucleo/tipos/TipoHardware.h"

class Interfaz;
class Grafo;

// Panel de despliegue de hardware
namespace PanelHardware {

inline const char* icono(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return ICON_FA_SERVER;
        case TipoHardware::Router:   return ICON_FA_NETWORK_WIRED;
        case TipoHardware::Switch:   return ICON_FA_RIGHT_LEFT;
        case TipoHardware::Firewall: return ICON_FA_SHIELD_HALVED;
        case TipoHardware::Terminal: return ICON_FA_DESKTOP;
        default: return ICON_FA_CIRCLE;
    }
}

inline void desplegar(Interfaz& self, Grafo& red, const ImVec2& pos_click) {
    if (ImGui::BeginPopup("CrearEquipo")) {
        ImGui::Text(ICON_FA_PLUS " Nuevo Equipamiento");
        ImGui::Separator();
        const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
        const char* iconos[] = {ICON_FA_SERVER, ICON_FA_NETWORK_WIRED, ICON_FA_RIGHT_LEFT, ICON_FA_SHIELD_HALVED, ICON_FA_DESKTOP};
        for (int i = 0; i < 5; i++) {
            char label[64];
            snprintf(label, sizeof(label), "%s %s", iconos[i], tipos[i]);
            if (ImGui::Selectable(label)) {
                red.agregarNodo(pos_click, (TipoHardware)i);
                self.registrarLog("Hardware desplegado: " + std::string(tipos[i]) + " " + red.nodos.back().nombre);
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            }
        }
        ImGui::EndPopup();
    }
}

}
