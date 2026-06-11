#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/algoritmos/Dijkstra.h"
#include "nucleo/algoritmos/Kruskal.h"
#include "nucleo/algoritmos/BFS.h"
#include "nucleo/algoritmos/DFS.h"
#include "nucleo/algoritmos/Ciclos.h"
#include "nucleo/algoritmos/Coloreo.h"
#include "nucleo/algoritmos/Arbol.h"
#include "interfaz/util/Animacion.h"

class Interfaz;
namespace PanelIsomorfismo {
    inline void dibujar(Interfaz& self, Grafo& red);
}

// PanelGrafos: namespace que agrupa funciones del panel "Herramientas de Red"
// y sus subpaneles de algoritmos. El subpanel Isomorfismo se delega a PanelIsomorfismo.
namespace PanelGrafos {

// ── Selector de modo (Grafos / Redes) ─────────────────────────────────────
inline void selectorModo(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "MODO DE OPERACION");
    float ancho = ImGui::GetContentRegionAvail().x;
    ImVec2 btnSize(ancho * 0.48f, 32);

    bool en_grafos = (self.modo_actual == Interfaz::ModoApp::Grafos);
    if (en_grafos) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
    if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " GRAFOS", btnSize)) {
        self.modo_actual = Interfaz::ModoApp::Grafos;
        self.registrarLog("Modo cambiado: Grafos");
    }
    if (en_grafos) ImGui::PopStyleColor();

    ImGui::SameLine();

    bool en_redes = (self.modo_actual == Interfaz::ModoApp::Redes);
    if (en_redes) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
    if (ImGui::Button(ICON_FA_NETWORK_WIRED " REDES", btnSize)) {
        self.modo_actual = Interfaz::ModoApp::Redes;
        self.registrarLog("Modo cambiado: Redes");
    }
    if (en_redes) ImGui::PopStyleColor();
}

// ── Controles de animacion (Play / Pausa / Paso / Reset) ──────────────────
inline void controlesAnimacion(Interfaz& self) {
    float ancho = ImGui::GetContentRegionAvail().x;

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
        ICON_FA_FILM " ANIMACION");

    float progreso = 0.0f;
    if (!self.anim_estado.pasos.empty())
        progreso = (float)(self.anim_estado.paso_actual + 1) / (float)self.anim_estado.pasos.size();

    ImGui::ProgressBar(progreso, ImVec2(-1, 8));
    ImGui::Text("Paso %d / %d", self.anim_estado.paso_actual + 1, (int)self.anim_estado.pasos.size());

    if (self.anim_estado.paso_actual >= 0 && self.anim_estado.paso_actual < (int)self.anim_estado.pasos.size()) {
        const auto& paso = self.anim_estado.pasos[self.anim_estado.paso_actual];
        if (!paso.descripcion.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 230, 255, 255));
            ImGui::TextWrapped("%s", paso.descripcion.c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::SliderFloat("Velocidad##anim", &self.anim_estado.velocidad_paso, 0.05f, 2.0f, "%.2f s/paso");

    float bw = (ancho - 16) / 3.0f;
    if (self.anim_estado.activa && !self.anim_estado.pausada) {
        if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 0)))
            self.anim_estado.pausada = true;
    } else {
        if (ImGui::Button(ICON_FA_PLAY " Play", ImVec2(bw, 0))) {
            if (!self.anim_estado.activa && self.anim_estado.paso_actual >= 0) self.anim_estado.activa = true;
            self.anim_estado.pausada = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FORWARD_STEP " Paso", ImVec2(bw, 0))) {
        if (self.anim_estado.paso_actual + 1 < (int)self.anim_estado.pasos.size()) {
            self.anim_estado.paso_actual++;
            AnimacionUI::aplicarPaso(self, self.anim_estado.pasos[self.anim_estado.paso_actual]);
            self.anim_estado.pausada = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reset", ImVec2(bw, 0))) {
        AnimacionUI::reset(self);
    }
}

