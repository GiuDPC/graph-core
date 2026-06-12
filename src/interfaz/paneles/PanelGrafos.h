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
#include "../../nucleo/algoritmos/Isomorfismo.h"
#include "../../nucleo/algoritmos/Planaridad.h"
#include "../../nucleo/algoritmos/AnalizadorGrafo.h"
#include "../../nucleo/algoritmos/TopologiasFractales.h"
#include "../../nucleo/algoritmos/EulerHamilton.h"
#include "interfaz/util/Animacion.h"

class Interfaz;
namespace PanelIsomorfismo {
    inline void dibujar(Interfaz& self, Grafo& red);
}
namespace Matrices {
    inline void dibujar(Grafo& red, class Interfaz& self);
}

// PanelGrafos: namespace que agrupa funciones del panel "Herramientas de Red"
// y sus subpaneles de algoritmos. El subpanel Isomorfismo se delega a PanelIsomorfismo.
namespace PanelGrafos {

// ── Selector de modo (Grafos / Redes) ─────────────────────────────────────
inline void selectorModo(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "MODO DE OPERACION");
    float ancho = ImGui::GetContentRegionAvail().x;
    ImVec2 btnSize(ancho * 0.48f, 32);

    bool en_grafos = (self.estado_ui.modo_actual == Interfaz::ModoApp::Grafos);
    if (en_grafos) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
    if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " GRAFOS", btnSize)) {
        self.estado_ui.modo_actual = Interfaz::ModoApp::Grafos;
        self.registrarLog("Modo cambiado: Grafos");
    }
    if (en_grafos) ImGui::PopStyleColor();

    ImGui::SameLine();

    bool en_redes = (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes);
    if (en_redes) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
    if (ImGui::Button(ICON_FA_NETWORK_WIRED " REDES", btnSize)) {
        self.estado_ui.modo_actual = Interfaz::ModoApp::Redes;
        self.registrarLog("Modo cambiado: Redes");
    }
    if (en_redes) ImGui::PopStyleColor();
}

// ── Controles de animacion (Play / Pausa / Paso / Reset) ──────────────────
inline void controlesAnimacion(Interfaz& self) {
    float ancho = ImGui::GetContentRegionAvail().x;

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
        ICON_FA_FILM " ANIMACION");

    int paso_mostrado = std::min(self.estado_grafos.anim_estado.paso_actual + 1, (int)self.estado_grafos.anim_estado.pasos.size());
    float progreso = 0.0f;
    if (!self.estado_grafos.anim_estado.pasos.empty())
        progreso = (float)paso_mostrado / (float)self.estado_grafos.anim_estado.pasos.size();

    ImGui::ProgressBar(progreso, ImVec2(-1, 18));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Progreso: paso %d de %d",
            paso_mostrado, (int)self.estado_grafos.anim_estado.pasos.size());
    ImGui::Text("Paso %d / %d", paso_mostrado, (int)self.estado_grafos.anim_estado.pasos.size());

    if (self.estado_grafos.anim_estado.paso_actual >= 0 && self.estado_grafos.anim_estado.paso_actual < (int)self.estado_grafos.anim_estado.pasos.size()) {
        const auto& paso = self.estado_grafos.anim_estado.pasos[self.estado_grafos.anim_estado.paso_actual];
        if (!paso.descripcion.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 230, 255, 255));

            // Icono según tipo de paso
            const char* icono_paso = "";
            switch (paso.accion) {
                case PasoAnimacion::VISITAR:   icono_paso = ICON_FA_CIRCLE " "; break;
                case PasoAnimacion::EXPLORAR:  icono_paso = ICON_FA_MAGNIFYING_GLASS " "; break;
                case PasoAnimacion::CONFIRMAR: icono_paso = ICON_FA_CHECK " "; break;
                case PasoAnimacion::DESCARTAR: icono_paso = ICON_FA_XMARK " "; break;
                case PasoAnimacion::COLOREAR:  icono_paso = ICON_FA_PAINTBRUSH " "; break;
            }
            ImGui::TextWrapped("%s%s", icono_paso, paso.descripcion.c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::SliderFloat("Velocidad##anim", &self.estado_grafos.anim_estado.velocidad_paso, 0.05f, 2.0f, "%.2f s/paso");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Tiempo entre cada paso. Mas rapido = menos sonidos para no saturar.");

    float bw = (ancho - 16) / 3.0f;
    if (self.estado_grafos.anim_estado.activa && !self.estado_grafos.anim_estado.pausada) {
            if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 32))) {
            self.estado_grafos.anim_estado.pausada = true;
            g_sonidos.reproducir(Sonidos::CLICK_MENU);
        }
    } else {
        if (ImGui::Button(ICON_FA_PLAY " Play", ImVec2(bw, 32))) {
            if (!self.estado_grafos.anim_estado.activa && self.estado_grafos.anim_estado.paso_actual >= 0) self.estado_grafos.anim_estado.activa = true;
            self.estado_grafos.anim_estado.pausada = false;
            g_sonidos.reproducir(Sonidos::CLICK_MENU);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Reanudar reproduccion automatica de la animacion.");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FORWARD_STEP " Paso", ImVec2(bw, 32))) {
        if (self.estado_grafos.anim_estado.paso_actual + 1 < (int)self.estado_grafos.anim_estado.pasos.size()) {
            self.estado_grafos.anim_estado.paso_actual++;
            AnimacionUI::aplicarPaso(self, self.estado_grafos.anim_estado.pasos[self.estado_grafos.anim_estado.paso_actual]);
            self.estado_grafos.anim_estado.pausada = true;
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Avanza UN paso manualmente (con sonido y efecto).");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reset", ImVec2(bw, 32))) {
        AnimacionUI::reset(self);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Reinicia la animacion desde el principio.");
}

// ── Propiedades del nodo seleccionado + tabla de aristas editables ─────────
inline void propiedadesNodo(Interfaz& self, Grafo& red) {
    if (self.estado_ui.nodo_seleccionado >= 0) {
        Nodo* n = red.obtenerNodo(self.estado_ui.nodo_seleccionado);
        if (n) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), ICON_FA_CIRCLE_INFO " PROPIEDADES");
            ImGui::Text("ID: %d", n->id);

            snprintf(self.estado_ui.buffer_nombre, sizeof(self.estado_ui.buffer_nombre), "%s", n->nombre.c_str());
            if (ImGui::InputText("Nombre", self.estado_ui.buffer_nombre, sizeof(self.estado_ui.buffer_nombre))) {
                n->nombre = self.estado_ui.buffer_nombre;
            }

            if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
                int tipo_int = (int)n->tipo;
                const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
                if (ImGui::Combo("Tipo", &tipo_int, tipos, 5)) {
                    n->tipo = (TipoHardware)tipo_int;
                    self.registrarLog("Tipo cambiado: " + n->nombre + " -> " + tipos[tipo_int]);
                }
                ImGui::Text("Latencia: +%.0f ms", latenciaHardware(n->tipo));
            }

            ImGui::Spacing();
            if (ImGui::Button(ICON_FA_TRASH_CAN " Eliminar Nodo", ImVec2(-1, 32))) {
                self.registrarLog("Nodo eliminado: " + n->nombre);
                red.eliminarNodo(self.estado_ui.nodo_seleccionado);
                self.estado_ui.nodo_seleccionado = -1;
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
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
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
    }
}

