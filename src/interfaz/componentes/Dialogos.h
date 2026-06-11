#pragma once

#include "interfaz/Interfaz.h"
#include "persistencia/SerializadorJSON.h"

// Diálogos modales: Acerca de, fallback de carga/guardado
namespace Dialogos {

// ── Modal "Acerca de" ─────────────────────────────────────────────────────
void acercaDe() {
    if (ImGui::BeginPopupModal("Acerca de", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("graph-core v1.0");
        ImGui::Text("Motor avanzado de visualizacion de grafos");
        ImGui::Text("Con audio y simulacion mejorada");
        ImGui::Separator();
        if (ImGui::Button("Cerrar")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

// ── Fallback de carga (cuando pfd falla) ──────────────────────────────────
inline void fallbackCargar(Interfaz& self, Grafo& red) {
    if (ImGui::BeginPopupModal("FallbackCargar", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("No se pudo abrir el dialogo del sistema.");
        ImGui::Text("Escribi la ruta del archivo .json manualmente:");
        static char ruta_cargar[256] = {};
        ImGui::SetNextItemWidth(400);
        ImGui::InputText("##rutaCargar", ruta_cargar, sizeof(ruta_cargar));
        ImGui::Spacing();
        if (ImGui::Button("Cargar", ImVec2(120, 0))) {
            if (strlen(ruta_cargar) > 0) {
                Persistencia::cargar(red, std::string(ruta_cargar));
                Animacion::reset(self.anim_estado);
                self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
                self.registrarLog("[OK] Cargado desde ruta manual: " + std::string(ruta_cargar));
                memset(ruta_cargar, 0, sizeof(ruta_cargar));
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

// ── Fallback de guardado (cuando pfd falla) ───────────────────────────────
inline void fallbackGuardar(Interfaz& self, Grafo& red) {
    if (ImGui::BeginPopupModal("FallbackGuardar", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("No se pudo abrir el dialogo del sistema.");
        ImGui::Text("Asegurate de que la ruta sea valida o relativa al binario:");
        static char ruta_guardar[256] = {};
        ImGui::SetNextItemWidth(400);
        ImGui::InputText("##rutaGuardar", ruta_guardar, sizeof(ruta_guardar));
        ImGui::Spacing();
        if (ImGui::Button("Guardar", ImVec2(120, 0))) {
            if (strlen(ruta_guardar) > 0) {
                std::string ruta = ruta_guardar;
                if (ruta.find(".json") == std::string::npos) ruta += ".json";
                Persistencia::guardar(red, ruta);
                self.registrarLog("[OK] Guardado en ruta manual: " + ruta);
                memset(ruta_guardar, 0, sizeof(ruta_guardar));
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

} // namespace Dialogos