// ── Propiedades del nodo seleccionado + tabla de aristas editables ─────────
inline void propiedadesNodo(Interfaz& self, Grafo& red) {
    if (self.nodo_seleccionado >= 0) {
        Nodo* n = red.obtenerNodo(self.nodo_seleccionado);
        if (n) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), ICON_FA_CIRCLE_INFO " PROPIEDADES");
            ImGui::Text("ID: %d", n->id);

            snprintf(self.buffer_nombre, sizeof(self.buffer_nombre), "%s", n->nombre.c_str());
            if (ImGui::InputText("Nombre", self.buffer_nombre, sizeof(self.buffer_nombre))) {
                n->nombre = self.buffer_nombre;
            }

            if (self.modo_actual == Interfaz::ModoApp::Redes) {
                int tipo_int = (int)n->tipo;
                const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
                if (ImGui::Combo("Tipo", &tipo_int, tipos, 5)) {
                    n->tipo = (TipoHardware)tipo_int;
                    self.registrarLog("Tipo cambiado: " + n->nombre + " -> " + tipos[tipo_int]);
                }
                ImGui::Text("Latencia: +%.0f ms", latenciaHardware(n->tipo));
            }

            ImGui::Spacing();
            if (ImGui::Button(ICON_FA_TRASH_CAN " Eliminar Nodo", ImVec2(-1, 0))) {
                self.registrarLog("Nodo eliminado: " + n->nombre);
                red.eliminarNodo(self.nodo_seleccionado);
                self.nodo_seleccionado = -1;
                self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
            }
        }
    }

    // --- Tabla de aristas editables ---
    if (!red.aristas.empty() && ImGui::CollapsingHeader(ICON_FA_LINK " Aristas")) {
        for (size_t i = 0; i < red.aristas.size(); i++) {
            auto& a = red.aristas[i];
            ImGui::PushID((int)i);
            ImGui::Text("%s - %s", red.nombreNodo(a.origen_id).c_str(), red.nombreNodo(a.destino_id).c_str());
            ImGui::SameLine(180);
            ImGui::SetNextItemWidth(80);
            if (ImGui::InputFloat("##peso", &a.peso, 0, 0, "%.1f")) {
                a.peso_actual = a.peso;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_TRASH_CAN)) {
                self.registrarLog("Arista eliminada: " + red.nombreNodo(a.origen_id) + " - " + red.nombreNodo(a.destino_id));
                red.aristas.erase(red.aristas.begin() + i);
                self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
    }
}

// ── Menu general ──────────────────────────────────────────────────────────
inline void menuGeneral(Interfaz& self, Grafo& red) {
    float ancho = ImGui::GetContentRegionAvail().x;

    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_ROUTE " ENRUTAMIENTO");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.35f, 0.28f, 1.0f));
    if (ImGui::Button(ICON_FA_DIAMOND " Dijkstra — Ruta Optima", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Dijkstra;
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
        ICON_FA_CIRCLE_NODES " TOPOLOGIA OPTIMA");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
    if (ImGui::Button(ICON_FA_TREE " Kruskal — MST", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Kruskal;
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
        ICON_FA_MAGNIFYING_GLASS " RECORRIDO");
    float bw = (ancho - 8) / 2.0f;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.18f, 0.38f, 1.0f));
    if (ImGui::Button(ICON_FA_LAYER_GROUP " BFS", ImVec2(bw, 36))) {
        self.modo_panel = Interfaz::ModoPanel::BFS;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_CODE_BRANCH " DFS", ImVec2(bw, 36))) {
        self.modo_panel = Interfaz::ModoPanel::DFS;
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.1f, 1.0f),
        ICON_FA_CIRCLE_EXCLAMATION " ANALISIS");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.15f, 0.05f, 1.0f));
    if (ImGui::Button(ICON_FA_ROTATE " Detectar Ciclos", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Ciclos;
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.28f, 1.0f));
    if (ImGui::Button(ICON_FA_PAINTBRUSH " Coloreo Greedy", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Coloreo;
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.20f, 0.25f, 1.0f));
    if (ImGui::Button(ICON_FA_OBJECT_GROUP " Isomorfismo", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Isomorfismo;
        self.resetGrafoIsomorfismo();
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.28f, 0.10f, 1.0f));
    if (ImGui::Button(ICON_FA_SITEMAP " Analisis de Arbol", ImVec2(-1, 36))) {
        self.modo_panel = Interfaz::ModoPanel::Arbol;
        self.arbol_analizado = false;
        self.arbol_layout_aplicado = false;
    }
    ImGui::PopStyleColor();

    if (self.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            ICON_FA_WAVE_SQUARE " SIMULACION");
        ImGui::Checkbox("Activar Jitter", &self.simulacion_jitter);
        if (self.simulacion_jitter) {
            ImGui::SliderFloat("Intensidad##jitter", &self.jitter_porcentaje, 0.01f, 0.50f, "%.0f%%");
        }
    }
}

