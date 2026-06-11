#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/SimuladorRed.h"
#include "nucleo/Topologias.h"
#include "audio/Sonidos.h"

class Interfaz;

extern Sonidos g_sonidos;

namespace PanelRed {

inline void dibujar(Interfaz& self, Grafo& red) {
    ImGui::Begin("Panel de Red");

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
        ICON_FA_NETWORK_WIRED " MODO RED — SIMULACION");
    ImGui::Separator();

    // ── Topologias predefinidas ──────────────────────────────────────────────
    if (ImGui::CollapsingHeader(ICON_FA_SITEMAP " Topologias Predefinidas")) {
        ImVec2 centro(700.0f, 400.0f);

        if (ImGui::Button("Red Empresarial", ImVec2(-1, 0))) {
            Topologias::empresarialBasica(red, centro);
            self.sim_inicializada = false;
            self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
            self.registrarLog("[OK] Topologia: Red Empresarial cargada");
        }
        if (ImGui::Button("Mesh Tolerante", ImVec2(-1, 0))) {
            Topologias::meshTolerante(red, centro);
            self.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Mesh cargada");
        }
        if (ImGui::Button("Red Estrella", ImVec2(-1, 0))) {
            Topologias::estrellaSimple(red, centro);
            self.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Estrella cargada");
        }
        if (ImGui::Button("Internet Simplificado", ImVec2(-1, 0))) {
            Topologias::internetSimple(red, centro);
            self.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Internet Simple cargada");
        }
    }
    ImGui::Separator();

