#pragma once

#include "interfaz/Interfaz.h"
#include "audio/Sonidos.h"

// Barra de estado inferior — modo actual, FPS, simulación, audio
namespace StatusBar {

inline void dibujar(Interfaz& self) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - 30));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, 30));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.09f, 1.0f));
    ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

    const char* modo_txt = (self.modo_actual == Interfaz::ModoApp::Grafos)
        ? ICON_FA_DIAGRAM_PROJECT " GRAFOS"
        : ICON_FA_NETWORK_WIRED " REDES";
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f), "%s", modo_txt);

    ImGui::SameLine(180);
    if (self.anim_estado.activa) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), ICON_FA_SPINNER " ANIMANDO");
    } else if (self.modo_actual == Interfaz::ModoApp::Redes && self.sim_inicializada) {
        if (self.simulador.estado.activa) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_PLAY " SIMULANDO x%.1f", self.simulador.estado.velocidad);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), ICON_FA_PAUSE " PAUSADA");
        }
        ImGui::SameLine(400);
        ImGui::Text("T: %.0fs | Flujos: %d | Paqs: %d",
            self.simulador.estado.tiempo,
            (int)self.simulador.estado.flujos.size(),
            (int)self.simulador.obtenerPaquetes().size());
    } else if (self.simulacion_jitter && self.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ICON_FA_WAVE_SQUARE " JITTER ACTIVO");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.5f, 1.0f), ICON_FA_CIRCLE_CHECK " ENGINE OK");
    }

    ImGui::SameLine(650);
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);

    ImGui::SameLine(750);
    if (g_sonidos.funciona()) {
        float v = g_sonidos.getVolumen();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
            ICON_FA_VOLUME_HIGH " %d%%", (int)(v*100));
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
            ICON_FA_VOLUME_XMARK " SIN AUDIO");
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

} // namespace StatusBar