// ── Subpanel: Dijkstra ────────────────────────────────────────────────────
inline void subpanelDijkstra(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_DIAMOND " DIJKSTRA");
    ImGui::TextWrapped(
        "Encuentra la ruta de menor costo entre dos nodos. "
        "En redes: equivale al protocolo OSPF de enrutamiento.");
    ImGui::Spacing();

    ImGui::Text("Origen:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##origen_d", red.nombreNodo(self.dijkstra_origen).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.dijkstra_origen);
            if (ImGui::Selectable(n.nombre.c_str(), sel))
                self.dijkstra_origen = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Text("Destino:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##destino_d", red.nombreNodo(self.dijkstra_destino).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.dijkstra_destino);
            if (ImGui::Selectable(n.nombre.c_str(), sel))
                self.dijkstra_destino = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (self.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::Checkbox("Incluir latencia de hardware", &self.dijkstra_usar_latencia);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Suma la latencia de cada tipo de equipo al costo de la ruta");
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::generarPasos(
            red, self.dijkstra_origen, self.dijkstra_destino, self.dijkstra_usar_latencia);
        AnimacionUI::iniciar(self, pasos);
        self.ruta_optima.clear(); self.mostrar_mst = false;
        self.registrarLog("[Dijkstra] Animacion iniciada: " +
            red.nombreNodo(self.dijkstra_origen) + " -> " + red.nombreNodo(self.dijkstra_destino));
    }
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        auto res = Algoritmos::dijkstra(
            red, self.dijkstra_origen, self.dijkstra_destino, self.dijkstra_usar_latencia);
        self.ruta_optima = res.ruta;
        self.mostrar_mst = false; AnimacionUI::reset(self);
        if (res.hay_ruta)
            self.registrarLog("[OK] Dijkstra: " + std::to_string(res.saltos) +
                " saltos, costo=" + std::to_string((int)res.costo_total));
        else
            self.registrarLog("[!] Dijkstra: no existe ruta entre esos nodos");
    }

    ImGui::Separator();
    if (!self.ruta_optima.empty()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
            ICON_FA_CHECK " Ruta encontrada:");
        std::string ruta_str;
        for (size_t i = 0; i < self.ruta_optima.size(); i++) {
            if (i > 0) ruta_str += " -> ";
            ruta_str += red.nombreNodo(self.ruta_optima[i]);
        }
        ImGui::TextWrapped("%s", ruta_str.c_str());
        ImGui::Text("Saltos: %d", (int)self.ruta_optima.size() - 1);
    } else if (!self.anim_estado.activa) {
        ImGui::TextDisabled("Ejecuta un algoritmo para ver resultados.");
    }

    if (self.anim_estado.activa && !self.dijkstra_tabla_dist.empty()) {
        ImGui::Separator();
        ImGui::Text("Tabla de distancias:");
        if (ImGui::BeginTable("tabDist", 2,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Nodo", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Dist", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (const auto& n : red.nodos) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", n.nombre.c_str());
                ImGui::TableSetColumnIndex(1);
                float d = (n.id < (int)self.dijkstra_tabla_dist.size())
                    ? self.dijkstra_tabla_dist[n.id]
                    : std::numeric_limits<float>::infinity();
                if (d == std::numeric_limits<float>::infinity())
                    ImGui::TextDisabled("INF");
                else
                    ImGui::Text("%.1f", d);
            }
            ImGui::EndTable();
        }
    }
}

