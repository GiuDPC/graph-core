#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/SimuladorRed.h"
#include "nucleo/Topologias.h"
#include "audio/Sonidos.h"
#include "interfaz/lienzo/LienzoRed.h"

class Interfaz;
extern Sonidos g_sonidos;

namespace PanelRed {

inline void sparkline(ImDrawList* dl, const float* datos, int total, ImVec2 pos, float w, float h, ImU32 col, float max_v = -1.0f) {
    if (total < 2) return;
    float m = max_v;
    if (m < 0) { for (int i = 0; i < total; i++) if (datos[i] > m) m = datos[i]; }
    if (m < 0.001f) m = 1.0f;
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(10, 10, 15, 120), 2.0f);
    for (int i = 1; i < total; i++) {
        float x0 = pos.x + (float)(i - 1) / (float)(total - 1) * w;
        float x1 = pos.x + (float)i / (float)(total - 1) * w;
        float y0 = pos.y + h - (datos[i - 1] / m) * h;
        float y1 = pos.y + h - (datos[i] / m) * h;
        dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), col, 1.5f);
    }
    if (total > 0) {
        float last_v = datos[(total - 1 + HistorialUso::MAX_MUESTRAS) % HistorialUso::MAX_MUESTRAS];
        float lx = pos.x + w;
        float ly = pos.y + h - (last_v / m) * h;
        dl->AddCircleFilled(ImVec2(lx, ly), 2.5f, col, 8);
    }
}