// ── Menu general (con tooltips educativos) ─────────────────────────────────
inline void menuGeneral(Interfaz& self, Grafo& red) {
    float ancho = ImGui::GetContentRegionAvail().x;

    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_ROUTE " ENRUTAMIENTO");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.35f, 0.28f, 1.0f));
    if (ImGui::Button(ICON_FA_DIAMOND " Dijkstra — Ruta Optima", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Dijkstra;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Encuentra el camino mas corto entre dos nodos.\n"
            "Su equivalente en redes es OSPF (Open Shortest Path First).\n"
            "Usa una cola de prioridad para explorar siempre el nodo mas cercano.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
        ICON_FA_CIRCLE_NODES " TOPOLOGIA OPTIMA");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
    if (ImGui::Button(ICON_FA_TREE " Kruskal — MST", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Kruskal;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Arbol de Expansion Minima: conecta todos los nodos\n"
            "con el menor costo total posible.\n"
            "En redes = STP (Spanning Tree Protocol) para evitar bucles en switches.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
        ICON_FA_MAGNIFYING_GLASS " RECORRIDO DE GRAFOS");
    float bw = (ancho - 8) / 2.0f;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.18f, 0.38f, 1.0f));
    if (ImGui::Button(ICON_FA_LAYER_GROUP " BFS", ImVec2(bw, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::BFS;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Busqueda en Anchura (Breadth First Search):\n"
            "Explora nivel por nivel. Garantiza el camino con MENOS saltos.\n"
            "Aplicacion: descubrir la red a N saltos de distancia.");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_CODE_BRANCH " DFS", ImVec2(bw, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::DFS;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Busqueda en Profundidad (Depth First Search):\n"
            "Explora yendo tan profundo como puede, luego retrocede.\n"
            "Sirve para detectar ciclos (back-edges) en el grafo.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.1f, 1.0f),
        ICON_FA_CIRCLE_EXCLAMATION " ANALISIS DE GRAFOS");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.15f, 0.05f, 1.0f));
    if (ImGui::Button(ICON_FA_ROTATE " Detectar Ciclos", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Ciclos;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Un ciclo es un camino que empieza y termina en el mismo nodo.\n"
            "Un grafo conexo sin ciclos es un ARBOL.\n"
            "En redes: los ciclos causan 'broadcast storms' que saturan la red.");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.28f, 1.0f));
    if (ImGui::Button(ICON_FA_PAINTBRUSH " Coloreo Greedy", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Coloreo;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Asigna colores a los nodos para que adyacentes tengan colores distintos.\n"
            "Aplicacion real: asignacion de canales WiFi sin interferencias.\n"
            "El numero cromatico minimo es un problema NP-duro (teorema de los 4 colores).");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.20f, 0.25f, 1.0f));
    if (ImGui::Button(ICON_FA_OBJECT_GROUP " Isomorfismo", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Isomorfismo;
        self.resetGrafoIsomorfismo();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Dos grafos son isomorfos si tienen la MISMA estructura\n"
            "aunque los nodos se llamen diferente.\n"
            "Se usa para detectar redes equivalentes a nivel topologico.");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.28f, 0.10f, 1.0f));
    if (ImGui::Button(ICON_FA_SITEMAP " Analisis de Arbol", ImVec2(-1, 32))) {
        self.estado_grafos.modo_panel = Interfaz::ModoPanel::Arbol;
        self.estado_grafos.arbol_analizado = false;
        self.estado_grafos.arbol_layout_aplicado = false;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Analiza si el grafo es un arbol y muestra propiedades:\n"
            "altura, hojas, nivel por nodo, ancestros, primos.\n"
            "Un arbol con n nodos debe tener exactamente n-1 aristas y ser conexo.");
    ImGui::PopStyleColor();

    if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            ICON_FA_WAVE_SQUARE " SIMULACION");
        ImGui::Checkbox("Activar Jitter", &self.estado_redes.simulacion_jitter);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Simula inestabilidad en la red: variacion aleatoria de latencia.\n"
                "Comun en redes WiFi y enlaces satelitales.");
        if (self.estado_redes.simulacion_jitter) {
            ImGui::SliderFloat("Intensidad##jitter", &self.estado_redes.jitter_porcentaje, 0.01f, 0.50f, "%.0f%%");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Porcentaje de variacion sobre el peso original de cada arista.");
        }
    }
    
    // ── Estadisticas rapidas del grafo actual ───────────────────────────────
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled(ICON_FA_CHART_BAR " Resumen del grafo");
    if (!red.nodos.empty()) {
        int aristas = (int)red.aristas.size();
        int n_nodos = (int)red.nodos.size();
        float densidad = (n_nodos > 1) ? (2.0f * aristas) / (n_nodos * (n_nodos - 1)) : 0;
        ImGui::Text("%s %d nodos  |  %s %d aristas  |  Densidad: %.1f%%",
            ICON_FA_CIRCLE, n_nodos, ICON_FA_LINK, aristas, densidad * 100.0f);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Densidad = aristas reales / aristas posibles.\n"
                "0%% = grafo vacio, 100%% = grafo completo (todos conectados con todos).");
    } else {
        ImGui::TextDisabled(ICON_FA_CIRCLE_INFO " Haz clic derecho en el lienzo para crear nodos");
    }
}