// ── Subpanel: Kruskal (MST) ───────────────────────────────────────────────
inline void subpanelKruskal(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
        ICON_FA_TREE " KRUSKAL — MST");
    ImGui::TextWrapped(
        "Encuentra el Arbol de Expansion Minima: la red que conecta "
        "todos los nodos con el menor costo total. "
        "En redes: equivale al protocolo STP (Spanning Tree Protocol) "
        "que evita loops en switches.");
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::Kruskal::generarPasos(red);
        AnimacionUI::iniciar(self, pasos);
        self.ruta_optima.clear(); self.mostrar_mst = false;
        self.registrarLog("[Kruskal] Animacion MST iniciada");
    }
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        auto res = Algoritmos::Kruskal::kruskal(red);
        self.aristas_mst = res.aristas_mst;
        self.mostrar_mst = true;
        self.ruta_optima.clear(); AnimacionUI::reset(self);
        self.registrarLog("[OK] Kruskal MST: " + std::to_string(res.aristas_aceptadas) +
            " aristas, peso total=" + std::to_string((int)res.peso_total));
    }

    if (self.mostrar_mst && !self.aristas_mst.empty()) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
            ICON_FA_CHECK " MST calculado:");

        float peso_total_grafo = 0;
        for (const auto& a : red.aristas) peso_total_grafo += a.peso;

        float peso_mst = 0;
        for (const auto& a : self.aristas_mst) peso_mst += a.peso;

        ImGui::Text("Aristas en MST:  %d / %d",
            (int)self.aristas_mst.size(), (int)red.aristas.size());
        ImGui::Text("Peso MST:        %.1f", peso_mst);
        ImGui::Text("Peso total red:  %.1f", peso_total_grafo);

        float ahorro = peso_total_grafo - peso_mst;
        if (ahorro > 0) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                "Ahorro vs. red completa: %.1f", ahorro);
        }

        ImGui::Separator();
        ImGui::Text("Aristas del MST:");
        for (const auto& a : self.aristas_mst) {
            ImGui::BulletText("%s - %s (%.1f)",
                red.nombreNodo(a.origen_id).c_str(),
                red.nombreNodo(a.destino_id).c_str(),
                a.peso);
        }
    }
}

// ── Subpanel: BFS ─────────────────────────────────────────────────────────
inline void subpanelBFS(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
        ICON_FA_LAYER_GROUP " BFS — Busqueda en Anchura");
    ImGui::TextWrapped(
        "Explora el grafo nivel por nivel desde un nodo origen. "
        "Garantiza encontrar el camino con MENOS SALTOS. "
        "En redes: descubre todos los equipos a N saltos de distancia "
        "(broadcast domain discovery).");
    ImGui::Spacing();

    ImGui::Text("Nodo de inicio:");
    ImGui::SetNextItemWidth(-1);
    if (red.nodos.empty()) {
        ImGui::TextDisabled("No hay nodos");
    } else {
        if (!red.obtenerNodo(self.bfs_nodo_inicio) && !red.nodos.empty())
            self.bfs_nodo_inicio = red.nodos[0].id;

        if (ImGui::BeginCombo("##inicio_bfs", red.nombreNodo(self.bfs_nodo_inicio).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == self.bfs_nodo_inicio);
                if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                    self.bfs_nodo_inicio = n.id;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_PLAY " Animacion BFS", ImVec2(-1, 32))) {
            auto pasos = Algoritmos::BFS::generarPasos(red, self.bfs_nodo_inicio);
            AnimacionUI::iniciar(self, pasos);
            self.bfs_resultado = Algoritmos::BFS::bfs(red, self.bfs_nodo_inicio);
            self.registrarLog("[BFS] Iniciado desde " + red.nombreNodo(self.bfs_nodo_inicio));
        }
        if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
            self.bfs_resultado = Algoritmos::BFS::bfs(red, self.bfs_nodo_inicio);
            AnimacionUI::reset(self);
            self.registrarLog("[OK] BFS: " + std::to_string(self.bfs_resultado.orden_visita.size()) +
                " nodos visitados");
        }

        if (!self.bfs_resultado.nivel.empty()) {
            ImGui::Separator();
            ImGui::Text("Distancias desde %s:", red.nombreNodo(self.bfs_nodo_inicio).c_str());
            if (ImGui::BeginTable("tabBFS", 2,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Nodo", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn("Saltos", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                for (const auto& n : red.nodos) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", n.nombre.c_str());
                    ImGui::TableSetColumnIndex(1);
                    auto it = self.bfs_resultado.nivel.find(n.id);
                    if (it != self.bfs_resultado.nivel.end())
                        ImGui::Text("%d saltos", it->second);
                    else
                        ImGui::TextDisabled("inaccesible");
                }
                ImGui::EndTable();
            }
        }
    }
}

