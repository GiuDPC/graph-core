#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <string>
#include <vector>

class Interfaz;

namespace LogPanel {

inline void dibujar(Interfaz& self) {
    ImGui::Begin("Registro del Kernel");
    if (self.fontMono) ImGui::PushFont(self.fontMono);

    if (ImGui::Button(ICON_FA_TRASH_CAN " Limpiar")) self.system_logs.clear();
    ImGui::SameLine();
    ImGui::Text("%d entradas", (int)self.system_logs.size());
    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : self.system_logs) {
        ImVec4 col(0.5f, 0.7f, 0.5f, 1.0f);
        if (log.find("[!]") != std::string::npos) col = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        else if (log.find("[OK]") != std::string::npos) col = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
        ImGui::TextColored(col, "%s", log.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    if (self.fontMono) ImGui::PopFont();
    ImGui::End();
}

} // namespace LogPanel
