cat << 'EOF' > /tmp/PanelIsomorfismo.h
#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/algoritmos/Isomorfismo.h"
#include "audio/Sonidos.h"

class Interfaz;
extern Sonidos g_sonidos;

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
            self.estado_grafos.iso_editando_g2 = false;
            ImGui::Text("Nodos: %d  |  Aristas: %d",
                (int)red.nodos.size(), (int)red.aristas.size());
            auto degs = Algoritmos::Isomorfismo::secuenciaGrados(red);
            std::string dstr;
            for (int d : degs) dstr += std::to_string(d) + " ";
            ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
            
            ImGui::Spacing();
            if (ImGui::Button(ICON_FA_SHAPES " Dar Forma Geometrica (Poligono)", ImVec2(-1, 32))) {
                if (!red.nodos.empty()) {
                    self.historial.capturar(red);
                    float cx = 400.0f;
                    float cy = 300.0f;
                    float radio = 150.0f;
                    for (size_t i = 0; i < red.nodos.size(); ++i) {
                        float ang = (2.0f * 3.14159f / red.nodos.size()) * i;
                        red.nodos[i].posicion = ImVec2(cx + cosf(ang) * radio, cy + sinf(ang) * radio);
                    }
                    g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                    self.registrarLog("[OK] G1 ordenado en forma geometrica.");
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Acomoda todos los nodos del grafo en un circulo perfecto.");
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("G2 (El Grafo a Comparar)")) {
            self.estado_grafos.iso_editando_g2 = true;
            ImGui::Text("Nodos: %d  |  Aristas: %d",
                (int)self.estado_grafos.grafo_iso_g2.nodos.size(), (int)self.estado_grafos.grafo_iso_g2.aristas.size());
            auto degs = Algoritmos::Isomorfismo::secuenciaGrados(self.estado_grafos.grafo_iso_g2);
            std::string dstr;
            for (int d : degs) dstr += std::to_string(d) + " ";
            ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Opciones para G2:");

            if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Generar Isomorfo Aleatorio", ImVec2(-1, 32))) {
                self.estado_grafos.grafo_iso_g2.limpiar();
                if (!red.nodos.empty()) {
                    std::vector<int> idx(red.nodos.size());
                    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
                    for (size_t i = 0; i < idx.size(); ++i) {
                        size_t j = i + std::uniform_int_distribution<size_t>(0, idx.size() - i - 1)(red.obtenerGeneradorAleatorio());
                        std::swap(idx[i], idx[j]);
                    }
                    std::map<int, int> mapa;
                    float cx = 800.0f; // desplazar a la derecha
                    float cy = 300.0f;
                    float radio = 150.0f;
                    for (size_t i = 0; i < idx.size(); ++i) {
                        const auto& n_orig = red.nodos[idx[i]];
                        float ang = (2.0f * 3.14159f / idx.size()) * i;
                        Nodo n(i, ImVec2(cx + cosf(ang) * radio, cy + sinf(ang) * radio), n_orig.tipo);
                        n.nombre = "Iso_" + n_orig.nombre;
                        self.estado_grafos.grafo_iso_g2.nodos.push_back(n);
                        mapa[n_orig.id] = i;
                    }
                    for (const auto& a : red.aristas) {
                        self.estado_grafos.grafo_iso_g2.aristas.push_back(Arista(mapa[a.origen_id], mapa[a.destino_id], a.peso, a.es_dirigida));
                    }
                    self.estado_grafos.grafo_iso_g2.contador_ids = (int)red.nodos.size();
                    self.estado_grafos.iso_analizado = false;
                    self.registrarLog("[OK] G2 generado. Estructura identica pero posiciones aleatorias.");
                    g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Crea una copia isomorfa con los nodos mezclados caoticamente.");

            if (ImGui::Button(ICON_FA_SHAPES " Generar Isomorfo en Forma Geometrica", ImVec2(-1, 32))) {
                self.estado_grafos.grafo_iso_g2.limpiar();
                if (!red.nodos.empty()) {
                    std::map<int, int> mapa;
                    float cx = 800.0f; // desplazar a la derecha
                    float cy = 300.0f;
                    float radio = 150.0f;
                    
                    for (size_t i = 0; i < red.nodos.size(); ++i) {
                        const auto& n_orig = red.nodos[i];
                        float ang = (2.0f * 3.14159f / red.nodos.size()) * i;
                        Nodo n(i, ImVec2(cx + cosf(ang) * radio, cy + sinf(ang) * radio), n_orig.tipo);
                        n.nombre = "IsoGeo_" + n_orig.nombre;
                        self.estado_grafos.grafo_iso_g2.nodos.push_back(n);
                        mapa[n_orig.id] = i;
                    }
                    for (const auto& a : red.aristas) {
                        self.estado_grafos.grafo_iso_g2.aristas.push_back(Arista(mapa[a.origen_id], mapa[a.destino_id], a.peso, a.es_dirigida));
                    }
                    self.estado_grafos.grafo_iso_g2.contador_ids = (int)red.nodos.size();
                    self.estado_grafos.iso_analizado = false;
                    self.registrarLog("[OK] G2 generado. Forma geometrica perfecta.");
                    g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Crea una copia isomorfa y la dibuja como un poligono regular.");

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), ICON_FA_PENCIL " Click derecho en el lienzo dibuja en G2");
            ImGui::Spacing();
            
            if (ImGui::Button("Limpiar G2", ImVec2(-1, 32))) {
                self.estado_grafos.grafo_iso_g2.limpiar();
                self.estado_grafos.iso_analizado = false;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Elimina el segundo grafo y reinicia la verificacion de isomorfismo.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), ICON_FA_CHECK_DOUBLE " Ejecutar Verificacion");
    ImGui::TextWrapped("Compara matemáticamente la estructura y conexiones. Limitado a <= 12 nodos.");
    
    bool habilitado_topo = (red.nodos.size() <= 12 && self.estado_grafos.grafo_iso_g2.nodos.size() <= 12);
    if (!habilitado_topo) ImGui::BeginDisabled();
    
    if (ImGui::Button(ICON_FA_SHUFFLE " Verificar Isomorfismo", ImVec2(-1, 35))) {
        if (red.nodos.empty() || self.estado_grafos.grafo_iso_g2.nodos.empty()) {
            self.registrarLog("[!] Isomorfismo: ambos grafos deben tener nodos");
            g_sonidos.reproducir(Sonidos::DESCARTAR);
        } else {
            self.estado_grafos.resultado_iso = Algoritmos::Isomorfismo::verificar(red, self.estado_grafos.grafo_iso_g2);
            self.estado_grafos.iso_analizado = true;
            bool son = self.estado_grafos.resultado_iso.son_isomorfos;
            self.registrarLog(son ? "[OK] Grafos Isomorfos: Si." : "[!] Grafos Isomorfos: No.");
            g_sonidos.reproducir(son ? Sonidos::TRIUNFO_DIJKSTRA : Sonidos::DESCARTAR);
        }
    }
    
    if (!habilitado_topo) ImGui::EndDisabled();
    if (!habilitado_topo) {
        ImGui::TextColored(ImVec4(1,0.3f,0,1), "Maximo 12 nodos (complejidad O(N!))");
    }

    if (self.estado_grafos.iso_analizado) {
        ImGui::Separator();

        auto icono_cond = [](bool ok) {
            return ok ? ICON_FA_CHECK : ICON_FA_XMARK;
        };
        auto col_cond = [](bool ok) -> ImVec4 {
            return ok ? ImVec4(0.0f, 1.0f, 0.5f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        };

        ImGui::Text("Condiciones necesarias:");
        ImGui::TextColored(col_cond(self.estado_grafos.resultado_iso.misma_cantidad_nodos),
            "%s Misma cantidad de nodos", icono_cond(self.estado_grafos.resultado_iso.misma_cantidad_nodos));
        ImGui::TextColored(col_cond(self.estado_grafos.resultado_iso.misma_cantidad_aristas),
            "%s Misma cantidad de aristas", icono_cond(self.estado_grafos.resultado_iso.misma_cantidad_aristas));
        ImGui::TextColored(col_cond(self.estado_grafos.resultado_iso.misma_secuencia_grados),
            "%s Misma secuencia de grados", icono_cond(self.estado_grafos.resultado_iso.misma_secuencia_grados));

        ImGui::Spacing();
        ImGui::Spacing();
        if (self.estado_grafos.resultado_iso.son_isomorfos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " SON ISOMORFOS");
            
            // --- Nueva seccion visual de Matrices ---
            if (ImGui::TreeNode("Ver Matrices de Adyacencia")) {
                ImGui::Text("Grafo 1 (G1):");
                for (size_t i = 0; i < red.nodos.size(); i++) {
                    std::string fila = "[ ";
                    for (size_t j = 0; j < red.nodos.size(); j++) {
                        bool hay = red.obtenerArista(red.nodos[i].id, red.nodos[j].id) != nullptr;
                        fila += (hay ? "1 " : "0 ");
                    }
                    fila += "]";
                    ImGui::TextDisabled("%s", fila.c_str());
                }
                ImGui::Spacing();
                ImGui::Text("Grafo 2 (G2) reordenado por mapeo:");
                for (size_t i = 0; i < red.nodos.size(); i++) {
                    std::string fila = "[ ";
                    for (size_t j = 0; j < red.nodos.size(); j++) {
                        int id_g1_i = red.nodos[i].id;
                        int id_g1_j = red.nodos[j].id;
                        
                        int id_g2_i = -1;
                        int id_g2_j = -1;
                        for (const auto& par : self.estado_grafos.resultado_iso.mapeo) {
                            if (par.first == id_g1_i) id_g2_i = par.second;
                            if (par.first == id_g1_j) id_g2_j = par.second;
                        }

                        bool hay = false;
                        if (id_g2_i != -1 && id_g2_j != -1) {
                            hay = self.estado_grafos.grafo_iso_g2.obtenerArista(id_g2_i, id_g2_j) != nullptr;
                        }
                        fila += (hay ? "1 " : "0 ");
                    }
                    fila += "]";
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", fila.c_str());
                }
                ImGui::TreePop();
            }

            ImGui::Spacing();
            ImGui::Text("Mapeo de nodos:");
            for (const auto& par : self.estado_grafos.resultado_iso.mapeo) {
                ImGui::BulletText("%s (G1) <-> %s (G2)",
                    red.nombreNodo(par.first).c_str(),
                    self.estado_grafos.grafo_iso_g2.nombreNodo(par.second).c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                ICON_FA_XMARK " NO SON ISOMORFOS");
            ImGui::TextWrapped("%s", self.estado_grafos.resultado_iso.descripcion.c_str());
        }
    }
}

}
EOF
mv /tmp/PanelIsomorfismo.h src/interfaz/paneles/PanelIsomorfismo.h