// ── Subpanel: DFS ─────────────────────────────────────────────────────────
inline void subpanelDFS(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f),
        ICON_FA_CODE_BRANCH " DFS — Busqueda en Profundidad");
    ImGui::TextWrapped(
        "Explora el grafo yendo tan profundo como sea posible antes de retroceder. "
        "Las aristas que vuelven a un nodo ya visitado (back-edges) indican ciclos.");
    ImGui::Spacing();

    if (!red.obtenerNodo(self.dfs_nodo_inicio) && !red.nodos.empty())
        self.dfs_nodo_inicio = red.nodos[0].id;

    ImGui::Text("Nodo de inicio:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##inicio_dfs", red.nombreNodo(self.dfs_nodo_inicio).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.dfs_nodo_inicio);
            if (ImGui::Selectable(n.nombre.c_str(), sel)) self.dfs_nodo_inicio = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion DFS", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::DFS::generarPasos(red, self.dfs_nodo_inicio);
        AnimacionUI::iniciar(self, pasos);
        self.registrarLog("[DFS] Iniciado desde " + red.nombreNodo(self.dfs_nodo_inicio));
    }
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        self.dfs_resultado = Algoritmos::DFS::dfs(red, self.dfs_nodo_inicio);
        AnimacionUI::reset(self);
        self.registrarLog("[OK] DFS: " + std::to_string(self.dfs_resultado.orden_visita.size()) +
            " nodos, " + std::to_string(self.dfs_resultado.back_edges.size()) + " back-edges");
    }

    if (!self.dfs_resultado.orden_visita.empty()) {
        ImGui::Separator();
        ImGui::Text("Orden de visita:");
        std::string orden;
        for (size_t i = 0; i < self.dfs_resultado.orden_visita.size(); i++) {
            if (i > 0) orden += " -> ";
            orden += red.nombreNodo(self.dfs_resultado.orden_visita[i]);
        }
        ImGui::TextWrapped("%s", orden.c_str());

        if (!self.dfs_resultado.back_edges.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.2f, 1.0f),
                ICON_FA_TRIANGLE_EXCLAMATION " Back-edges (ciclos):");
            for (const auto& [u, v] : self.dfs_resultado.back_edges) {
                ImGui::BulletText("%s -> %s",
                    red.nombreNodo(u).c_str(), red.nombreNodo(v).c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " Sin back-edges — no hay ciclos");
        }
    }
}

// ── Subpanel: Ciclos ──────────────────────────────────────────────────────
inline void subpanelCiclos(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.1f, 1.0f),
        ICON_FA_ROTATE " DETECCION DE CICLOS");
    ImGui::TextWrapped(
        "Un ciclo es un camino que comienza y termina en el mismo nodo. "
        "Un grafo sin ciclos (y conexo) es un arbol. "
        "En redes: los ciclos en switches causan broadcast storms — "
        "el STP los elimina automaticamente.");
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar", ImVec2(-1, 36))) {
        self.resultado_ciclos = Algoritmos::detectarCiclos(red);
        self.ciclo_analizado = true;
        if (self.resultado_ciclos.tiene_ciclo)
            self.registrarLog("[!] Ciclo detectado: " + self.resultado_ciclos.descripcion);
        else
            self.registrarLog("[OK] Grafo aciclico: " + self.resultado_ciclos.descripcion);
    }

    if (self.ciclo_analizado) {
        ImGui::Separator();
        if (self.resultado_ciclos.tiene_ciclo) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                ICON_FA_TRIANGLE_EXCLAMATION " CICLO DETECTADO");
            ImGui::TextWrapped("%s", self.resultado_ciclos.descripcion.c_str());
            ImGui::Spacing();
            ImGui::Text("Aristas que forman el ciclo:");
            for (const auto& [u, v] : self.resultado_ciclos.aristas_ciclo) {
                ImGui::BulletText("%s - %s",
                    red.nombreNodo(u).c_str(), red.nombreNodo(v).c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " Grafo aciclico");
            ImGui::TextWrapped("%s", self.resultado_ciclos.descripcion.c_str());
        }
    }
}

