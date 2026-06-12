#pragma once

#include "interfaz/Interfaz.h"
#include "audio/Sonidos.h"

// Barra de estado inferior — modo actual, FPS, simulacion, audio
namespace StatusBar {

inline void dibujar(Interfaz& self) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float bar_h = 26.0f;
    float bar_w = vp->WorkSize.x;
    
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - bar_h));
    ImGui::SetNextWindowSize(ImVec2(bar_w, bar_h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
    ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

    // Modo actual
    const char* modo_txt = (self.estado_ui.modo_actual == Interfaz::ModoApp::Grafos)
        ? ICON_FA_DIAGRAM_PROJECT " GRAFOS"
        : ICON_FA_NETWORK_WIRED " REDES";
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 0.65f, 1.0f), "%s", modo_txt);

    // Estado de simulacion / animacion
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|");
    ImGui::SameLine();

    if (self.estado_grafos.anim_estado.activa) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), ICON_FA_SPINNER " Animando");
    } else if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes && self.estado_redes.sim_inicializada) {
        if (self.estado_redes.simulador.estado.activa) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_PLAY " x%.1f", self.estado_redes.simulador.estado.velocidad);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), ICON_FA_PAUSE " Pausada");
        }
    } else if (self.estado_redes.simulacion_jitter && self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ICON_FA_WAVE_SQUARE " Jitter");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.5f, 1.0f), ICON_FA_CIRCLE_CHECK " OK");
    }

    // FPS a la derecha, usando posicion relativa
    float fps_w = ImGui::CalcTextSize("FPS: 60").x + 20;
    float audio_w = 180.0f;
    float right_x = bar_w - fps_w - audio_w - 20;
    
    if (right_x > ImGui::GetCursorPosX() + 20) {
        ImGui::SameLine(right_x);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "FPS: %.0f", ImGui::GetIO().Framerate);

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|");
        ImGui::SameLine();
        
        if (g_sonidos.funciona()) {
            int v = g_sonidos.getVolumenInt();
            const char* be = g_sonidos.backendString();
            ImGui::TextColored(
                ImVec4(
                    (v > 0) ? 0.0f : 0.5f,
                    (v > 0) ? 0.75f : 0.5f,
                    (v > 0) ? 0.47f : 0.5f, 1.0f),
                "%s %d%% [%s]", ICON_FA_VOLUME_HIGH, v, be);
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.3f, 0.3f, 1.0f),
                ICON_FA_VOLUME_XMARK " Sin audio");
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace StatusBar
