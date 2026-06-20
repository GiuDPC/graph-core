#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"

class Interfaz;

// Panel de matrices de adyacencia e incidencia
namespace Matrices {

inline void dibujarAdyacencia(Grafo& red, bool aristas_dirigidas, ImFont* fontMono) {
    if (red.nodos.empty()) { ImGui::TextDisabled("Grafo vacio."); return; }

    ImVec2 region = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("MatAdjChild", ImVec2(region.x, region.y - 4), false,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    int cols = (int)red.nodos.size() + 1;
    float col_w_label = 52.0f;
    float col_w_data  = 32.0f;

    if (ImGui::BeginTable("MatAdj", cols, 
        ImGuiTableFlags_Borders | 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_ScrollX)) 
    {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, col_w_label);
        for (size_t c = 0; c < red.nodos.size(); c++) {
            ImGui::TableSetupColumn(red.nodos[c].nombre.c_str(), ImGuiTableColumnFlags_WidthFixed, col_w_data);
        }

        // Header
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.72f, 0.83f, 1.0f));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        // Rows
        for (size_t f = 0; f < red.nodos.size(); f++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), "%s", red.nodos[f].nombre.c_str());

            for (size_t c = 0; c < red.nodos.size(); c++) {
                ImGui::TableSetColumnIndex((int)c + 1);
                if (f == c) {
                    float peso_loop = -1;
                    for (const auto& a : red.aristas) {
                        if (a.origen_id == red.nodos[f].id && a.destino_id == red.nodos[f].id) {
                            peso_loop = a.peso_actual;
                            break;
                        }
                    }
                    if (peso_loop >= 0) {
                        char label[32];
                        snprintf(label, sizeof(label), "%g##%zu_%zu", peso_loop, f, c);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 0.5f, 1.0f));
                        if (ImGui::Selectable(label, true, 0, ImVec2(col_w_data, 0))) {
                            int nid = red.nodos[f].id;
                            auto it = std::remove_if(red.aristas.begin(), red.aristas.end(), [nid](const Arista& a) {
                                return a.origen_id == nid && a.destino_id == nid;
                            });
                            red.aristas.erase(it, red.aristas.end());
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Clic para eliminar bucle de %s", red.nodos[f].nombre.c_str());
                        ImGui::PopStyleColor();
                    } else {
                        char label[32];
                        snprintf(label, sizeof(label), "x##%zu_%zu", f, c);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
                        if (ImGui::Selectable(label, false, 0, ImVec2(col_w_data, 0))) {
                            red.agregarArista(red.nodos[f].id, red.nodos[f].id, 1.0f, true);
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Clic para crear bucle en %s", red.nodos[f].nombre.c_str());
                        ImGui::PopStyleColor();
                    }
                    continue;
                }

                float peso = -1;
                for (const auto& a : red.aristas) {
                    if ((a.origen_id == red.nodos[f].id && a.destino_id == red.nodos[c].id) ||
                        (!a.es_dirigida && a.origen_id == red.nodos[c].id && a.destino_id == red.nodos[f].id)) {
                        peso = a.peso_actual;
                        break;
                    }
                }

                bool has_edge = (peso >= 0);
                ImVec4 col;
                if (has_edge) {
                    if (peso <= 3.0f) col = ImVec4(0.0f, 0.9f, 0.5f, 1.0f);
                    else if (peso <= 10.0f) col = ImVec4(1.0f, 0.85f, 0.0f, 1.0f);
                    else col = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                } else {
                    col = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
                }

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                char label[32];
                if (has_edge) snprintf(label, sizeof(label), "%g##%zu_%zu", peso, f, c);
                else snprintf(label, sizeof(label), "-##%zu_%zu", f, c);
                
                if (ImGui::Selectable(label, has_edge, 0, ImVec2(col_w_data, 0))) {
                    if (has_edge) {
                        auto it = std::remove_if(red.aristas.begin(), red.aristas.end(), [&](const Arista& a) {
                            return (a.origen_id == red.nodos[f].id && a.destino_id == red.nodos[c].id) ||
                                   (!a.es_dirigida && a.origen_id == red.nodos[c].id && a.destino_id == red.nodos[f].id);
                        });
                        red.aristas.erase(it, red.aristas.end());
                    } else {
                        red.agregarArista(red.nodos[f].id, red.nodos[c].id, 1.0f, aristas_dirigidas);
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Clic para %s arista %s - %s", 
                        has_edge ? "eliminar" : "crear", 
                        red.nodos[f].nombre.c_str(), red.nodos[c].nombre.c_str());
                }
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

inline void dibujarIncidencia(Grafo& red, ImFont* fontMono) {
    if (red.nodos.empty() || red.aristas.empty()) {
        ImGui::TextDisabled("Sin datos.");
        return;
    }

    ImVec2 region = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("MatIncChild", ImVec2(region.x, region.y - 4), false,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    int cols = (int)red.aristas.size() + 1;
    float col_w_label = 52.0f;
    float col_w_data  = 32.0f;

    if (ImGui::BeginTable("MatInc", cols,
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg   |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_ScrollX))
    {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, col_w_label);
        for (int e = 0; e < (int)red.aristas.size(); e++) {
            char hdr[16];
            snprintf(hdr, sizeof(hdr), "e%d", e);
            ImGui::TableSetupColumn(hdr, ImGuiTableColumnFlags_WidthFixed, col_w_data);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        for (size_t f = 0; f < red.nodos.size(); f++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), "%s", red.nodos[f].nombre.c_str());

            for (size_t e = 0; e < red.aristas.size(); e++) {
                ImGui::TableSetColumnIndex((int)e + 1);
                const auto& a = red.aristas[e];
                if (a.origen_id == red.nodos[f].id || a.destino_id == red.nodos[f].id) {
                    ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), "1");
                } else {
                    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.35f, 1.0f), "0");
                }
            }
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();
}

inline void dibujar(Grafo& red, Interfaz& self) {
    (void)self;
    ImGui::Begin("Matrices");
    if (self.estado_ui.fontMono) ImGui::PushFont(self.estado_ui.fontMono);

    if (ImGui::BeginTabBar("MatricesTabs")) {
        if (ImGui::BeginTabItem(ICON_FA_TABLE_CELLS " Adyacencia")) {
            dibujarAdyacencia(red, self.estado_ui.aristas_dirigidas, self.estado_ui.fontMono);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(ICON_FA_TABLE_LIST " Incidencia")) {
            dibujarIncidencia(red, self.estado_ui.fontMono);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    if (self.estado_ui.fontMono) ImGui::PopFont();
    ImGui::End();
}

} 