// ── Subpanel: Coloreo ─────────────────────────────────────────────────────
inline void subpanelColoreo(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.7f, 0.1f, 0.7f, 1.0f),
        ICON_FA_PAINTBRUSH " COLOREO GREEDY");
    ImGui::TextWrapped(
        "Asigna colores a los nodos de forma que nodos adyacentes tengan colores distintos. "
        "El numero cromatico es el minimo de colores necesarios. "
        "Aplicacion real: asignacion de canales WiFi para evitar interferencias.");
    ImGui::Spacing();

    if (ImGui::Button(self.mostrar_coloreo
        ? ICON_FA_EYE_SLASH " Ocultar Coloreo"
        : ICON_FA_PAINT_ROLLER " Aplicar Coloreo", ImVec2(-1, 36)))
    {
        self.mostrar_coloreo = !self.mostrar_coloreo;
        if (self.mostrar_coloreo) {
            self.resultado_coloreo = Algoritmos::coloreoGreedy(red);
            self.colores_nodos = self.resultado_coloreo.colores;
            self.registrarLog("[OK] Coloreo: " + std::to_string(self.resultado_coloreo.num_colores) +
                " colores usados (numero cromatico greedy)");
        }
    }

    if (self.mostrar_coloreo && !self.colores_nodos.empty()) {
        ImGui::Separator();
        ImGui::Text("Numero cromatico (greedy): %d", self.resultado_coloreo.num_colores);
        ImGui::TextDisabled("Nota: greedy no garantiza el optimo.");
        ImGui::TextDisabled("El optimo puede ser menor o igual.");
        ImGui::Spacing();
        ImGui::Text("Asignacion de colores:");
        const char* nombres_colores[] = {
            "Rojo", "Verde", "Azul", "Amarillo", "Magenta", "Cyan"
        };
        for (const auto& n : red.nodos) {
            if (n.id < (int)self.colores_nodos.size() && self.colores_nodos[n.id] >= 0) {
                int c = self.colores_nodos[n.id];
                ImGui::BulletText("%s -> Color %d (%s)",
                    n.nombre.c_str(), c,
                    c < 6 ? nombres_colores[c] : "Color extra");
            }
        }
    }
}

// ── Layout jerarquico para arbol ──────────────────────────────────────────
inline void aplicarLayoutArbol(Grafo& red, const Algoritmos::Arbol::PropiedadesArbol& props,
                             ImVec2 origen_canvas) {
    if (!props.es_arbol) return;

    std::map<int, std::vector<int>> nodos_por_nivel;
    for (const auto& n : red.nodos) {
        if (props.nivel.count(n.id))
            nodos_por_nivel[props.nivel.at(n.id)].push_back(n.id);
    }

    float espacio_h  = 120.0f;
    float espacio_v  = 90.0f;
    float cx         = origen_canvas.x;
    float cy_base    = origen_canvas.y + 80.0f;

    for (const auto& [nivel, ids] : nodos_por_nivel) {
        float total_w = (float)ids.size() * espacio_v;
        float x_start = cx - total_w / 2.0f + espacio_v / 2.0f;
        for (size_t i = 0; i < ids.size(); i++) {
            Nodo* n = red.obtenerNodo(ids[i]);
            if (n) {
                n->posicion.x = x_start + (float)i * espacio_v;
                n->posicion.y = cy_base + (float)nivel * espacio_h;
            }
        }
    }
}

