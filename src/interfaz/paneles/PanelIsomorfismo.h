#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/algoritmos/Isomorfismo.h"

class Interfaz;

namespace PanelIsomorfismo {

inline void dibujar(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.8f, 1.0f),
        ICON_FA_OBJECT_GROUP " ISOMORFISMO");

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(190, 200, 210, 255));
    ImGui::TextWrapped(
        "¿Qué es esto?\n"
        "Dos grafos son isomorfos si tienen exactamente la misma forma y "
        "conexiones, sin importar dónde estén dibujados o cómo se llamen sus nodos.\n"
        "Es útil para saber si dos redes aparentemente distintas son en realidad la misma.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("IsoTabs")) {
        if (ImGui::BeginTabItem("G1 (Tu Grafo Principal)")) {
            ImGui::Text("Nodos: %d  |  Aristas: %d",
                (int)red.nodos.size(), (int)red.aristas.size());
            auto degs = Algoritmos::Isomorfismo::secuenciaGrados(red);
            std::string dstr;
            for (int d : degs) dstr += std::to_string(d) + " ";
            ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("G2 (El Grafo a Comparar)")) {
            ImGui::Text("Nodos: %d  |  Aristas: %d",
                (int)self.grafo_iso_g2.nodos.size(), (int)self.grafo_iso_g2.aristas.size());
            auto degs = Algoritmos::Isomorfismo::secuenciaGrados(self.grafo_iso_g2);
            std::string dstr;
            for (int d : degs) dstr += std::to_string(d) + " ";
            ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Opciones para G2:");

            if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Generar Isomorfo Automatico", ImVec2(-1, 0))) {
                self.grafo_iso_g2.limpiar();
                if (!red.nodos.empty()) {
                    std::vector<int> idx(red.nodos.size());
                    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
                    for (size_t i = 0; i < idx.size(); ++i) {
                        size_t j = i + rand() % (idx.size() - i);
                        std::swap(idx[i], idx[j]);
                    }
                    std::map<int, int> mapa;
                    float min_x = 9999, max_x = -9999;
                    float min_y = 9999, max_y = -9999;
                    for (auto& n : red.nodos) {
                        if (n.posicion.x < min_x) min_x = n.posicion.x;
                        if (n.posicion.x > max_x) max_x = n.posicion.x;
                        if (n.posicion.y < min_y) min_y = n.posicion.y;
                        if (n.posicion.y > max_y) max_y = n.posicion.y;
                    }
                    if (min_x > max_x) { min_x = 0; max_x = 800; min_y = 0; max_y = 600; }
                    float cx = max_x + 350.0f;
                    float cy = (min_y + max_y) / 2.0f;
                    float radio = std::max(150.0f, (max_y - min_y) / 2.0f);
                    for (size_t i = 0; i < idx.size(); ++i) {
                        const auto& n_orig = red.nodos[idx[i]];
                        float ang = (2.0f * 3.14159f / idx.size()) * i;
                        Nodo n(i, ImVec2(cx + cosf(ang) * radio, cy + sinf(ang) * radio), n_orig.tipo);
                        n.nombre = "Iso_" + n_orig.nombre;
                        self.grafo_iso_g2.nodos.push_back(n);
                        mapa[n_orig.id] = i;
                    }
                    for (const auto& a : red.aristas) {
                        self.grafo_iso_g2.aristas.push_back(Arista(mapa[a.origen_id], mapa[a.destino_id], a.peso, a.es_dirigida));
                    }
                    self.grafo_iso_g2.contador_ids = (int)red.nodos.size();
                    self.iso_editando_g2 = true;
                    self.iso_analizado = false;
                    self.registrarLog("[OK] G2 generado automaticamente. ¡Estan desordenados pero son identicos!");
                }
            }

            ImGui::Checkbox("Dibujar G2 manualmente", &self.iso_editando_g2);
            if (self.iso_editando_g2) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                    ICON_FA_PENCIL " Click derecho en el lienzo para crear nodos/aristas en G2");
            }
            if (ImGui::Button("Limpiar G2", ImVec2(-1, 0))) {
                self.grafo_iso_g2.limpiar();
                self.iso_analizado = false;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Verificar Isomorfismo", ImVec2(-1, 36))) {
        if (red.nodos.empty() || self.grafo_iso_g2.nodos.empty()) {
            self.registrarLog("[!] Isomorfismo: ambos grafos deben tener nodos");
        } else {
            self.resultado_iso = Algoritmos::Isomorfismo::verificar(red, self.grafo_iso_g2);
            self.iso_analizado = true;
            self.registrarLog(self.resultado_iso.son_isomorfos
                ? "[OK] Isomorfismo: ¡Genial! Los grafos SON isomorfos."
                : "[!] Isomorfismo: Los grafos NO son isomorfos.");
        }
    }

    if (self.iso_analizado) {
        ImGui::Separator();

        auto icono_cond = [](bool ok) {
            return ok ? ICON_FA_CHECK : ICON_FA_XMARK;
        };
        auto col_cond = [](bool ok) -> ImVec4 {
            return ok ? ImVec4(0.0f, 1.0f, 0.5f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        };

        ImGui::Text("Condiciones necesarias:");
        ImGui::TextColored(col_cond(self.resultado_iso.misma_cantidad_nodos),
            "%s Misma cantidad de nodos", icono_cond(self.resultado_iso.misma_cantidad_nodos));
        ImGui::TextColored(col_cond(self.resultado_iso.misma_cantidad_aristas),
            "%s Misma cantidad de aristas", icono_cond(self.resultado_iso.misma_cantidad_aristas));
        ImGui::TextColored(col_cond(self.resultado_iso.misma_secuencia_grados),
            "%s Misma secuencia de grados", icono_cond(self.resultado_iso.misma_secuencia_grados));

        ImGui::Spacing();
        if (self.resultado_iso.son_isomorfos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " SON ISOMORFOS");
            ImGui::Text("Mapeo de nodos:");
            for (const auto& par : self.resultado_iso.mapeo) {
                ImGui::BulletText("%s (G1) <-> %s (G2)",
                    red.nombreNodo(par.first).c_str(),
                    self.grafo_iso_g2.nombreNodo(par.second).c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                ICON_FA_XMARK " NO SON ISOMORFOS");
            ImGui::TextWrapped("%s", self.resultado_iso.descripcion.c_str());
        }
    }
}

} // namespace PanelIsomorfismo
