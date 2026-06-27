#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "interfaz/estado/EstadoUI.h"

// -- toolbar compacta con botones horizontales (icono + texto en una linea) --
// cada categoria es un boton en la barra superior.
// al seleccionar una, el panel contextual derecho muestra su config.

struct Toolbar {
    // retorna true si se cambio de categoria
    static bool dibujar(EstadoUI& ui, bool en_modo_aero) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        bool cambio = false;
        float avail = ImGui::GetContentRegionAvail().x;

        // -- selector de modo a la izquierda --
        {
            bool en_g = (ui.modo_actual == EstadoUI::ModoApp::Grafos);
            if (en_g) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
            if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " Graph Lab", ImVec2(0, 28))) {
                ui.modo_actual = EstadoUI::ModoApp::Grafos;
                ui.herramienta_activa = EstadoUI::CatGeneral;
                cambio = true;
            }
            if (en_g) ImGui::PopStyleColor();

            ImGui::SameLine(0, 2);

            bool en_a = (ui.modo_actual == EstadoUI::ModoApp::AeroGrafos);
            if (en_a) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
            if (ImGui::Button(ICON_FA_PLANE " AeroGrafos", ImVec2(0, 28))) {
                ui.modo_actual = EstadoUI::ModoApp::AeroGrafos;
                ui.herramienta_activa = EstadoUI::CatGeneral;
                cambio = true;
            }
            if (en_a) ImGui::PopStyleColor();
        }

        // -- separador vertical visual --
        ImGui::SameLine(0, 8);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|");
        ImGui::SameLine(0, 8);

        // -- categorias segun modo --
        if (ui.modo_actual == EstadoUI::ModoApp::Grafos) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ICON_FA_CIRCLE_INFO " panel derecho para algoritmos");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ICON_FA_CIRCLE_INFO " panel derecho para AeroGrafos");
        }

        // -- boton de enciclopedia (wiki) --
        ImGui::SameLine(avail - 300);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.25f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.4f, 1.0f));
        if (ImGui::Button(ICON_FA_BOOK_OPEN " Enciclopedia", ImVec2(130, 28))) {
            ui.mostrar_ventana_ayuda = true;
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abre la documentacion completa del programa");

        // -- boton de fisicas (derecha) --
        if (ui.modo_actual == EstadoUI::ModoApp::Grafos) {
            ImGui::SameLine(avail - 150);
            bool fisicas_pushed = false;
            if (ui.fisicas_activas) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.8f, 0.8f));
                fisicas_pushed = true;
            }
            if (ImGui::Button(ui.fisicas_activas ? ICON_FA_MAGNET " FA2 (ON)" : ICON_FA_MAGNET " FA2 (OFF)", ImVec2(145, 28))) {
                ui.fisicas_activas = !ui.fisicas_activas;
                ui.fisicas_estado_cambiado = true;
                if (ui.fisicas_activas) ui.fa2.reset();
            }
            if (fisicas_pushed) ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Activa ForceAtlas2: layout de fuerzas que agrupa clusters.");
        }

        ImGui::PopStyleVar(3);
        return cambio;
    }

private:
    // Boton horizontal compacto: icono + texto en una sola linea
    static bool botonCat(const char* id, const char* label_con_icono,
                         EstadoUI::Categoria cat, EstadoUI& ui) {
        bool activa = (ui.herramienta_activa == cat);
        bool click = false;

        ImVec4 col_bg = activa
            ? ImVec4(0.0f, 0.55f, 0.45f, 0.85f)
            : ImVec4(0.15f, 0.15f, 0.20f, 0.8f);
        ImVec4 col_hover = activa
            ? ImVec4(0.0f, 0.65f, 0.55f, 0.9f)
            : ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
        ImVec4 col_text = activa
            ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
            : ImVec4(0.70f, 0.72f, 0.78f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, col_bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.75f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, col_text);
        ImGui::PushID(id);

        if (ImGui::Button(label_con_icono, ImVec2(0, 28))) {
            if (ui.herramienta_activa == cat) {
                ui.herramienta_activa = EstadoUI::CatGeneral;
            } else {
                ui.herramienta_activa = cat;
            }
            click = true;
        }

        // Indicador de activo: linea inferior cyan
        if (activa) {
            ImVec2 rmin = ImGui::GetItemRectMin();
            ImVec2 rmax = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(rmin.x + 4, rmax.y - 1),
                ImVec2(rmax.x - 4, rmax.y - 1),
                IM_COL32(0, 220, 170, 255), 2.0f);
        }

        ImGui::PopID();
        ImGui::PopStyleColor(4);
        return click;
    }
};