inline void dibujar(Interfaz& self, Grafo& red) {
    ImGui::Begin("Panel de Red");

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
        ICON_FA_NETWORK_WIRED " SIMULACION DE RED");
    ImGui::Separator();

    // Topologias
    if (ImGui::CollapsingHeader(ICON_FA_SITEMAP " Topologias Predefinidas")) {
        ImVec2 centro(700.0f, 400.0f);
        if (ImGui::Button("Red Empresarial", ImVec2(-1, 28))) {
            Topologias::empresarialBasica(red, centro);
            self.estado_redes.sim_inicializada = false;
            self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            self.registrarLog("[OK] Topologia: Red Empresarial cargada");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Servidores, routers y switches organizados por departamentos.");
        if (ImGui::Button("Mesh Tolerante", ImVec2(-1, 28))) {
            Topologias::meshTolerante(red, centro);
            self.estado_redes.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Mesh cargada");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Malla con caminos redundantes entre todos los nodos.");
        if (ImGui::Button("Red Estrella", ImVec2(-1, 28))) {
            Topologias::estrellaSimple(red, centro);
            self.estado_redes.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Estrella cargada");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Un nodo central conectado a todos los demas.");
        if (ImGui::Button("Internet Simplificado", ImVec2(-1, 28))) {
            Topologias::internetSimple(red, centro);
            self.estado_redes.sim_inicializada = false;
            self.registrarLog("[OK] Topologia: Internet Simple cargada");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Estructura basica de internet con AS y backbones.");
    }
    ImGui::Separator();

    // Jitter y condiciones de red
    if (ImGui::CollapsingHeader(ICON_FA_WAVE_SQUARE " Condiciones de Red", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Simular Jitter", &self.estado_redes.simulacion_jitter);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aplica variacion aleatoria a los pesos de las aristas simulando inestabilidad real.");
        if (self.estado_redes.simulacion_jitter) {
            ImGui::SliderFloat("Intensidad##jitter", &self.estado_redes.jitter_porcentaje, 0.01f, 0.5f, "%.0f%%");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Porcentaje de variacion. 15%% = un enlace de 100ms oscila entre 85 y 115ms.");
        }
        ImGui::Spacing();
        ImGui::Checkbox("Mostrar Estadisticas", &self.estado_redes.mostrar_stats);
        ImGui::SameLine();
        ImGui::Checkbox("Mostrar Timeline", &self.estado_redes.mostrar_timeline);
    }
    ImGui::Separator();

    // Simulacion 
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_PLAY " SIMULACION EN TIEMPO REAL");

    if (!self.estado_redes.sim_inicializada) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            ICON_FA_CIRCLE_EXCLAMATION " No iniciada");
        ImGui::TextDisabled("Preset de trafico:");
        const char* presets[] = {"Supervivencia", "Videollamada", "Empresarial", "DDoS"};
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##preset", &self.estado_redes.preset_trafico, presets, 4);

        if (ImGui::Button(ICON_FA_PLAY " Iniciar Simulacion", ImVec2(-1, 28))) {
            self.estado_redes.simulador.inicializar(red);
            self.estado_redes.sim_inicializada = true;
            self.registrarLog("[OK] Simulacion iniciada");
            self.estado_redes.simulador.enviarFlujoPreset(red, self.estado_redes.preset_trafico + 1);
            self.estado_redes.mostrar_modal_inicio = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Inicia la simulacion con el preset seleccionado.");
    } else {
        auto& sim = self.estado_redes.simulador;
        auto& est = sim.estado;
        ImGui::Text("%s T: %.1fs | %s Flujos: %d | %s Pkts: %d",
            ICON_FA_CLOCK, est.tiempo,
            ICON_FA_RIGHT_LEFT, (int)est.flujos.size(),
            ICON_FA_DIAGRAM_PROJECT, (int)sim.obtenerPaquetes().size());

        ImGui::SliderFloat("Velocidad##sim", &est.velocidad, 0.1f, 5.0f, "x%.1f");
        ImGui::Checkbox("Eventos automaticos", &sim.eventos_automaticos);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Genera spikes de trafico y microcortes aleatorios automaticamente.");

        float bw = (ImGui::GetContentRegionAvail().x - 8) / 2.0f;
        if (est.activa) {
            if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 28))) est.activa = false;
        } else {
            if (ImGui::Button(ICON_FA_PLAY " Reanudar", ImVec2(bw, 28))) est.activa = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reiniciar", ImVec2(bw, 28))) {
            sim.inicializar(red);
            self.registrarLog("[OK] Simulacion reiniciada");
        }

        // presets rapidos
        ImGui::Spacing();
        ImGui::TextDisabled("Generar trafico:");
        if (ImGui::SmallButton("Videollamada")) sim.enviarFlujoPreset(red, 1);
        ImGui::SameLine();
        if (ImGui::SmallButton("Empresarial")) sim.enviarFlujoPreset(red, 2);
        ImGui::SameLine();
        if (ImGui::SmallButton("DDoS")) sim.enviarFlujoPreset(red, 3);
        ImGui::SameLine();
        if (ImGui::SmallButton("Supervivencia")) sim.enviarFlujoPreset(red, 4);
    }

    // Leyenda de colores de protocolo 
    if (self.estado_redes.sim_inicializada) {
        ImGui::Spacing();
        ImGui::TextDisabled("Protocolos:");
        struct Proto { const char* nombre; ImVec4 color; };
        Proto protos[] = {
            {"HTTP",  ImVec4(0.30f, 0.69f, 0.31f, 1.0f)},
            {"VIDEO", ImVec4(0.61f, 0.15f, 0.69f, 1.0f)},
            {"PING",  ImVec4(0.00f, 0.74f, 0.83f, 1.0f)},
            {"DDoS",  ImVec4(0.96f, 0.26f, 0.21f, 1.0f)},
            {"VoIP",  ImVec4(1.00f, 0.60f, 0.00f, 1.0f)},
            {"DNS",   ImVec4(0.13f, 0.59f, 0.95f, 1.0f)},
        };
        for (int i = 0; i < 6; i++) {
            ImGui::ColorButton(("##prot" + std::to_string(i)).c_str(), protos[i].color,
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(10, 10));
            ImGui::SameLine();
            ImGui::TextDisabled("%s", protos[i].nombre);
            if (i < 5) ImGui::SameLine(0, 12);
        }
    }

    // Estadisticas 
    if (self.estado_redes.sim_inicializada && self.estado_redes.mostrar_stats) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f),
            ICON_FA_CHART_LINE " ESTADISTICAS EN VIVO");
        ImGui::Spacing();

        auto& stats = self.estado_redes.simulador.estado.stats;
        float ancho_stats = ImGui::GetContentRegionAvail().x;

        ImGui::Text("%s Throughput: %.1f Mbps", ICON_FA_ARROW_RIGHT, stats.throughput_total_mbps);
        ImGui::Text("%s Perdidos: %d", ICON_FA_XMARK, self.estado_redes.simulador.totalPaquetesPerdidos());
        ImGui::Text("%s Entregados: %d", ICON_FA_CHECK, self.estado_redes.simulador.totalPaquetesEntregados());
        ImGui::Text("%s Latencia promedio: %.1f ms", ICON_FA_CLOCK, stats.latencia_promedio_ms);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float h_spark = 30.0f;

        ImGui::TextDisabled("Throughput (Mbps):");
        sparkline(dl, stats.hist_throughput.muestras, stats.hist_throughput.total,
            pos, ancho_stats, h_spark, IM_COL32(0, 200, 255, 200));
        ImGui::Dummy(ImVec2(0, h_spark + 4));

        pos = ImGui::GetCursorScreenPos();
        ImGui::TextDisabled("Packet loss:");
        sparkline(dl, stats.hist_perdida.muestras, stats.hist_perdida.total,
            pos, ancho_stats, h_spark, IM_COL32(255, 100, 100, 200));
        ImGui::Dummy(ImVec2(0, h_spark + 4));
    }

    ImGui::Separator();

    // Enviar trafico 
    if (self.estado_redes.sim_inicializada) {
        if (ImGui::CollapsingHeader(ICON_FA_PAPER_PLANE " Enviar Trafico")) {
            const char* tipos_flujo[] = {"HTTP (10Mbps)", "Video (50Mbps)", "Ping (0.1Mbps)", "DDoS (500Mbps)", "VoIP (0.5Mbps)", "DNS (0.01Mbps)"};
            float mbps_flujo[] = {10.0f, 50.0f, 0.1f, 500.0f, 0.5f, 0.01f};
            const char* nombre_flujo[] = {"HTTP", "VIDEO", "PING", "DDOS", "VOIP", "DNS"};

            ImGui::Text("Origen:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##fl_orig", red.nombreNodo(self.estado_redes.flujo_origen).c_str())) {
                for (const auto& n : red.nodos) {
                    bool sel = (n.id == self.estado_redes.flujo_origen);
                    if (ImGui::Selectable(n.nombre.c_str(), sel)) self.estado_redes.flujo_origen = n.id;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Text("Destino:");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##fl_dst", red.nombreNodo(self.estado_redes.flujo_destino).c_str())) {
                for (const auto& n : red.nodos) {
                    bool sel = (n.id == self.estado_redes.flujo_destino);
                    if (ImGui::Selectable(n.nombre.c_str(), sel)) self.estado_redes.flujo_destino = n.id;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            int tipo_idx = self.estado_redes.flujo_tipo;
            uint32_t col_preview = ColoresProtocolo::paraTipo(nombre_flujo[tipo_idx % 6]);
            ImVec4 col_preview_f(
                ((col_preview >> 16) & 0xFF) / 255.0f,
                ((col_preview >> 8) & 0xFF) / 255.0f,
                (col_preview & 0xFF) / 255.0f,
                1.0f
            );
            ImGui::ColorButton("##col", col_preview_f, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(12, 12));
            ImGui::SameLine();
            ImGui::Combo("Tipo##flujo", &self.estado_redes.flujo_tipo, tipos_flujo, 6);
            ImGui::SliderFloat("Duracion (s)", &self.estado_redes.flujo_dur, 1.0f, 60.0f, "%.0f s");

            if (ImGui::Button(ICON_FA_PAPER_PLANE " Enviar", ImVec2(-1, 28))) {
                if (self.estado_redes.flujo_origen != self.estado_redes.flujo_destino) {
                    self.estado_redes.simulador.enviarFlujo(self.estado_redes.flujo_origen, self.estado_redes.flujo_destino,
                        mbps_flujo[self.estado_redes.flujo_tipo], nombre_flujo[self.estado_redes.flujo_tipo],
                        self.estado_redes.flujo_dur, red);
                    g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Crea un flujo entre origen y destino.");
        }

        // Simular fallos 
        if (ImGui::CollapsingHeader(ICON_FA_TRIANGLE_EXCLAMATION " Simular Fallos")) {
            ImGui::TextDisabled("Selecciona un nodo o arista en el lienzo:");
            if (self.estado_ui.nodo_seleccionado >= 0) {
                if (!self.estado_redes.simulador.estado.nodos.count(self.estado_ui.nodo_seleccionado)) {
                    ImGui::TextDisabled("Nodo no registrado en la simulacion");
                } else {
                    auto& en = self.estado_redes.simulador.estado.nodos[self.estado_ui.nodo_seleccionado];
                    ImGui::Text(ICON_FA_SERVER " Nodo: %s", red.nombreNodo(self.estado_ui.nodo_seleccionado).c_str());
                    if (en.activo) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                        if (ImGui::Button(ICON_FA_BOLT " Derribar Nodo", ImVec2(-1, 0))) {
                            self.estado_redes.simulador.simularFalloNodo(self.estado_ui.nodo_seleccionado, red);
                            g_sonidos.reproducir(Sonidos::NODO_CAIDO);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Desactiva el nodo para simular falla de hardware.");
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                            ICON_FA_TRIANGLE_EXCLAMATION " CAIDO (%.0fs)", en.tiempo_caida);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                        if (ImGui::Button(ICON_FA_ROTATE_LEFT " Restaurar Nodo", ImVec2(-1, 0))) {
                            self.estado_redes.simulador.restaurarNodo(self.estado_ui.nodo_seleccionado, red);
                            g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reactiva el nodo y restablece sus conexiones.");
                        ImGui::PopStyleColor();
                    }
                }
            } else {
                ImGui::TextDisabled("Ningun nodo seleccionado");
            }

            // fallo de arista individual
            ImGui::Spacing();
            ImGui::TextDisabled("Derribar enlace:");
            if (red.aristas.size() > 0) {
                static int arista_seleccionada = 0;
                if (arista_seleccionada >= (int)red.aristas.size()) arista_seleccionada = 0;
                
                std::string preview = red.nombreNodo(red.aristas[arista_seleccionada].origen_id) + " <-> " +
                                     red.nombreNodo(red.aristas[arista_seleccionada].destino_id);
                ImGui::SetNextItemWidth(-1);
                if (ImGui::BeginCombo("##arista_fallo", preview.c_str())) {
                    for (int i = 0; i < (int)red.aristas.size(); i++) {
                        std::string label = red.nombreNodo(red.aristas[i].origen_id) + " <-> " +
                                           red.nombreNodo(red.aristas[i].destino_id) +
                                           " (" + std::to_string((int)red.aristas[i].peso) + "ms)";
                        if (ImGui::Selectable(label.c_str(), arista_seleccionada == i))
                            arista_seleccionada = i;
                    }
                    ImGui::EndCombo();
                }
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.3f, 0.1f, 1.0f));
                if (ImGui::Button(ICON_FA_LINK_SLASH " Cortar Enlace", ImVec2(-1, 0))) {
                    self.estado_redes.simulador.simularFalloArista(
                        red.aristas[arista_seleccionada].origen_id,
                        red.aristas[arista_seleccionada].destino_id, red);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simula un corte de cable en el enlace seleccionado.");
                ImGui::PopStyleColor();
            }
        }

        // Tabla de ruteo 
        if (ImGui::CollapsingHeader(ICON_FA_TABLE_LIST " Tabla de Ruteo")) {
            if (self.estado_ui.nodo_seleccionado >= 0) {
                ImGui::Text(ICON_FA_SERVER " Rutas desde: %s", red.nombreNodo(self.estado_ui.nodo_seleccionado).c_str());
                auto tabla = self.estado_redes.simulador.tablaRuteo(red, self.estado_ui.nodo_seleccionado);
                if (tabla.empty()) {
                    ImGui::TextDisabled("Sin rutas (nodo aislado o caido)");
                } else {
                    if (ImGui::BeginTable("tabRuteo", 2,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp,
                        ImVec2(0, 120)))
                    {
                        ImGui::TableSetupColumn("Destino", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Sig. Salto", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableHeadersRow();
                        for (const auto& [dst_id, next_hop] : tabla) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", red.nombreNodo(dst_id).c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.6f, 1.0f), "%s", next_hop.c_str());
                        }
                        ImGui::EndTable();
                    }
                }
            } else {
                ImGui::TextDisabled("Selecciona un nodo para ver su tabla de ruteo");
            }
        }

        // Estado de nodos 
        if (ImGui::CollapsingHeader(ICON_FA_CHART_BAR " Estado de Nodos")) {
            if (ImGui::BeginTable("tabNodos", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY,
                ImVec2(0, 150)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Nodo",  ImGuiTableColumnFlags_WidthFixed, 55);
                ImGui::TableSetupColumn("CPU",   ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("RAM",   ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("RX",    ImGuiTableColumnFlags_WidthFixed, 35);
                ImGui::TableSetupColumn("TX",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& n : red.nodos) {
                    if (!self.estado_redes.simulador.estado.nodos.count(n.id)) continue;
                    const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
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

        // Timeline 
        if (self.estado_redes.mostrar_timeline && !self.estado_redes.simulador.estado.timeline.empty()) {
            if (ImGui::CollapsingHeader(ICON_FA_CLOCK " Timeline de Eventos", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& timeline = self.estado_redes.simulador.estado.timeline;
                ImDrawList* dl_tl = ImGui::GetWindowDrawList();
                ImVec2 tl_pos = ImGui::GetCursorScreenPos();
                float tl_w = ImGui::GetContentRegionAvail().x;
                float tl_h = 28.0f;

                dl_tl->AddRectFilled(tl_pos, ImVec2(tl_pos.x + tl_w, tl_pos.y + tl_h), IM_COL32(15, 15, 20, 180), 3.0f);
                float t_max = self.estado_redes.simulador.estado.tiempo;
                if (t_max < 1.0f) t_max = 1.0f;

                for (const auto& ev : timeline) {
                    float x = tl_pos.x + (ev.tiempo / t_max) * tl_w;
                    uint32_t c = ev.color;
                    ImVec4 col_f(
                        ((c >> 16) & 0xFF) / 255.0f,
                        ((c >> 8) & 0xFF) / 255.0f,
                        (c & 0xFF) / 255.0f, 1.0f);
                    float marker_h = tl_h * (ev.duracion > 0 ? 0.5f : 0.8f);
                    dl_tl->AddLine(ImVec2(x, tl_pos.y + tl_h - marker_h), ImVec2(x, tl_pos.y + tl_h), ev.color, 2.5f);
                    if (ImGui::IsMouseHoveringRect(ImVec2(x - 3, tl_pos.y), ImVec2(x + 3, tl_pos.y + tl_h))) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(col_f, "%s", ev.etiqueta.c_str());
                        ImGui::Text("t=%.1fs", ev.tiempo);
                        ImGui::EndTooltip();
                    }
                }
                ImGui::Dummy(ImVec2(0, tl_h + 6));
            }
        }

        // Log 
        if (ImGui::CollapsingHeader(ICON_FA_LIST " Log de Eventos Red")) {
            ImGui::BeginChild("LogRed", ImVec2(0, 120), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (auto it = self.estado_redes.simulador.estado.log_eventos.rbegin();
                 it != self.estado_redes.simulador.estado.log_eventos.rend(); ++it)
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

    // Inspector de paquetes 
    if (self.estado_redes.mostrar_inspector && self.estado_redes.sim_inicializada) {
        int target_id = self.estado_redes.paquete_inspector_id;
        bool encontrado = false;
        for (const auto& pkt : self.estado_redes.simulador.obtenerPaquetes()) {
            if (pkt.id == target_id) {
                encontrado = true;
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f),
                    ICON_FA_CIRCLE_INFO " INSPECTOR DE PAQUETE");

                uint32_t col_uint = ColoresProtocolo::paraTipo(pkt.tipo);
                ImVec4 col_pkt(
                    ((col_uint >> 16) & 0xFF) / 255.0f,
                    ((col_uint >> 8) & 0xFF) / 255.0f,
                    (col_uint & 0xFF) / 255.0f, 1.0f);
                ImGui::ColorButton("##colPkt", col_pkt, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(14, 14));
                ImGui::SameLine();
                ImGui::Text("Protocolo: %s", pkt.tipo.c_str());
                ImGui::Text("Origen:  %s", red.nombreNodo(pkt.origen_id).c_str());
                ImGui::Text("Destino: %s", red.nombreNodo(pkt.destino_id).c_str());
                ImGui::Text("Ancho de banda: %.1f Mbps", pkt.mbps);
                ImGui::Text("Tamano: %.1f MB", pkt.tamaño_mb);
                ImGui::Text("Salto: %d / %d", pkt.paso_actual, (int)pkt.ruta.size() - 1);

                ImGui::TextDisabled("Ruta:");
                std::string ruta_str;
                for (size_t i = 0; i < pkt.ruta.size(); i++) {
                    if (i > 0) ruta_str += " → ";
                    ruta_str += red.nombreNodo(pkt.ruta[i]);
                }
                ImGui::TextWrapped("%s", ruta_str.c_str());

                if (ImGui::SmallButton(ICON_FA_XMARK " Cerrar inspector")) {
                    self.estado_redes.mostrar_inspector = false;
                    self.estado_redes.paquete_inspector_id = -1;
                }
                break;
            }
        }
        if (!encontrado) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "El paquete ya llego a su destino.");
            if (ImGui::SmallButton("Cerrar")) {
                self.estado_redes.mostrar_inspector = false;
                self.estado_redes.paquete_inspector_id = -1;
            }
        }
    }

    // Popup de inicio 
    if (self.estado_redes.mostrar_modal_inicio) {
        ImGui::OpenPopup("Simulacion iniciada");
        self.estado_redes.mostrar_modal_inicio = false;
    }
    if (ImGui::BeginPopupModal("Simulacion iniciada", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_PLAY " Simulacion en marcha!");
        ImGui::Text("Trafico generado segun preset seleccionado.");
        ImGui::Text("Haz click en los paquetes viajando para inspeccionarlos.");
        ImGui::Text("Usa 'Simular Fallos' para derribar nodos y ver como se recalculan las rutas.");
        ImGui::Separator();
        if (ImGui::Button("Entendido")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}

}
