#pragma once

#include "interfaz/Interfaz.h"
#include "persistencia/SerializadorJSON.h"

// Diálogos modales: Acerca de, fallback de carga/guardado
namespace Dialogos {

// ── Modal "Acerca de" ─────────────────────────────────────────────────────
void acercaDe() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    
    if (ImGui::BeginPopupModal("Acerca de", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Spacing();
        
        // Titulo
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 0.7f, 1.0f));
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("graphCore").x) * 0.5f);
        ImGui::Text("graphCore");
        ImGui::PopStyleColor();
        
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("graph-core v3.0").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 1.0f), "graph-core v3.0");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Descripcion
        ImGui::TextWrapped(
            "Motor avanzado de visualizacion y analisis de grafos y redes. "
            "Incluye algoritmos de caminos minimos, arboles de expansion, "
            "busqueda, ciclos, coloreo, isomorfismo, simulacion de redes "
            "y nuevo motor fisico QuadTree (Force Atlas 2).");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Tecnologias
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.65f, 1.0f), ICON_FA_MICROCHIP " Stack tecnologico:");
        ImGui::BulletText("C++17 con ImGui (Docking)");
        ImGui::BulletText("OpenGL 3.3 + GLFW");
        ImGui::BulletText("miniaudio (sistema de audio)");
        ImGui::BulletText("GoogleTest (testing)");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Algoritmos
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.65f, 1.0f), ICON_FA_GEARS " Algoritmos implementados:");
        ImGui::BulletText("Dijkstra, BFS, DFS");
        ImGui::BulletText("Kruskal, Prim (MST)");
        ImGui::BulletText("Deteccion de ciclos");
        ImGui::BulletText("Coloreo de grafos");
        ImGui::BulletText("Isomorfismo (VF2)");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Boton cerrar centrado
        float btn_w = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - btn_w) * 0.5f);
        if (ImGui::Button(ICON_FA_CHECK " Cerrar", ImVec2(btn_w, 32))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::Spacing();
        
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
                Animacion::reset(self.estado_grafos.anim_estado);
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
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