// ── Subpanel: Arbol ───────────────────────────────────────────────────────
inline void subpanelArbol(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
        ICON_FA_SITEMAP " ANALISIS DE ARBOL");
    ImGui::TextWrapped(
        "Un arbol es un grafo conexo sin ciclos. "
        "Para n nodos debe tener exactamente n-1 aristas.");
    ImGui::Spacing();

    std::string razon;
    bool puede_ser_arbol = Algoritmos::Arbol::verificarEsArbol(red, razon);

    if (!puede_ser_arbol) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
            ICON_FA_TRIANGLE_EXCLAMATION " No es un arbol:");
        ImGui::TextWrapped("%s", razon.c_str());
        ImGui::Spacing();
        ImGui::TextDisabled("Ajusta el grafo para que sea conexo y aciclico,");
        ImGui::TextDisabled("con exactamente n-1 aristas.");
        return;
    }

    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
        ICON_FA_CHECK " Es un arbol valido");
    ImGui::Spacing();

    ImGui::Text("Nodo raiz:");
    ImGui::SetNextItemWidth(-1);
    if (!red.nodos.empty() && !red.obtenerNodo(self.arbol_raiz_id))
        self.arbol_raiz_id = red.nodos[0].id;
    if (ImGui::BeginCombo("##raiz_arbol", red.nombreNodo(self.arbol_raiz_id).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.arbol_raiz_id);
            if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                self.arbol_raiz_id = n.id;
                self.arbol_analizado = false;
                self.arbol_layout_aplicado = false;
            }
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar Arbol", ImVec2(-1, 36))) {
        self.arbol_props = Algoritmos::Arbol::analizar(red, self.arbol_raiz_id);
        self.arbol_analizado = true;
        self.arbol_layout_aplicado = false;
        self.registrarLog("[OK] Arbol analizado. Altura=" + std::to_string(self.arbol_props.altura) +
            ", Grado=" + std::to_string(self.arbol_props.grado_arbol));
    }

    if (self.arbol_analizado && !self.arbol_layout_aplicado) {
        if (ImGui::Button(ICON_FA_SITEMAP " Aplicar Layout Jerarquico", ImVec2(-1, 28))) {
            ImVec2 centro(800.0f, 400.0f);
            aplicarLayoutArbol(red, self.arbol_props, centro);
            self.arbol_layout_aplicado = true;
            self.registrarLog("[OK] Layout jerarquico aplicado");
        }
    }

    if (self.arbol_analizado) {
        ImGui::Separator();

        ImGui::Text("Altura:              %d", self.arbol_props.altura);
        ImGui::Text("Grado del arbol:     %d", self.arbol_props.grado_arbol);
        ImGui::Text("Numero de hojas:     %d", (int)self.arbol_props.hojas.size());
        ImGui::Spacing();

        if (!self.arbol_props.rama_mas_larga.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Rama mas larga:");
            std::string rml;
            for (size_t i = 0; i < self.arbol_props.rama_mas_larga.size(); i++) {
                if (i > 0) rml += " -> ";
                rml += red.nombreNodo(self.arbol_props.rama_mas_larga[i]);
            }
            ImGui::TextWrapped("%s", rml.c_str());
        }
        if (!self.arbol_props.rama_mas_corta.empty()) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Rama mas corta:");
            std::string rmc;
            for (size_t i = 0; i < self.arbol_props.rama_mas_corta.size(); i++) {
                if (i > 0) rmc += " -> ";
                rmc += red.nombreNodo(self.arbol_props.rama_mas_corta[i]);
            }
            ImGui::TextWrapped("%s", rmc.c_str());
        }

        ImGui::Separator();
        ImGui::Text("Propiedades por nodo:");
        ImGui::TextDisabled("(selecciona un nodo en el lienzo para ver detalles)");

        if (ImGui::BeginTable("TabArbol", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY,
            ImVec2(0, 200)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Nodo",  ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Nivel", ImGuiTableColumnFlags_WidthFixed, 45);
            ImGui::TableSetupColumn("Grado", ImGuiTableColumnFlags_WidthFixed, 45);
            ImGui::TableSetupColumn("Tipo",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& n : red.nodos) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", n.nombre.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", self.arbol_props.nivel.count(n.id) ? self.arbol_props.nivel.at(n.id) : -1);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", self.arbol_props.hijos.count(n.id) ? (int)self.arbol_props.hijos.at(n.id).size() : 0);
                ImGui::TableSetColumnIndex(3);
                if (n.id == self.arbol_props.raiz_id) {
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Raiz");
                } else if (std::find(self.arbol_props.hojas.begin(), self.arbol_props.hojas.end(), n.id) != self.arbol_props.hojas.end()) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Hoja");
                } else {
                    ImGui::TextDisabled("Interno");
                }
            }
            ImGui::EndTable();
        }

        if (self.nodo_seleccionado >= 0 && self.arbol_props.nivel.count(self.nodo_seleccionado)) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.8f, 1.0f),
                "Detalles de %s:", red.nombreNodo(self.nodo_seleccionado).c_str());

            int nid = self.nodo_seleccionado;
            int pid = self.arbol_props.padre.count(nid) ? self.arbol_props.padre.at(nid) : -1;
            ImGui::Text("Padre:       %s", pid >= 0 ? red.nombreNodo(pid).c_str() : "(raiz)");

            const auto& hijos = self.arbol_props.hijos.at(nid);
            if (hijos.empty()) {
                ImGui::Text("Hijos:       (hoja)");
            } else {
                std::string sh;
                for (size_t i = 0; i < hijos.size(); i++) {
                    if (i > 0) sh += ", ";
                    sh += red.nombreNodo(hijos[i]);
                }
                ImGui::Text("Hijos:       %s", sh.c_str());
            }

            if (pid >= 0 && self.arbol_props.hermanos.count(pid)) {
                const auto& herm = self.arbol_props.hermanos.at(pid);
                std::string sh;
                for (int h : herm) {
                    if (h == nid) continue;
                    if (!sh.empty()) sh += ", ";
                    sh += red.nombreNodo(h);
                }
                ImGui::Text("Hermanos:    %s", sh.empty() ? "(ninguno)" : sh.c_str());
            }

            if (self.arbol_props.ancestros.count(nid)) {
                const auto& anc = self.arbol_props.ancestros.at(nid);
                std::string sa;
                for (size_t i = 0; i < anc.size(); i++) {
                    if (i > 0) sa += " -> ";
                    sa += red.nombreNodo(anc[i]);
                }
                ImGui::Text("Ancestros:   %s", sa.empty() ? "(raiz)" : sa.c_str());
            }

            if (self.arbol_props.descendientes.count(nid)) {
                ImGui::Text("Descendientes: %d nodos",
                    (int)self.arbol_props.descendientes.at(nid).size());
            }

            std::vector<std::string> mis_primos;
            for (const auto& [a, b] : self.arbol_props.primos) {
                if (a == nid) mis_primos.push_back(red.nombreNodo(b));
                else if (b == nid) mis_primos.push_back(red.nombreNodo(a));
            }
            if (!mis_primos.empty()) {
                std::string sp;
                for (size_t i = 0; i < mis_primos.size(); i++) {
                    if (i > 0) sp += ", ";
                    sp += mis_primos[i];
                }
                ImGui::Text("Primos:      %s", sp.c_str());
            } else {
                ImGui::Text("Primos:      (ninguno)");
            }
        }

        if (!self.arbol_props.primos.empty()) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Pares de primos")) {
                for (const auto& [a, b] : self.arbol_props.primos) {
                    ImGui::BulletText("%s y %s (nivel %d)",
                        red.nombreNodo(a).c_str(),
                        red.nombreNodo(b).c_str(),
                        self.arbol_props.nivel.at(a));
                }
            }
        }
    }
}