// ── Subpanel: Dijkstra (con tooltips educativos) ──────────────────────────
inline void subpanelDijkstra(Interfaz& self, Grafo& red) {
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_DIAMOND " DIJKSTRA — RUTA OPTIMA");
    ImGui::TextWrapped(
        "Encuentra la ruta de menor costo entre dos nodos. "
        "Usa una cola de prioridad para explorar siempre el nodo mas cercano.");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("🔍 Edsger Dijkstra (1956): '¿Cual es el camino mas corto entre dos ciudades?'\n"
            "📡 En redes: OSPF (Open Shortest Path First) usa exactamente este algoritmo.\n"
            "🧮 Complejidad: O((V+E) log V) con cola de prioridad.");
    ImGui::Spacing();

    ImGui::Text("Origen:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##origen_d", red.nombreNodo(self.estado_grafos.dijkstra_origen).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.estado_grafos.dijkstra_origen);
            if (ImGui::Selectable(n.nombre.c_str(), sel))
                self.estado_grafos.dijkstra_origen = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Text("Destino:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##destino_d", red.nombreNodo(self.estado_grafos.dijkstra_destino).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.estado_grafos.dijkstra_destino);
            if (ImGui::Selectable(n.nombre.c_str(), sel))
                self.estado_grafos.dijkstra_destino = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
        ImGui::Checkbox("Incluir latencia de hardware", &self.estado_grafos.dijkstra_usar_latencia);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Suma la latencia de cada tipo de equipo al costo de la ruta.\n"
                "Servidor=1ms, Router=3ms, Switch=5ms, Firewall=10ms, Terminal=2ms.");
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::generarPasos(
            red, self.estado_grafos.dijkstra_origen, self.estado_grafos.dijkstra_destino, self.estado_grafos.dijkstra_usar_latencia);
        AnimacionUI::iniciar(self, pasos);
        self.estado_grafos.ruta_optima.clear(); self.estado_grafos.mostrar_mst = false;
        self.registrarLog("[Dijkstra] Animacion iniciada: " +
            red.nombreNodo(self.estado_grafos.dijkstra_origen) + " -> " + red.nombreNodo(self.estado_grafos.dijkstra_destino));
        g_sonidos.reproducir(Sonidos::VISITAR_NODO);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Muestra cada paso del algoritmo con particulas animadas y sonido.");
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        auto res = Algoritmos::dijkstra(
            red, self.estado_grafos.dijkstra_origen, self.estado_grafos.dijkstra_destino, self.estado_grafos.dijkstra_usar_latencia);
        self.estado_grafos.ruta_optima = res.ruta;
        self.estado_grafos.dijkstra_costo_total = res.costo_total;
        self.estado_grafos.mostrar_mst = false; AnimacionUI::reset(self);
        if (res.hay_ruta) {
            self.registrarLog("[OK] Dijkstra: " + std::to_string(res.saltos) +
                " saltos, costo=" + std::to_string((int)res.costo_total));
            g_sonidos.reproducir(Sonidos::TRIUNFO_DIJKSTRA);
        } else {
            self.registrarLog("[!] Dijkstra: no existe ruta entre esos nodos");
            g_sonidos.reproducir(Sonidos::DESCARTAR);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Calcula la ruta optima al instante (sin animacion).");

    ImGui::Separator();
    if (!self.estado_grafos.ruta_optima.empty()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
            ICON_FA_CHECK " Ruta encontrada:");
        std::string ruta_str;
        for (size_t i = 0; i < self.estado_grafos.ruta_optima.size(); i++) {
            if (i > 0) ruta_str += " → ";
            ruta_str += red.nombreNodo(self.estado_grafos.ruta_optima[i]);
        }
        ImGui::TextWrapped("%s", ruta_str.c_str());
        ImGui::Text("Saltos: %d | Costo total: %.1f",
            (int)self.estado_grafos.ruta_optima.size() - 1,
            self.estado_grafos.dijkstra_costo_total);
    } else if (!self.estado_grafos.anim_estado.activa && self.estado_grafos.anim_estado.paso_actual < 0) {
        ImGui::TextDisabled(ICON_FA_CIRCLE_INFO " Selecciona origen/destino y ejecuta.");
    }

    if (self.estado_grafos.anim_estado.activa && !self.estado_grafos.dijkstra_tabla_dist.empty()) {
        ImGui::Separator();
        ImGui::Text("Tabla de distancias:");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Muestra la distancia desde el origen a cada nodo.\n"
                "INF = nodo aun no alcanzable.");
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
                float d = (n.id < (int)self.estado_grafos.dijkstra_tabla_dist.size())
                    ? self.estado_grafos.dijkstra_tabla_dist[n.id]
                    : std::numeric_limits<float>::infinity();
                if (d == std::numeric_limits<float>::infinity())
                    ImGui::TextDisabled("∞");
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
        ICON_FA_TREE " KRUSKAL — ARBOL DE EXPANSION MINIMA");
    ImGui::TextWrapped(
        "Encuentra la red que conecta todos los nodos con el menor costo total posible.");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("🔍 Joseph Kruskal (1956): ordena aristas por peso y las agrega si no forman ciclo.\n"
            "📡 En redes: STP (Spanning Tree Protocol) evita bucles en switches.\n"
            "🧮 Usa Union-Find (DSU) para detectar ciclos en tiempo casi constante.\n"
            "⚡ Complejidad: O(E log E) por ordenar aristas.");
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::Kruskal::generarPasos(red);
        AnimacionUI::iniciar(self, pasos);
        self.estado_grafos.ruta_optima.clear(); self.estado_grafos.mostrar_mst = false;
        self.registrarLog("[Kruskal] Animacion MST iniciada");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Visualiza paso a paso como se construye el arbol de expansion minima.");
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        auto res = Algoritmos::Kruskal::kruskal(red);
        self.estado_grafos.aristas_mst = res.aristas_mst;
        self.estado_grafos.mostrar_mst = true;
        self.estado_grafos.ruta_optima.clear(); AnimacionUI::reset(self);
        self.registrarLog("[OK] Kruskal MST: " + std::to_string(res.aristas_aceptadas) +
            " aristas, peso total=" + std::to_string((int)res.peso_total));
        g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Calcula el arbol de expansion minima al instante.");

    if (self.estado_grafos.mostrar_mst && !self.estado_grafos.aristas_mst.empty()) {
        ImGui::Separator();

        float peso_total_grafo = 0;
        for (const auto& a : red.aristas) peso_total_grafo += a.peso;

        float peso_mst = 0;
        for (const auto& a : self.estado_grafos.aristas_mst) peso_mst += a.peso;

        ImGui::Text("Aristas en MST:  %d / %d", (int)self.estado_grafos.aristas_mst.size(), (int)red.aristas.size());
        ImGui::Text("Peso MST:        %.1f", peso_mst);
        ImGui::Text("Peso total red:  %.1f", peso_total_grafo);

        float ahorro = peso_total_grafo - peso_mst;
        if (ahorro > 0) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                "Ahorro vs. red completa: %.1f (%.0f%%)", ahorro,
                (peso_total_grafo > 0) ? (ahorro / peso_total_grafo * 100.0f) : 0);
        }

        ImGui::Separator();
        ImGui::Text("Aristas del MST:");
        for (const auto& a : self.estado_grafos.aristas_mst) {
            ImGui::BulletText("%s — %s (%.1f)",
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
        if (!red.obtenerNodo(self.estado_grafos.bfs_nodo_inicio) && !red.nodos.empty())
            self.estado_grafos.bfs_nodo_inicio = red.nodos[0].id;

        if (ImGui::BeginCombo("##inicio_bfs", red.nombreNodo(self.estado_grafos.bfs_nodo_inicio).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == self.estado_grafos.bfs_nodo_inicio);
                if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                    self.estado_grafos.bfs_nodo_inicio = n.id;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_PLAY " Animacion BFS", ImVec2(-1, 32))) {
            auto pasos = Algoritmos::BFS::generarPasos(red, self.estado_grafos.bfs_nodo_inicio);
            AnimacionUI::iniciar(self, pasos);
            self.estado_grafos.bfs_resultado = Algoritmos::BFS::bfs(red, self.estado_grafos.bfs_nodo_inicio);
            self.registrarLog("[BFS] Iniciado desde " + red.nombreNodo(self.estado_grafos.bfs_nodo_inicio));
            g_sonidos.reproducir(Sonidos::VISITAR_NODO);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Recorre el grafo por niveles, visitando todos los vecinos antes de avanzar.");
        if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
            self.estado_grafos.bfs_resultado = Algoritmos::BFS::bfs(red, self.estado_grafos.bfs_nodo_inicio);
            AnimacionUI::reset(self);
            self.registrarLog("[OK] BFS: " + std::to_string(self.estado_grafos.bfs_resultado.orden_visita.size()) +
                " nodos visitados");
            g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Ejecuta la busqueda en anchura y muestra el resultado.");

        if (!self.estado_grafos.bfs_resultado.nivel.empty()) {
            ImGui::Separator();
            ImGui::Text("Distancias desde %s:", red.nombreNodo(self.estado_grafos.bfs_nodo_inicio).c_str());
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
                    auto it = self.estado_grafos.bfs_resultado.nivel.find(n.id);
                    if (it != self.estado_grafos.bfs_resultado.nivel.end())
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

    if (!red.obtenerNodo(self.estado_grafos.dfs_nodo_inicio) && !red.nodos.empty())
        self.estado_grafos.dfs_nodo_inicio = red.nodos[0].id;

    ImGui::Text("Nodo de inicio:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##inicio_dfs", red.nombreNodo(self.estado_grafos.dfs_nodo_inicio).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.estado_grafos.dfs_nodo_inicio);
            if (ImGui::Selectable(n.nombre.c_str(), sel)) self.estado_grafos.dfs_nodo_inicio = n.id;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_PLAY " Animacion DFS", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::DFS::generarPasos(red, self.estado_grafos.dfs_nodo_inicio);
        AnimacionUI::iniciar(self, pasos);
        self.registrarLog("[DFS] Iniciado desde " + red.nombreNodo(self.estado_grafos.dfs_nodo_inicio));
        g_sonidos.reproducir(Sonidos::VISITAR_NODO);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Explora el grafo en profundidad, retrocediendo solo al llegar a un callejon.");
    if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
        self.estado_grafos.dfs_resultado = Algoritmos::DFS::dfs(red, self.estado_grafos.dfs_nodo_inicio);
        AnimacionUI::reset(self);
        self.registrarLog("[OK] DFS: " + std::to_string(self.estado_grafos.dfs_resultado.orden_visita.size()) +
            " nodos, " + std::to_string(self.estado_grafos.dfs_resultado.back_edges.size()) + " back-edges");
        g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Ejecuta la busqueda en profundidad y muestra el resultado.");

    if (!self.estado_grafos.dfs_resultado.orden_visita.empty()) {
        ImGui::Separator();
        ImGui::Text("Orden de visita:");
        std::string orden;
        for (size_t i = 0; i < self.estado_grafos.dfs_resultado.orden_visita.size(); i++) {
            if (i > 0) orden += " -> ";
            orden += red.nombreNodo(self.estado_grafos.dfs_resultado.orden_visita[i]);
        }
        ImGui::TextWrapped("%s", orden.c_str());

        if (!self.estado_grafos.dfs_resultado.back_edges.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.2f, 1.0f),
                ICON_FA_TRIANGLE_EXCLAMATION " Back-edges (ciclos):");
            for (const auto& [u, v] : self.estado_grafos.dfs_resultado.back_edges) {
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

    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar", ImVec2(-1, 32))) {
        self.estado_grafos.resultado_ciclos = Algoritmos::detectarCiclos(red);
        self.estado_grafos.ciclo_analizado = true;
        if (self.estado_grafos.resultado_ciclos.tiene_ciclo) {
            self.registrarLog("[!] Ciclo detectado: " + self.estado_grafos.resultado_ciclos.descripcion);
            g_sonidos.reproducir(Sonidos::DESCARTAR);
        } else {
            self.registrarLog("[OK] Grafo aciclico: " + self.estado_grafos.resultado_ciclos.descripcion);
            g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Determina si el grafo contiene ciclos usando el algoritmo de Union-Find.");

    if (self.estado_grafos.ciclo_analizado) {
        ImGui::Separator();
        if (self.estado_grafos.resultado_ciclos.tiene_ciclo) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                ICON_FA_TRIANGLE_EXCLAMATION " CICLO DETECTADO");
            ImGui::TextWrapped("%s", self.estado_grafos.resultado_ciclos.descripcion.c_str());
            ImGui::Spacing();
            ImGui::Text("Aristas que forman el ciclo:");
            for (const auto& [u, v] : self.estado_grafos.resultado_ciclos.aristas_ciclo) {
                ImGui::BulletText("%s - %s",
                    red.nombreNodo(u).c_str(), red.nombreNodo(v).c_str());
            }
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " Grafo aciclico");
            ImGui::TextWrapped("%s", self.estado_grafos.resultado_ciclos.descripcion.c_str());
        }
    }
}

// ── Subpanel: Coloreo (OVERHAUL) ──────────────────────────────────────────
inline void subpanelColoreo(Interfaz& self, Grafo& red) {
    // ── Cabecera educativa ─────────────────────────────────────────────────
    ImGui::TextColored(ImVec4(0.7f, 0.1f, 0.7f, 1.0f),
        ICON_FA_PAINTBRUSH " COLOREO — TEORIA DE GRAFOS");
    ImGui::TextWrapped(
        "Asigna colores a los nodos para que adyacentes tengan colores distintos. "
        "El minimo de colores necesarios es el NUMERO CROMATICO χ(G). "
        "Problema NP-duro — el algoritmo greedy da una cota superior.");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("🎨 Teorema de los 4 colores (Appel & Haken, 1976):\n"
            "TODO mapa/grafo planar puede colorearse con ≤4 colores.\n"
            "Fue el PRIMER teorema demostrado por computadora.\n"
            "📡 En redes: asignacion de canales WiFi sin interferencias.\n"
            "🧮 Numero cromatico = problema NP-duro.");
    ImGui::Spacing();

    // ── Boton principal toggle ────────────────────────────────────────────
    if (ImGui::Button(self.estado_grafos.mostrar_coloreo
        ? ICON_FA_EYE_SLASH " Ocultar Color"
        : ICON_FA_PAINT_ROLLER " Aplicar Color", ImVec2(-1, 32)))
    {
        self.estado_grafos.mostrar_coloreo = !self.estado_grafos.mostrar_coloreo;
        if (self.estado_grafos.mostrar_coloreo) {
            self.estado_grafos.resultado_coloreo = Algoritmos::coloreoGreedy(red);
            self.estado_grafos.resultado_welsh_powell = Algoritmos::coloreoWelshPowell(red);
            self.estado_grafos.colores_nodos = self.estado_grafos.resultado_coloreo.colores;
            self.estado_grafos.modo_fractal = false;
            self.estado_grafos.planar_analizado = false;
            self.registrarLog("[OK] Coloreo greedy: " + std::to_string(self.estado_grafos.resultado_coloreo.num_colores) +
                " colores usados");
            g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Aplica un coloreo greedy a los nodos o lo oculta.");

    if (!self.estado_grafos.mostrar_coloreo) return;

    ImGui::Separator();

    // ── Info header ────────────────────────────────────────────────────────
    auto& col = self.estado_grafos.resultado_coloreo;
    auto& wp  = self.estado_grafos.resultado_welsh_powell;
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        "χ(greedy)=%d  |  χ(welsh-powell)=%d  |  Nodos=%d",
        col.num_colores, wp.num_colores, (int)red.nodos.size());

    // ── Planaridad ────────────────────────────────────────────────────────
    if (!self.estado_grafos.planar_analizado) {
        self.estado_grafos.resultado_planaridad = Algoritmos::Planaridad::analizar(red);
        self.estado_grafos.planar_analizado = true;
    }
    const auto& plan = self.estado_grafos.resultado_planaridad;

    if (plan.es_planar) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
            ICON_FA_CHECK " GRAFO PLANAR — 4-coloreable por teorema");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("El teorema de los 4 colores (Appel & Haken, 1976)\n"
                "garantiza que este grafo necesita ≤4 colores.\n"
                "χ(greedy)=%d es una cota superior.", col.num_colores);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            ICON_FA_TRIANGLE_EXCLAMATION " NO PLANAR");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", plan.descripcion.c_str());
        if (plan.sospecha_k5)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "  ⚠ Posible K5 (5 nodos de alto grado)");
        if (plan.sospecha_k33)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "  ⚠ Posible K3,3 (6+ nodos grado >=3)");
    }

    // ── Cruces detectados ─────────────────────────────────────────────────
    if (plan.cruces_estimadas > 0 && self.estado_grafos.mostrar_cruces) {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f),
            "✕ %d cruces de aristas detectados", plan.cruces_estimadas);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Las aristas que se cruzan en el dibujo actual.\n"
                "Prueba a mover nodos o usar 'Distribuir' para reducirlos.");
    }

    ImGui::Spacing();

    // ── Modos de visualizacion ────────────────────────────────────────────
    ImGui::TextDisabled("Modo de coloreo:");

    bool modo_greedy = !self.estado_grafos.modo_fractal;

    if (ImGui::RadioButton(ICON_FA_PALETTE " clasico (greedy)", modo_greedy)) {
        if (!modo_greedy) {
            self.estado_grafos.modo_fractal = false;
            self.estado_grafos.colores_nodos = col.colores;
        }
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(ICON_FA_FAN " matematico (mandelbrot)", self.estado_grafos.modo_fractal)) {
        if (!self.estado_grafos.modo_fractal) {
            self.estado_grafos.modo_fractal = true;
            self.estado_grafos.fractal_tiempo = 0.0f;
            self.estado_grafos.resultado_fractal = Algoritmos::ColorFractal::calcular(
                red, col.colores, 0.0f);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLAY " Animar", ImVec2(-1, 32))) {
        auto pasos = Algoritmos::generarPasosColoreo(red);
        AnimacionUI::iniciar(self, pasos);
        self.estado_grafos.mostrar_coloreo = true;
        self.estado_grafos.modo_fractal = false;
        
        // Calcular resultado y preparar array de colores vacio
        auto res = Algoritmos::coloreoGreedy(red);
        self.estado_grafos.resultado_coloreo = res;
        self.estado_grafos.colores_nodos.assign(red.rangoIds(), -1);
        
        self.registrarLog("[OK] Animacion de coloreo iniciada: " +
            std::to_string(pasos.size()) + " pasos");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Muestra como se asignan los colores nodo por nodo.");

    // ── Modo fractal: actualizar morphing ──────────────────────────────────
    if (self.estado_grafos.modo_fractal) {
        self.estado_grafos.fractal_tiempo += ImGui::GetIO().DeltaTime;
        self.estado_grafos.resultado_fractal = Algoritmos::ColorFractal::calcular(
            red, col.colores, self.estado_grafos.fractal_tiempo);

        // Convertir a colores_fractales fusionando fractal + base
        auto& frac = self.estado_grafos.resultado_fractal;
        self.estado_grafos.colores_fractales.assign(red.rangoIds(), 0);
        for (size_t i = 0; i < red.nodos.size(); i++) {
            int nid = red.nodos[i].id;
            if (nid < (int)col.colores.size() && col.colores[nid] >= 0) {
                float mod = (i < frac.modulacion_fractal.size()) ? frac.modulacion_fractal[i] : 0.5f;
                float fase = (i < frac.morph_fase.size()) ? frac.morph_fase[i] : 0.0f;
                self.estado_grafos.colores_fractales[nid] = Algoritmos::ColorFractal::colorFusionado(
                    col.colores[nid], mod, fase, col.num_colores);
            }
        }

        if (ImGui::CollapsingHeader(ICON_FA_FAN " Coloreo Fractal")) {
            ImGui::TextWrapped(
                "Cada nodo hereda el color base del greedy, pero su matiz, "
                "saturacion y brillo se modulan con un fractal de Mandelbrot "
                "adaptado a la posicion del nodo en el canvas.");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Los parametros del fractal rotan lentamente -> colores 'respiran'.\n"
                    "Las zonas mas profundas del fractal son mas saturadas y brillantes.\n"
                    "Los colores base (adyacentes distintos) se preservan.");
            ImGui::TextDisabled("Morphing activo — los colores cambian suavemente.");
        }
    }

    // ── Botones de accion ─────────────────────────────────────────────────
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_ARROWS_SPIN " Re-analizar planaridad", ImVec2(-1, 32))) {
        self.estado_grafos.planar_analizado = false;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Vuelve a evaluar si el grafo es planar y detecta cruces de aristas.");

    if (ImGui::Button(ICON_FA_EYE_SLASH " Ocultar cruces", ImVec2(-1, 32))) {
        self.estado_grafos.mostrar_cruces = !self.estado_grafos.mostrar_cruces;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Alterna la visualizacion de los cruces de aristas detectados.");

    // ── Tabla comparativa ─────────────────────────────────────────────────
    if (ImGui::CollapsingHeader(ICON_FA_TABLE " Comparacion de algoritmos")) {
        if (ImGui::BeginTable("tabColores", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Algoritmo", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Colores", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Optimo?", ImGuiTableColumnFlags_WidthFixed, 55);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Greedy (orden entrada)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", col.num_colores);
            ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("~");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Welsh-Powell (grado desc.)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", wp.num_colores);
            ImGui::TableSetColumnIndex(2);
            if (wp.num_colores <= col.num_colores)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_CHECK);
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), ICON_FA_XMARK);

            ImGui::EndTable();
        }
        ImGui::TextDisabled("Nota: el numero cromatico optimo puede ser menor o igual.");
        if (plan.es_planar) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " Por teorema 4-colores: χ ≤ 4");
        }
    }

    // ── Asignacion detallada ──────────────────────────────────────────────
    if (ImGui::CollapsingHeader(ICON_FA_LIST " Asignacion por nodo")) {
        const char* nombres_colores[] = {
            "Rojo", "Verde", "Azul", "Amarillo", "Magenta", "Cyan"
        };
        for (const auto& n : red.nodos) {
            if (n.id < (int)self.estado_grafos.colores_nodos.size() && self.estado_grafos.colores_nodos[n.id] >= 0) {
                int c = self.estado_grafos.colores_nodos[n.id];
                // Si es color fractal (negativo), no mostrar detalle
                if (c < 0) {
                    ImGui::BulletText("%s → Color fractal", n.nombre.c_str());
                    continue;
                }
                ImGui::BulletText("%s → Color %d (%s)",
                    n.nombre.c_str(), c,
                    c < 6 ? nombres_colores[c] : "Extra");
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
    if (!red.nodos.empty() && !red.obtenerNodo(self.estado_grafos.arbol_raiz_id))
        self.estado_grafos.arbol_raiz_id = red.nodos[0].id;
    if (ImGui::BeginCombo("##raiz_arbol", red.nombreNodo(self.estado_grafos.arbol_raiz_id).c_str())) {
        for (const auto& n : red.nodos) {
            bool sel = (n.id == self.estado_grafos.arbol_raiz_id);
            if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                self.estado_grafos.arbol_raiz_id = n.id;
                self.estado_grafos.arbol_analizado = false;
                self.estado_grafos.arbol_layout_aplicado = false;
            }
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar Arbol", ImVec2(-1, 32))) {
        self.estado_grafos.arbol_props = Algoritmos::Arbol::analizar(red, self.estado_grafos.arbol_raiz_id);
        self.estado_grafos.arbol_analizado = true;
        self.estado_grafos.arbol_layout_aplicado = false;
        self.registrarLog("[OK] Arbol analizado. Altura=" + std::to_string(self.estado_grafos.arbol_props.altura) +
            ", Grado=" + std::to_string(self.estado_grafos.arbol_props.grado_arbol));
        g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Verifica si el grafo es un arbol y muestra su estructura jerarquica.");

    if (self.estado_grafos.arbol_analizado && !self.estado_grafos.arbol_layout_aplicado) {
        if (ImGui::Button(ICON_FA_SITEMAP " Aplicar Layout Jerarquico", ImVec2(-1, 32))) {
            ImVec2 centro(800.0f, 400.0f);
            aplicarLayoutArbol(red, self.estado_grafos.arbol_props, centro);
            self.estado_grafos.arbol_layout_aplicado = true;
            self.registrarLog("[OK] Layout jerarquico aplicado");
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reorganiza los nodos en un arbol visual segun la jerarquia calculada.");
    }

    if (self.estado_grafos.arbol_analizado) {
        ImGui::Separator();

        ImGui::Text("Altura:              %d", self.estado_grafos.arbol_props.altura);
        ImGui::Text("Grado del arbol:     %d", self.estado_grafos.arbol_props.grado_arbol);
        ImGui::Text("Numero de hojas:     %d", (int)self.estado_grafos.arbol_props.hojas.size());
        ImGui::Spacing();

        if (!self.estado_grafos.arbol_props.rama_mas_larga.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Rama mas larga:");
            std::string rml;
            for (size_t i = 0; i < self.estado_grafos.arbol_props.rama_mas_larga.size(); i++) {
                if (i > 0) rml += " -> ";
                rml += red.nombreNodo(self.estado_grafos.arbol_props.rama_mas_larga[i]);
            }
            ImGui::TextWrapped("%s", rml.c_str());
        }
        if (!self.estado_grafos.arbol_props.rama_mas_corta.empty()) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Rama mas corta:");
            std::string rmc;
            for (size_t i = 0; i < self.estado_grafos.arbol_props.rama_mas_corta.size(); i++) {
                if (i > 0) rmc += " -> ";
                rmc += red.nombreNodo(self.estado_grafos.arbol_props.rama_mas_corta[i]);
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
                ImGui::Text("%d", self.estado_grafos.arbol_props.nivel.count(n.id) ? self.estado_grafos.arbol_props.nivel.at(n.id) : -1);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", self.estado_grafos.arbol_props.hijos.count(n.id) ? (int)self.estado_grafos.arbol_props.hijos.at(n.id).size() : 0);
                ImGui::TableSetColumnIndex(3);
                if (n.id == self.estado_grafos.arbol_props.raiz_id) {
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Raiz");
                } else if (std::find(self.estado_grafos.arbol_props.hojas.begin(), self.estado_grafos.arbol_props.hojas.end(), n.id) != self.estado_grafos.arbol_props.hojas.end()) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Hoja");
                } else {
                    ImGui::TextDisabled("Interno");
                }
            }
            ImGui::EndTable();
        }

        if (self.estado_ui.nodo_seleccionado >= 0 && self.estado_grafos.arbol_props.nivel.count(self.estado_ui.nodo_seleccionado)) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.8f, 1.0f),
                "Detalles de %s:", red.nombreNodo(self.estado_ui.nodo_seleccionado).c_str());

            int nid = self.estado_ui.nodo_seleccionado;
            int pid = self.estado_grafos.arbol_props.padre.count(nid) ? self.estado_grafos.arbol_props.padre.at(nid) : -1;
            ImGui::Text("Padre:       %s", pid >= 0 ? red.nombreNodo(pid).c_str() : "(raiz)");

            if (self.estado_grafos.arbol_props.hijos.count(nid)) {
                const auto& hijos = self.estado_grafos.arbol_props.hijos.at(nid);
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
            } else {
                ImGui::Text("Hijos:       (sin informacion)");
            }

            if (pid >= 0 && self.estado_grafos.arbol_props.hermanos.count(pid)) {
                const auto& herm = self.estado_grafos.arbol_props.hermanos.at(pid);
                std::string sh;
                for (int h : herm) {
                    if (h == nid) continue;
                    if (!sh.empty()) sh += ", ";
                    sh += red.nombreNodo(h);
                }
                ImGui::Text("Hermanos:    %s", sh.empty() ? "(ninguno)" : sh.c_str());
            }

            if (self.estado_grafos.arbol_props.ancestros.count(nid)) {
                const auto& anc = self.estado_grafos.arbol_props.ancestros.at(nid);
                std::string sa;
                for (size_t i = 0; i < anc.size(); i++) {
                    if (i > 0) sa += " -> ";
                    sa += red.nombreNodo(anc[i]);
                }
                ImGui::Text("Ancestros:   %s", sa.empty() ? "(raiz)" : sa.c_str());
            }

            if (self.estado_grafos.arbol_props.descendientes.count(nid)) {
                ImGui::Text("Descendientes: %d nodos",
                    (int)self.estado_grafos.arbol_props.descendientes.at(nid).size());
            }

            std::vector<std::string> mis_primos;
            for (const auto& [a, b] : self.estado_grafos.arbol_props.primos) {
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

        if (!self.estado_grafos.arbol_props.primos.empty()) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Pares de primos")) {
                for (const auto& [a, b] : self.estado_grafos.arbol_props.primos) {
                    ImGui::BulletText("%s y %s (nivel %d)",
                        red.nombreNodo(a).c_str(),
                        red.nombreNodo(b).c_str(),
                        self.estado_grafos.arbol_props.nivel.at(a));
                }
            }
        }
    }
}

