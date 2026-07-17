#include "interfaz/util/AtajosTeclado.hpp"
#include "../../nucleo/Grafo.hpp"
#include "../../nucleo/HistorialGrafos.hpp"
#include "../estado/EstadoUI.hpp"
#include "../estado/EstadoGrafos.hpp"
#include <imgui.h>

namespace AtajosTeclado {

void procesar(Grafo& red, HistorialGrafos& historial, EstadoUI& ui, EstadoGrafos& estado_grafos) {
    ImGuiIO& io = ImGui::GetIO();
    
    if (io.WantTextInput) return;

    bool ctrl  = io.KeyCtrl;
    bool shift = io.KeyShift;

    // [Ctrl+Z] Deshacer
    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (historial.deshacer(red)) {
            ui.nodo_seleccionado = -1;
            ui.nodo_hover = -1;
        }
    }
    
    // [Ctrl+Y] o [Ctrl+Shift+Z] Rehacer
    if ((ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Y)) || 
        (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Z))) {
        if (historial.rehacer(red)) {
            ui.nodo_seleccionado = -1;
            ui.nodo_hover = -1;
        }
    }
    
    // [Ctrl+N] Nuevo grafo
    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_N)) {
        if (!red.nodos.empty()) {
            historial.capturar(red);
            red.limpiar();
            ui.nodo_seleccionado = -1;
            estado_grafos.grafo_iso_g2.limpiar();
            estado_grafos.resultado_iso.son_isomorfos = false;
            estado_grafos.iso_analizado = false;
        }
    }
    
    // [Delete] o [Backspace] Borrar nodo/arista seleccionado
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (ui.nodo_seleccionado != -1) {
            historial.capturar(red);
            red.eliminarNodo(ui.nodo_seleccionado);
            ui.nodo_seleccionado = -1;
        }
    }
    
    // [Ctrl+A] Seleccionar todos los nodos
    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_A)) {
        // placeholder
    }
}

} // namespace AtajosTeclado