// ── Funcion principal: dibuja el panel "Herramientas de Red" completo ─────
inline void dibujar(Interfaz& self, Grafo& red) {
    ImGui::Begin("Herramientas de Red");

    selectorModo(self, red);

    ImGui::Separator();

    if (self.modo_panel != Interfaz::ModoPanel::General) {
        if (ImGui::Button(ICON_FA_ARROW_LEFT " Volver", ImVec2(-1, 28))) {
            self.modo_panel = Interfaz::ModoPanel::General;
        }
        ImGui::Separator();
    }

    switch (self.modo_panel) {
        case Interfaz::ModoPanel::General:     menuGeneral(self, red);   break;
        case Interfaz::ModoPanel::Dijkstra:    subpanelDijkstra(self, red); break;
        case Interfaz::ModoPanel::Kruskal:     subpanelKruskal(self, red);  break;
        case Interfaz::ModoPanel::BFS:         subpanelBFS(self, red);   break;
        case Interfaz::ModoPanel::DFS:         subpanelDFS(self, red);   break;
        case Interfaz::ModoPanel::Ciclos:      subpanelCiclos(self, red); break;
        case Interfaz::ModoPanel::Coloreo:     subpanelColoreo(self, red); break;
        case Interfaz::ModoPanel::Isomorfismo: PanelIsomorfismo::dibujar(self, red); break;
        case Interfaz::ModoPanel::Arbol:       subpanelArbol(self, red); break;
        default: break;
    }

    if (self.anim_estado.activa || self.anim_estado.paso_actual >= 0) {
        ImGui::Separator();
        controlesAnimacion(self);
    }

    propiedadesNodo(self, red);

    ImGui::End();
}

} // namespace PanelGrafos