// -- barra lateral izquierda (delgada, informacion rapida) --
inline void sidebarInfo(Interfaz& self, Grafo& red) {
    ImGui::Begin("Info del Grafo");
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "MODO");
    const char* modo_txt = (self.estado_ui.modo_actual == Interfaz::ModoApp::Grafos)
        ? ICON_FA_DIAGRAM_PROJECT " Grafos" : ICON_FA_NETWORK_WIRED " Redes";
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f), "%s", modo_txt);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "GRAFO");
    int aristas = (int)red.aristas.size();
    int n_nodos = (int)red.nodos.size();
    float densidad = (n_nodos > 1) ? (2.0f * aristas) / (n_nodos * (n_nodos - 1)) : 0;
    ImGui::Text("%s %d nodos", ICON_FA_CIRCLE, n_nodos);
    ImGui::Text("%s %d aristas", ICON_FA_LINK, aristas);
    ImGui::TextDisabled("densidad %.1f%%", densidad * 100.0f);

    // Deteccion automatica de propiedades
    auto props = Algoritmos::AnalizadorGrafo::analizar(red);
    ImGui::Spacing();
    
    // Mostramos Badges si cumple ciertas propiedades
    if (n_nodos > 0) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
        
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        ImGuiStyle& style = ImGui::GetStyle();
        bool is_first = true;

        // Función auxiliar para dibujar un badge coloreado con wrap
        auto drawBadge = [&window_visible_x2, &style, &is_first](const char* text, ImVec4 color) {
            if (!is_first) {
                float last_button_x2 = ImGui::GetItemRectMax().x;
                float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ImGui::CalcTextSize(text).x + style.FramePadding.x * 2.0f;
                if (next_button_x2 < window_visible_x2)
                    ImGui::SameLine();
            }
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::Button(text);
            ImGui::PopStyleColor(4);
            is_first = false;
        };

        if (props.es_arbol) drawBadge("Arbol", ImVec4(0.1f, 0.6f, 0.3f, 1.0f));
        else if (props.es_conexo) drawBadge("Conexo", ImVec4(0.0f, 0.5f, 0.8f, 1.0f));
        else drawBadge("Desconexo", ImVec4(0.6f, 0.2f, 0.2f, 1.0f));

        if (props.es_bipartito) drawBadge("Bipartito", ImVec4(0.6f, 0.3f, 0.7f, 1.0f));
        if (props.es_completo) drawBadge("Completo", ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        if (props.es_euleriano) drawBadge("Euleriano", ImVec4(0.2f, 0.6f, 0.6f, 1.0f));
        
        if (props.es_regular && props.grado_regular >= 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d-Regular", props.grado_regular);
            drawBadge(buf, ImVec4(0.5f, 0.5f, 0.1f, 1.0f));
        }
        
        ImGui::PopStyleVar();
    }

    if (self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "ANIMACION");
        controlesAnimacion(self);
    }

    ImGui::End();
}