    // ── Simulacion ───────────────────────────────────────────────────────────
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_WAVE_SQUARE " SIMULACION EN TIEMPO REAL");

    if (!self.sim_inicializada) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            ICON_FA_CIRCLE_EXCLAMATION " Simulacion no iniciada");
        if (ImGui::Button("Iniciar Simulacion", ImVec2(-1, 36))) {
            self.simulador.inicializar(red);
            self.sim_inicializada = true;
            self.registrarLog("[OK] Simulacion iniciada");
            int n_nodos = (int)red.nodos.size();
            if (n_nodos >= 3) {
                int a = red.nodos[0].id;
                int b = red.nodos[n_nodos - 1].id;
                int c = red.nodos[n_nodos / 2].id;
                self.simulador.enviarFlujo(a, b, 10.0f, "HTTP", 30.0f, red);
                self.simulador.enviarFlujo(b, c, 0.1f, "PING", 30.0f, red);
                if (n_nodos >= 5) {
                    int d = red.nodos[n_nodos / 3].id;
                    self.simulador.enviarFlujo(d, a, 50.0f, "VIDEO", 20.0f, red);
                }
                self.registrarLog("[OK] Trafico de prueba generado (HTTP, PING, VIDEO)");
            }
            self.mostrar_modal_inicio = true;
        }
    } else {
        ImGui::Text("Tiempo: %.1f s", self.simulador.estado.tiempo);
        ImGui::SameLine();
        ImGui::Text("| Flujos: %d", (int)self.simulador.estado.flujos.size());
        ImGui::SameLine();
        ImGui::Text("| Pkt: %d", (int)self.simulador.obtenerPaquetes().size());

        ImGui::SliderFloat("Velocidad##sim", &self.simulador.estado.velocidad, 0.1f, 5.0f, "x%.1f");
        ImGui::Checkbox("Eventos automaticos", &self.simulador.eventos_automaticos);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Spikes de trafico y microcortes aleatorios");

        float bw = (ImGui::GetContentRegionAvail().x - 8) / 2.0f;
        if (self.simulador.estado.activa) {
            if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 0)))
                self.simulador.estado.activa = false;
        } else {
            if (ImGui::Button(ICON_FA_PLAY " Reanudar", ImVec2(bw, 0)))
                self.simulador.estado.activa = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reiniciar", ImVec2(bw, 0))) {
            self.simulador.inicializar(red);
            self.registrarLog("[OK] Simulacion reiniciada");
        }
    }
    ImGui::Separator();

    // ── Enviar trafico ───────────────────────────────────────────────────────
    if (self.sim_inicializada) {
        if (ImGui::CollapsingHeader(ICON_FA_PAPER_PLANE " Enviar Trafico")) {
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Crea un flujo de datos entre dos nodos.\nHTTP=10, Video=50, Ping=0.1, DDoS=500 Mbps.\nLos puntitos de colores viajan por las aristas.");
            const char* tipos_flujo[] = {"HTTP (10Mbps)", "Video (50Mbps)", "Ping (0.1Mbps)", "DDoS (500Mbps)"};
            float mbps_flujo[] = {10.0f, 50.0f, 0.1f, 500.0f};
            const char* nombre_flujo[] = {"HTTP", "VIDEO", "PING", "DDOS"};

            ImGui::Text("Origen:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##fl_orig", red.nombreNodo(self.flujo_origen).c_str())) {
                for (const auto& n : red.nodos) {
                    bool sel = (n.id == self.flujo_origen);
                    if (ImGui::Selectable(n.nombre.c_str(), sel)) self.flujo_origen = n.id;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Text("Destino:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##fl_dst", red.nombreNodo(self.flujo_destino).c_str())) {
                for (const auto& n : red.nodos) {
                    bool sel = (n.id == self.flujo_destino);
                    if (ImGui::Selectable(n.nombre.c_str(), sel)) self.flujo_destino = n.id;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Combo("Tipo##flujo", &self.flujo_tipo, tipos_flujo, 4);
            ImGui::SliderFloat("Duracion (s)", &self.flujo_dur, 1.0f, 60.0f, "%.0f s");

            if (ImGui::Button(ICON_FA_PAPER_PLANE " Enviar", ImVec2(-1, 32))) {
                if (self.flujo_origen != self.flujo_destino) {
                    self.simulador.enviarFlujo(self.flujo_origen, self.flujo_destino,
                        mbps_flujo[self.flujo_tipo], nombre_flujo[self.flujo_tipo],
                        self.flujo_dur, red);
                    g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
                }
            }
        }

        // ── Simular fallos ───────────────────────────────────────────────────
        if (ImGui::CollapsingHeader(ICON_FA_TRIANGLE_EXCLAMATION " Simular Fallos")) {
            ImGui::TextDisabled("Selecciona un nodo en el lienzo y haz click aqui:");

            if (self.nodo_seleccionado >= 0) {
                auto& en = self.simulador.estado.nodos[self.nodo_seleccionado];
                if (en.activo) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::Button(("Derribar " + red.nombreNodo(self.nodo_seleccionado)).c_str(),
                        ImVec2(-1, 0))) {
                        self.simulador.simularFalloNodo(self.nodo_seleccionado, red);
                        g_sonidos.reproducir(Sonidos::NODO_CAIDO);
                    }
                    ImGui::PopStyleColor();
                } else {
                    float tc = en.tiempo_caida;
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                        ICON_FA_TRIANGLE_EXCLAMATION " %s CAIDO (%.0fs)",
                        red.nombreNodo(self.nodo_seleccionado).c_str(), tc);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                    if (ImGui::Button(("Restaurar " + red.nombreNodo(self.nodo_seleccionado)).c_str(),
                        ImVec2(-1, 0))) {
                        self.simulador.restaurarNodo(self.nodo_seleccionado, red);
                        g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                    }
                    ImGui::PopStyleColor();
                }
            } else {
                ImGui::TextDisabled("Ningun nodo seleccionado");
            }
        }

        // ── Estado por nodo ──────────────────────────────────────────────────
        if (ImGui::CollapsingHeader(ICON_FA_CHART_BAR " Estado de Nodos")) {
            if (ImGui::BeginTable("tabNodos", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit |
                ImGuiTableFlags_ScrollY, ImVec2(0, 150)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Nodo",  ImGuiTableColumnFlags_WidthFixed, 55);
                ImGui::TableSetupColumn("CPU",   ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("RAM",   ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("RX",    ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn("TX",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& n : red.nodos) {
                    if (!self.simulador.estado.nodos.count(n.id)) continue;
                    const auto& en = self.simulador.estado.nodos.at(n.id);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (!en.activo)
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", n.nombre.c_str());
                    else
                        ImGui::Text("%s", n.nombre.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.0f%%", en.cpu_uso * 100.0f);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.0f%%", en.memoria_uso * 100.0f);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.0f", en.paquetes_rx);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.0f", en.paquetes_tx);
                }
                ImGui::EndTable();
            }
        }

        // ── Log de eventos ───────────────────────────────────────────────────
        if (ImGui::CollapsingHeader(ICON_FA_LIST " Log de Eventos Red")) {
            ImGui::BeginChild("LogRed", ImVec2(0, 150), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (auto it = self.simulador.estado.log_eventos.rbegin();
                 it != self.simulador.estado.log_eventos.rend(); ++it)
            {
                ImVec4 col;
                switch (it->severidad) {
                    case EventoRed::ERROR_RED:   col = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
                    case EventoRed::ADVERTENCIA: col = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); break;
                    default:                     col = ImVec4(0.5f, 0.8f, 0.5f, 1.0f); break;
                }
                ImGui::TextColored(col, "[%.1fs] %s", it->timestamp, it->mensaje.c_str());
            }
            ImGui::EndChild();
        }
    }

    // Popup de confirmacion (se muestra UNA vez al iniciar simulacion)
    if (self.mostrar_modal_inicio) {
        ImGui::OpenPopup("Simulacion iniciada");
        self.mostrar_modal_inicio = false;
    }
    if (ImGui::BeginPopupModal("Simulacion iniciada", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_PLAY " Simulacion en marcha!");
        ImGui::Text("Se generaron flujos de prueba para que veas actividad.");
        ImGui::Text("Usa 'Enviar Trafico' para crear mas, o activa");
        ImGui::Text("'Eventos automaticos' para spikes y microcortes.");
        ImGui::Separator();
        if (ImGui::Button("Entendido")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace PanelRed