// -- panel contextual derecho: muestra config de herramienta activa --
inline void panelContextual(Interfaz& self, Grafo& red) {
    // en modo redes, el panel de red maneja sus propias ventanas
    if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
        return;
    }

    ImGui::Begin("Algoritmos");
    
    // Dropdown para seleccionar herramienta
    const char* items[] = {
        ICON_FA_WRENCH " General",
        ICON_FA_ROUTE " Rutas",
        ICON_FA_TREE " Arbol",
        ICON_FA_MAGNIFYING_GLASS " Busqueda",
        ICON_FA_ROTATE " Ciclos",
        ICON_FA_PAINTBRUSH " Coloreo",
        ICON_FA_OBJECT_GROUP " Isomorfismo",
        ICON_FA_SHAPES " Fractales",
        ICON_FA_ARROW_RIGHT_ARROW_LEFT " Euler / Hamilton"
    };
    
    // Mapeo inverso
    int current_item = 0;
    if (self.estado_ui.herramienta_activa == EstadoUI::CatRutas) current_item = 1;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatArbol) current_item = 2;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatBusqueda) current_item = 3;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatCiclos) current_item = 4;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatColoreo) current_item = 5;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatIsomorfismo) current_item = 6;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatFractales) current_item = 7;
    else if (self.estado_ui.herramienta_activa == EstadoUI::CatEulerHamilton) current_item = 8;
    
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##algo_select", &current_item, items, 9)) {
        if (current_item == 0) self.estado_ui.herramienta_activa = EstadoUI::CatGeneral;
        else if (current_item == 1) self.estado_ui.herramienta_activa = EstadoUI::CatRutas;
        else if (current_item == 2) self.estado_ui.herramienta_activa = EstadoUI::CatArbol;
        else if (current_item == 3) self.estado_ui.herramienta_activa = EstadoUI::CatBusqueda;
        else if (current_item == 4) self.estado_ui.herramienta_activa = EstadoUI::CatCiclos;
        else if (current_item == 5) self.estado_ui.herramienta_activa = EstadoUI::CatColoreo;
        else if (current_item == 6) self.estado_ui.herramienta_activa = EstadoUI::CatIsomorfismo;
        else if (current_item == 7) self.estado_ui.herramienta_activa = EstadoUI::CatFractales;
        else if (current_item == 8) self.estado_ui.herramienta_activa = EstadoUI::CatEulerHamilton;
    }
    ImGui::Separator();

    switch (self.estado_ui.herramienta_activa) {
        case EstadoUI::CatGeneral:
            ImGui::TextWrapped("Selecciona una herramienta desde el menu desplegable de arriba.");
            if (red.nodos.empty()) {
                ImGui::Spacing();
                ImGui::TextDisabled(ICON_FA_CIRCLE_INFO " clic derecho en el lienzo para crear nodos.");
            }
            break;
        case EstadoUI::CatRutas:    subpanelDijkstra(self, red); break;
        case EstadoUI::CatArbol:
            subpanelKruskal(self, red);
            ImGui::Separator();
            subpanelArbol(self, red);
            break;
        case EstadoUI::CatBusqueda:
            subpanelBFS(self, red);
            ImGui::Separator();
            subpanelDFS(self, red);
            break;
        case EstadoUI::CatCiclos:   subpanelCiclos(self, red); break;
        case EstadoUI::CatColoreo:  subpanelColoreo(self, red); break;
        case EstadoUI::CatIsomorfismo: PanelIsomorfismo::dibujar(self, red); break;
        case EstadoUI::CatFractales: {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), ICON_FA_SHAPES " generadores fractales");
            ImGui::TextWrapped("genera grafos matematicos usando patrones recursivos.");
            ImGui::Spacing();
            static int iteraciones = 3;
            ImGui::SliderInt("iteraciones", &iteraciones, 1, 6);
            ImGui::Spacing();

            auto reset_fractal = [&]() {
                self.estado_grafos.ruta_optima.clear();
                self.estado_grafos.aristas_mst.clear();
                self.estado_grafos.mostrar_mst = false;
                AnimacionUI::reset(self);
            };

            if (ImGui::Button("triangulo de sierpinski", ImVec2(-1, 32))) {
                self.registrarLog("generando fractal sierpinski iteracion " + std::to_string(iteraciones));
                red = Algoritmos::TopologiasFractales::generarSierpinski(iteraciones);
                reset_fractal();
            }
            if (ImGui::Button("mandala geometrico", ImVec2(-1, 32))) {
                self.registrarLog("generando fractal mandala");
                red = Algoritmos::TopologiasFractales::generarMandala(iteraciones + 1, 10 + iteraciones * 2);
                reset_fractal();
            }
            if (ImGui::Button("arbol fractal", ImVec2(-1, 32))) {
                self.registrarLog("generando arbol fractal");
                red = Algoritmos::TopologiasFractales::generarArbolFractal(iteraciones);
                reset_fractal();
            }
            if (ImGui::Button("copo de nieve de koch", ImVec2(-1, 32))) {
                self.registrarLog("generando copo de koch");
                red = Algoritmos::TopologiasFractales::generarKoch(iteraciones);
                reset_fractal();
            }
            if (ImGui::Button("malla hexagonal", ImVec2(-1, 32))) {
                self.registrarLog("generando malla hexagonal");
                red = Algoritmos::TopologiasFractales::generarMallaHexagonal(iteraciones);
                reset_fractal();
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ve a 'coloreo' para probar los colores dinámicos.");
            break;
        }
        case EstadoUI::CatEulerHamilton: {
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.4f, 1.0f), ICON_FA_ARROW_RIGHT_ARROW_LEFT " EULER Y HAMILTON");
            ImGui::TextWrapped("Busca caminos y circuitos eulerianos y hamiltonianos en el grafo.");
            ImGui::Spacing();
            auto props = Algoritmos::AnalizadorGrafo::analizar(red);
            if (props.es_euleriano) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ICON_FA_CHECK " Es un grafo Euleriano.");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ICON_FA_XMARK " NO es un grafo Euleriano.");
            }
            ImGui::Spacing();
            if (ImGui::Button(ICON_FA_PLAY " Buscar Circuito Euleriano", ImVec2(-1, 32))) {
                self.registrarLog("Ejecutando busqueda de circuito euleriano...");
                auto camino = Algoritmos::EulerHamilton::buscarCaminoEuleriano(red);
                if (!camino.empty()) {
                    std::vector<PasoAnimacion> pasos;
                    for (size_t i = 0; i < camino.size(); i++) {
                        int actual = camino[i];
                        int prev = (i > 0) ? camino[i-1] : actual;
                        pasos.push_back({PasoAnimacion::VISITAR, actual, prev, actual, "Visitando " + red.nombreNodo(actual)});
                        if (i > 0) {
                            pasos.push_back({PasoAnimacion::CONFIRMAR, actual, prev, actual, "Arista cruzada"});
                        }
                    }
                    pasos.push_back({PasoAnimacion::CONFIRMAR, camino.back(), -1, -1, "Circuito euleriano completado"});
                    AnimacionUI::iniciar(self, pasos);
                    self.estado_grafos.ruta_optima = camino;
                } else {
                    self.registrarLog("El grafo no contiene circuito/camino euleriano.");
                }
                self.estado_grafos.mostrar_mst = false;
            }
            ImGui::Spacing();
            if (ImGui::Button(ICON_FA_PLAY " Buscar Camino Hamiltoniano", ImVec2(-1, 32))) {
                self.registrarLog("Ejecutando busqueda de camino hamiltoniano...");
                auto camino = Algoritmos::EulerHamilton::buscarCaminoHamiltoniano(red);
                if (!camino.empty()) {
                    std::vector<PasoAnimacion> pasos;
                    for (size_t i = 0; i < camino.size(); i++) {
                        int actual = camino[i];
                        int prev = (i > 0) ? camino[i-1] : actual;
                        pasos.push_back({PasoAnimacion::VISITAR, actual, prev, actual, "Visitando " + red.nombreNodo(actual)});
                        if (i > 0) {
                            pasos.push_back({PasoAnimacion::CONFIRMAR, actual, prev, actual, "Arista cruzada"});
                        }
                    }
                    pasos.push_back({PasoAnimacion::CONFIRMAR, camino.back(), -1, -1, "Camino hamiltoniano completado"});
                    AnimacionUI::iniciar(self, pasos);
                    self.estado_grafos.ruta_optima = camino;
                } else {
                    self.registrarLog("El grafo no contiene camino hamiltoniano.");
                }
                self.estado_grafos.mostrar_mst = false;
            }
            break;
        }
        case EstadoUI::CatMatrices: Matrices::dibujar(red, self); break;
        default: break;
    }

    propiedadesNodo(self, red);
    ImGui::End();
}

// -- funcion para dibujar el menu de herramientas en modo grafos dentro del toolbar area --
// ya no se usa menuGeneral, ahora se usa el toolbar con categorias

} // namespace PanelGrafos
