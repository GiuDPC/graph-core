#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "interfaz/util/Easing.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

class Interfaz;

namespace LienzoRed {

// Helper: icono FontAwesome segun tipo de hardware
inline const char* iconoHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return ICON_FA_SERVER;
        case TipoHardware::Router:   return ICON_FA_NETWORK_WIRED;
        case TipoHardware::Switch:   return ICON_FA_RIGHT_LEFT;
        case TipoHardware::Firewall: return ICON_FA_SHIELD_HALVED;
        case TipoHardware::Terminal: return ICON_FA_DESKTOP;
        default: return ICON_FA_CIRCLE;
    }
}

inline void dibujar(Grafo& red, Interfaz& self) {
        ImGui::Begin("Lienzo de Red");
        ImVec2 tamano = ImGui::GetContentRegionAvail();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos();

        // Grilla sutil (con offset de panning)
        float grid = 25.0f;
        float off_x = fmodf(self.offset_lienzo.x, grid);
        float off_y = fmodf(self.offset_lienzo.y, grid);
        if (off_x < 0) off_x += grid;
        if (off_y < 0) off_y += grid;
        for (float x = off_x; x < tamano.x; x += grid)
            dl->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + tamano.y), IM_COL32(255, 255, 255, 12));
        for (float y = off_y; y < tamano.y; y += grid)
            dl->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + tamano.x, origin.y + y), IM_COL32(255, 255, 255, 12));

        // Mouse
        ImVec2 mouse = ImGui::GetMousePos();
        bool en_canvas = ImGui::IsWindowHovered();

        bool editando_g2 = (self.modo_panel == Interfaz::ModoPanel::Isomorfismo && self.iso_editando_g2);
        Grafo& grafo_actual = editando_g2 ? self.grafo_iso_g2 : red;

        self.nodo_hover = -1;
        for (auto& n : grafo_actual.nodos) {
            float dx = mouse.x - n.posicion.x, dy = mouse.y - n.posicion.y;
            if (sqrtf(dx * dx + dy * dy) <= n.radio) self.nodo_hover = n.id;
        }

        // Click izquierdo — seleccion y arrastre
        if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            self.nodo_seleccionado = self.nodo_hover;
            if (self.nodo_seleccionado != -1) self.arrastrando = true;
        }
        if (self.arrastrando && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Nodo* n = grafo_actual.obtenerNodo(self.nodo_seleccionado);
            if (n) {
                ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                if (ImGui::GetIO().KeyShift) {
                    for (auto& n_iter : grafo_actual.nodos) {
                        n_iter.posicion.x += d.x;
                        n_iter.posicion.y += d.y;
                    }
                } else {
                    n->posicion.x += d.x; 
                    n->posicion.y += d.y;
                }
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) self.arrastrando = false;

        // Panning (Arrastre con boton medio o boton izquierdo en el fondo)
        bool panning_activo = ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || 
                              (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && self.nodo_seleccionado == -1 && self.nodo_hover == -1);
        if (en_canvas && panning_activo) {
            ImVec2 d = ImGui::GetIO().MouseDelta;
            self.offset_lienzo.x += d.x;
            self.offset_lienzo.y += d.y;
            for (auto& n : red.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
            for (auto& n : self.grafo_iso_g2.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
        }

        // Zoom (Rueda del raton)
        float wheel = ImGui::GetIO().MouseWheel;
        if (en_canvas && wheel != 0.0f) {
            float factor = (wheel > 0) ? 1.1f : 0.9f;
            for (auto& n : red.nodos) {
                n.posicion.x = mouse.x + (n.posicion.x - mouse.x) * factor;
                n.posicion.y = mouse.y + (n.posicion.y - mouse.y) * factor;
                n.radio = std::max(5.0f, std::min(60.0f, n.radio * factor));
            }
            for (auto& n : self.grafo_iso_g2.nodos) {
                n.posicion.x = mouse.x + (n.posicion.x - mouse.x) * factor;
                n.posicion.y = mouse.y + (n.posicion.y - mouse.y) * factor;
                n.radio = std::max(5.0f, std::min(60.0f, n.radio * factor));
            }
            self.offset_lienzo.x = mouse.x + (self.offset_lienzo.x - mouse.x) * factor;
            self.offset_lienzo.y = mouse.y + (self.offset_lienzo.y - mouse.y) * factor;
        }

        // Click derecho — crear nodo o arista
        if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            if (self.nodo_hover == -1) {
                self.pos_click_derecho = mouse;
                if (editando_g2) {
                    grafo_actual.agregarNodo(mouse);
                    grafo_actual.nodos.back().nombre = "U" + std::to_string(grafo_actual.nodos.back().id);
                    self.iso_analizado = false;
                } else if (self.modo_actual == Interfaz::ModoApp::Redes) {
                    ImGui::OpenPopup("CrearEquipo");
                } else {
                    grafo_actual.agregarNodo(mouse);
                    grafo_actual.nodos.back().nombre = "V" + std::to_string(grafo_actual.nodos.back().id);
                    self.registrarLog("Nodo creado: " + grafo_actual.nodos.back().nombre);
                    self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
                }
            } else if (self.nodo_seleccionado != -1 && self.nodo_seleccionado != self.nodo_hover) {
                self.pendiente_arista_origen = self.nodo_seleccionado;
                self.pendiente_arista_destino = self.nodo_hover;
                self.pendiente_arista_peso = 1.0f;
                ImGui::OpenPopup("CrearArista");
            }
        }

        // Delete
        if (self.nodo_seleccionado != -1 && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && !ImGui::GetIO().WantTextInput) {
            if (editando_g2) {
                grafo_actual.eliminarNodo(self.nodo_seleccionado);
                self.iso_analizado = false;
            } else {
                self.registrarLog("Nodo eliminado: " + grafo_actual.nombreNodo(self.nodo_seleccionado));
                grafo_actual.eliminarNodo(self.nodo_seleccionado);
                self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
            }
            self.nodo_seleccionado = -1; self.arrastrando = false;
        }

        // --- Popup: Crear Equipo (Redes) ---
        if (ImGui::BeginPopup("CrearEquipo")) {
            ImGui::Text(ICON_FA_PLUS " Nuevo Equipamiento");
            ImGui::Separator();
            static int tipo_sel = 0;
            const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
            const char* iconos[] = {ICON_FA_SERVER, ICON_FA_NETWORK_WIRED, ICON_FA_RIGHT_LEFT, ICON_FA_SHIELD_HALVED, ICON_FA_DESKTOP};
            for (int i = 0; i < 5; i++) {
                char label[64];
                snprintf(label, sizeof(label), "%s %s", iconos[i], tipos[i]);
                if (ImGui::Selectable(label)) {
                    grafo_actual.agregarNodo(self.pos_click_derecho, (TipoHardware)i);
                    self.registrarLog("Hardware desplegado: " + std::string(tipos[i]) + " " + grafo_actual.nodos.back().nombre);
                    self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
                }
            }
            ImGui::EndPopup();
        }

        // --- Popup: Crear Arista con peso ---
        if (ImGui::BeginPopup("CrearArista")) {
            ImGui::Text(ICON_FA_LINK " Nueva Conexion");
            ImGui::Text("%s -> %s", grafo_actual.nombreNodo(self.pendiente_arista_origen).c_str(),
                        grafo_actual.nombreNodo(self.pendiente_arista_destino).c_str());
            ImGui::Separator();
            ImGui::InputFloat("Peso / Latencia", &self.pendiente_arista_peso, 0.5f, 5.0f, "%.1f");
            if (self.pendiente_arista_peso < 0.1f) self.pendiente_arista_peso = 0.1f;

            if (ImGui::Button(ICON_FA_CHECK " Crear", ImVec2(100, 0))) {
                if (editando_g2) {
                    grafo_actual.agregarArista(self.pendiente_arista_origen, self.pendiente_arista_destino, self.pendiente_arista_peso);
                    self.iso_analizado = false;
                } else {
                    grafo_actual.agregarArista(self.pendiente_arista_origen, self.pendiente_arista_destino, self.pendiente_arista_peso);
                    self.registrarLog("Arista creada: " + grafo_actual.nombreNodo(self.pendiente_arista_origen) + " - " +
                        grafo_actual.nombreNodo(self.pendiente_arista_destino) + " (peso=" + std::to_string((int)self.pendiente_arista_peso) + ")");
                    self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK " Cancelar", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        std::vector<Grafo*> grafos_a_dibujar = {&red};
        if (self.modo_panel == Interfaz::ModoPanel::Isomorfismo) {
            grafos_a_dibujar.push_back(&self.grafo_iso_g2);
        }

        for (Grafo* ptr_g : grafos_a_dibujar) {
            Grafo& g_dib = *ptr_g;
            bool es_g2 = (&g_dib == &self.grafo_iso_g2);

            // ===== DIBUJAR ARISTAS =====
            for (size_t idx = 0; idx < g_dib.aristas.size(); idx++) {
                const auto& a = g_dib.aristas[idx];
                Nodo* o = g_dib.obtenerNodo(a.origen_id);
                Nodo* d = g_dib.obtenerNodo(a.destino_id);
                if (!o || !d) continue;

                ImU32 col = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 120);
                float grosor = 2.0f + (a.peso_actual / 10.0f);
                if (grosor > 6.0f) grosor = 6.0f;

                auto par = std::make_pair(std::min(a.origen_id, a.destino_id), std::max(a.origen_id, a.destino_id));

                if (!es_g2) {
                    // Animacion: color de aristas
                    bool anim_activa_o_finalizada = (self.anim_estado.activa || self.anim_estado.paso_actual >= 0);
                    
                    if (anim_activa_o_finalizada) {
                        auto parR = std::make_pair(a.destino_id, a.origen_id);

                        if (self.anim_estado.confirmadas.count(par) || self.anim_estado.confirmadas.count({a.origen_id, a.destino_id}) ||
                            self.anim_estado.confirmadas.count(parR)) {
                            // Arista confirmada: dorada con brillo
                            col    = IM_COL32(255, 179, 0, 255);
                            grosor = 5.0f;
                            // Agregar linea de brillo encima
                            dl->AddLine(o->posicion, d->posicion,
                                IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                        } else if (self.anim_estado.exploradas.count(par) || self.anim_estado.exploradas.count({a.origen_id, a.destino_id}) ||
                                   self.anim_estado.exploradas.count(parR)) {
                            col    = IM_COL32(0, 188, 212, 220);
                            grosor = 3.5f;
                        } else if (self.anim_estado.descartadas.count(par) || self.anim_estado.descartadas.count({a.origen_id, a.destino_id}) ||
                                   self.anim_estado.descartadas.count(parR)) {
                            // Arista descartada: roja pulsante
                            float pulse_a = (sinf((float)ImGui::GetTime() * 5.0f) + 1.0f) * 0.5f;
                            col    = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60));
                            grosor = 2.5f;
                        }
                    } 
                    
                    // Mostrar resultados finales siempre que no este corriendo la animacion activamente
                    // o cuando haya terminado.
                    bool mostrar_resultados_finales = (!self.anim_estado.activa && self.anim_estado.paso_actual >= (int)self.anim_estado.pasos.size() - 1);
                    
                    if (!anim_activa_o_finalizada || mostrar_resultados_finales) {
                        // Dijkstra highlight
                        if (!self.mostrar_mst && self.ruta_optima.size() >= 2) {
                            for (size_t i = 0; i + 1 < self.ruta_optima.size(); i++) {
                                if ((a.origen_id == self.ruta_optima[i] && a.destino_id == self.ruta_optima[i + 1]) ||
                                    (!a.es_dirigida && a.origen_id == self.ruta_optima[i + 1] && a.destino_id == self.ruta_optima[i])) {
                                    col = IM_COL32(255, 179, 0, 255); 
                                    grosor = 6.0f; 
                                    // Resaltar fuerte al final
                                    dl->AddLine(o->posicion, d->posicion, IM_COL32(255, 255, 255, 150), grosor + 4.0f);
                                    break;
                                }
                            }
                        }
                        // Kruskal highlight
                        if (self.mostrar_mst) {
                            for (const auto& m : self.aristas_mst) {
                                if ((a.origen_id == m.origen_id && a.destino_id == m.destino_id) ||
                                    (!a.es_dirigida && a.origen_id == m.destino_id && a.destino_id == m.origen_id)) {
                                    col = IM_COL32(200, 50, 200, 255); grosor = 5.0f; break;
                                }
                            }
                        }
                    }
                }

                dl->AddLine(o->posicion, d->posicion, col, grosor);

                // Animacion de flujo en aristas activas (modo red + sim)
                if (self.modo_actual == Interfaz::ModoApp::Redes && self.sim_inicializada && !es_g2) {
                    float uso_arista = self.simulador.usoArista(a.origen_id, a.destino_id);
                    if (uso_arista > 0.05f) {
                        // Linea punteada animada sobre la arista
                        float tiempo = (float)ImGui::GetTime() * 2.0f;
                        ImU32 col_flow = self.simulador.colorUsoArista(uso_arista);
                        for (int i = 0; i < 4; i++) {
                            float t_punto = fmod(tiempo + i * 0.25f, 1.0f);
                            ImVec2 p_flow(
                                o->posicion.x + (d->posicion.x - o->posicion.x) * t_punto,
                                o->posicion.y + (d->posicion.y - o->posicion.y) * t_punto
                            );
                            dl->AddCircleFilled(p_flow, 2.5f, col_flow, 8);
                        }
                    }
                }

                // Peso sobre la arista
                ImVec2 mid(
                    (o->posicion.x + d->posicion.x) * 0.5f,
                    (o->posicion.y + d->posicion.y) * 0.5f
                );
                char peso_txt[16];
                snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
                ImVec2 ts = ImGui::CalcTextSize(peso_txt);
                dl->AddRectFilled(
                    ImVec2(mid.x - ts.x * 0.5f - 3, mid.y - ts.y * 0.5f - 1),
                    ImVec2(mid.x + ts.x * 0.5f + 3, mid.y + ts.y * 0.5f + 1),
                    IM_COL32(20, 20, 25, 200), 3.0f
                );
                dl->AddText(ImVec2(mid.x - ts.x * 0.5f, mid.y - ts.y * 0.5f), IM_COL32(200, 210, 220, 220), peso_txt);
            }

            // Overlay de uso de aristas (modo red + simulacion activa)
            if (self.modo_actual == Interfaz::ModoApp::Redes && self.sim_inicializada) {
                for (const auto& a : red.aristas) {
                    Nodo* o = red.obtenerNodo(a.origen_id);
                    Nodo* d = red.obtenerNodo(a.destino_id);
                    if (!o || !d) continue;

                    float uso = self.simulador.usoArista(a.origen_id, a.destino_id);
                    if (uso > 0.05f) {
                        // Dibujar barra de uso sobre la arista
                        ImVec2 mid((o->posicion.x + d->posicion.x) * 0.5f,
                                   (o->posicion.y + d->posicion.y) * 0.5f);
                        ImU32 col_uso = self.simulador.colorUsoArista(uso);

                        // Pequeno badge con %
                        char badge[16];
                        snprintf(badge, sizeof(badge), "%.0f%%", uso * 100.0f);
                        ImVec2 ts = ImGui::CalcTextSize(badge);
                        dl->AddRectFilled(
                            ImVec2(mid.x - ts.x * 0.5f - 4, mid.y - ts.y * 0.5f - 2),
                            ImVec2(mid.x + ts.x * 0.5f + 4, mid.y + ts.y * 0.5f + 2),
                            IM_COL32(15, 15, 20, 210), 3.0f);
                        dl->AddText(
                            ImVec2(mid.x - ts.x * 0.5f, mid.y - ts.y * 0.5f),
                            col_uso, badge);
                    }

                    // Arista caida: dibujar en rojo con X
                    auto key = std::make_pair(a.origen_id, a.destino_id);
                    if (self.simulador.estado.aristas.count(key) &&
                        !self.simulador.estado.aristas.at(key).activa) {
                        ImVec2 mid2((o->posicion.x + d->posicion.x) * 0.5f,
                                    (o->posicion.y + d->posicion.y) * 0.5f);
                        dl->AddText(ImVec2(mid2.x - 6, mid2.y - 8),
                            IM_COL32(255, 50, 50, 255), "X");
                    }
                }
            }

            // ===== DIBUJAR NODOS =====
            float tiempo = (float)ImGui::GetTime();
            for (auto& n : g_dib.nodos) {
                ImU32 colorFondo, colorBorde;
                bool es_anim = (!es_g2 && (self.anim_estado.activa || self.anim_estado.paso_actual >= 0));

                if (es_g2) {
                    colorFondo = IM_COL32(180, 180, 0, 255);
                    colorBorde = IM_COL32(255, 255, 100, 255);
                } else if (es_anim && self.anim_estado.procesando.count(n.id)) {
                    colorFondo = IM_COL32(255, 215, 0, 200);
                    colorBorde = IM_COL32(255, 235, 100, 255);
                } else if (es_anim && self.anim_estado.visitados.count(n.id)) {
                    colorFondo = IM_COL32(0, 230, 118, 200);
                    colorBorde = IM_COL32(100, 255, 180, 255);
                } else if (self.mostrar_coloreo && (int)self.colores_nodos.size() > n.id && self.colores_nodos[n.id] != -1) {
                    ImU32 paleta[] = {
                        IM_COL32(230, 60, 60, 255), IM_COL32(60, 200, 80, 255),
                        IM_COL32(60, 100, 230, 255), IM_COL32(230, 200, 50, 255),
                        IM_COL32(200, 60, 200, 255), IM_COL32(60, 200, 200, 255)
                    };
                    colorFondo = paleta[self.colores_nodos[n.id] % 6];
                    colorBorde = IM_COL32(255, 255, 255, 180);
                } else if (self.modo_actual == Interfaz::ModoApp::Redes) {
                    colorFondo = IM_COL32(40, 40, 48, 255);
                    switch (n.tipo) {
                        case TipoHardware::Servidor: colorBorde = IM_COL32(0, 200, 150, 255); break;
                        case TipoHardware::Router:   colorBorde = IM_COL32(255, 150, 0, 255); break;
                        case TipoHardware::Switch:   colorBorde = IM_COL32(77, 166, 255, 255); break;
                        case TipoHardware::Firewall: colorBorde = IM_COL32(255, 51, 51, 255); break;
                        case TipoHardware::Terminal: colorBorde = IM_COL32(179, 102, 255, 255); break;
                        default: colorBorde = IM_COL32(200, 200, 200, 255); break;
                    }
                } else {
                    colorFondo = IM_COL32(0, 160, 130, 255);
                    colorBorde = IM_COL32(200, 220, 230, 200);
                }

                if ((es_g2 && editando_g2 && n.id == self.nodo_seleccionado) ||
                    (!es_g2 && !editando_g2 && n.id == self.nodo_seleccionado)) {
                    // Anillo pulsante mas elaborado
                    float t_sel  = (float)ImGui::GetTime();
                    float pulse1 = sinf(t_sel * 3.0f) * 3.0f + 5.0f;
                    float pulse2 = sinf(t_sel * 3.0f + 1.0f) * 2.0f + 3.0f;
                    float alpha1 = (sinf(t_sel * 3.0f) + 1.0f) * 0.5f;

                    // Anillo exterior (fade)
                    dl->AddCircle(n.posicion, n.radio + pulse1 + 4,
                        IM_COL32(255, 200, 80, (int)(40 + alpha1 * 40)), 32, 2.0f);
                    // Anillo intermedio
                    dl->AddCircleFilled(n.posicion, n.radio + pulse2 + 2,
                        IM_COL32(255, 180, 50, 30), 32);
                    colorBorde = IM_COL32(255, 210, 90, 255);
                }
                if ((es_g2 && editando_g2 && n.id == self.nodo_hover && n.id != self.nodo_seleccionado) ||
                    (!es_g2 && !editando_g2 && n.id == self.nodo_hover && n.id != self.nodo_seleccionado)) {
                    dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(0, 255, 200, 30), 32);
                }

                // Highlight de arbol (solo para G1)
                if (!es_g2 && self.arbol_analizado) {
                    if (n.id == self.arbol_props.raiz_id) {
                        dl->AddCircleFilled(n.posicion, n.radio + 6, IM_COL32(255, 200, 0, 60), 32);
                    }
                    auto& hojas_ref = self.arbol_props.hojas;
                    if (std::find(hojas_ref.begin(), hojas_ref.end(), n.id) != hojas_ref.end()) {
                        dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(50, 200, 50, 50), 32);
                    }
                    if (self.arbol_props.nivel.count(n.id)) {
                        char nivel_txt[8];
                        snprintf(nivel_txt, sizeof(nivel_txt), "L%d", self.arbol_props.nivel.at(n.id));
                        ImVec2 lt = ImGui::CalcTextSize(nivel_txt);
                        dl->AddText(
                            ImVec2(n.posicion.x - lt.x * 0.5f, n.posicion.y - n.radio - 18),
                            IM_COL32(180, 220, 180, 200), nivel_txt);
                    }
                }

                // Efecto de "pop" al ser visitado por primera vez
                if (es_anim && self.anim_estado.visitados.count(n.id) &&
                    self.anim_estado.tiempo_visita_nodo.count(n.id)) {
                    float t_since = (float)ImGui::GetTime() - self.anim_estado.tiempo_visita_nodo[n.id];
                    if (t_since < 0.4f) {
                        float scale = 1.0f + Easing::easeOutBounce(t_since / 0.4f) * 0.3f;
                        float r_extra = n.radio * (scale - 1.0f);
                        dl->AddCircleFilled(n.posicion, n.radio + r_extra,
                            IM_COL32(0, 230, 120, 80), 32);
                    }
                }

                dl->AddCircleFilled(n.posicion, n.radio, colorFondo, 32);
                dl->AddCircle(n.posicion, n.radio, colorBorde, 32, 2.5f);

                if (!es_g2 && self.modo_actual == Interfaz::ModoApp::Redes) {
                    const char* icono = iconoHardware(n.tipo);
                    ImVec2 is = ImGui::CalcTextSize(icono);
                    dl->AddText(ImVec2(n.posicion.x - is.x * 0.5f, n.posicion.y - is.y * 0.5f),
                        IM_COL32(255, 255, 255, 230), icono);
                    ImVec2 ns = ImGui::CalcTextSize(n.nombre.c_str());
                    dl->AddText(ImVec2(n.posicion.x - ns.x * 0.5f, n.posicion.y + n.radio + 3),
                        IM_COL32(190, 195, 200, 200), n.nombre.c_str());
                } else {
                    ImVec2 ts = ImGui::CalcTextSize(n.nombre.c_str());
                    dl->AddText(ImVec2(n.posicion.x - ts.x * 0.5f, n.posicion.y - ts.y * 0.5f),
                        IM_COL32(255, 255, 255, 255), n.nombre.c_str());
                }

                // Overlay metricas por nodo (modo red + sim activa)
                if (!es_g2 && self.modo_actual == Interfaz::ModoApp::Redes && self.sim_inicializada &&
                    self.simulador.estado.nodos.count(n.id)) {
                    const auto& en = self.simulador.estado.nodos.at(n.id);

                    if (!en.activo) {
                        // Nodo caido: tachado en rojo
                        dl->AddCircle(n.posicion, n.radio + 4, IM_COL32(255, 30, 30, 200), 32, 3.0f);
                        dl->AddLine(
                            ImVec2(n.posicion.x - n.radio, n.posicion.y - n.radio),
                            ImVec2(n.posicion.x + n.radio, n.posicion.y + n.radio),
                            IM_COL32(255, 30, 30, 200), 2.5f);
                    } else {
                        // Metricas compactas debajo del nodo
                        char metricas[32];
                        snprintf(metricas, sizeof(metricas), "CPU:%.0f%% RAM:%.0f%%",
                            en.cpu_uso * 100.0f, en.memoria_uso * 100.0f);
                        ImVec2 ms = ImGui::CalcTextSize(metricas);
                        float y_badge = n.posicion.y + n.radio + 16;
                        dl->AddRectFilled(
                            ImVec2(n.posicion.x - ms.x * 0.5f - 3, y_badge - 2),
                            ImVec2(n.posicion.x + ms.x * 0.5f + 3, y_badge + ms.y + 1),
                            IM_COL32(10, 10, 15, 180), 2.0f);
                        // Color de CPU
                        ImU32 col_cpu = en.cpu_uso > 0.8f ? IM_COL32(255, 80, 80, 200) :
                                        en.cpu_uso > 0.5f ? IM_COL32(255, 200, 0, 200) :
                                                            IM_COL32(100, 200, 100, 200);
                        dl->AddText(ImVec2(n.posicion.x - ms.x * 0.5f, y_badge), col_cpu, metricas);
                    }
                }
            } // end loop nodos

            // ── Dibujar paquetes de la simulacion de red (multiples) ──────────
            if (self.modo_actual == Interfaz::ModoApp::Redes && self.sim_inicializada && !es_g2) {
                for (const auto& pkt : self.simulador.obtenerPaquetes()) {
                    if (pkt.paso_actual + 1 >= (int)pkt.ruta.size()) continue;
                    int u = pkt.ruta[pkt.paso_actual];
                    int v = pkt.ruta[pkt.paso_actual + 1];
                    Nodo* no = g_dib.obtenerNodo(u);
                    Nodo* nd = g_dib.obtenerNodo(v);
                    if (!no || !nd) continue;

                    float t = Easing::easeInOutCubic(pkt.progreso);
                    ImVec2 pos_pkt(
                        no->posicion.x + (nd->posicion.x - no->posicion.x) * t,
                        no->posicion.y + (nd->posicion.y - no->posicion.y) * t
                    );

                    // Color segun tipo
                    ImU32 col_pkt;
                    if (pkt.tipo == "PING")       col_pkt = IM_COL32(0, 255, 100, 220);
                    else if (pkt.tipo == "HTTP")  col_pkt = IM_COL32(80, 180, 255, 220);
                    else if (pkt.tipo == "VIDEO") col_pkt = IM_COL32(255, 150, 50, 220);
                    else if (pkt.tipo == "DDOS")  col_pkt = IM_COL32(255, 50, 50, 220);
                    else                          col_pkt = IM_COL32(200, 200, 200, 200);

                    float pkt_radio = 2.0f + (pkt.tamaño_mb * 0.3f);
                    if (pkt_radio > 5.0f) pkt_radio = 5.0f;
                    if (pkt_radio < 2.5f) pkt_radio = 2.5f;

                    // Halo suave
                    dl->AddCircleFilled(pos_pkt, pkt_radio * 3.0f, (col_pkt & 0x00FFFFFF) | (25 << 24), 12);
                    // Punto principal
                    dl->AddCircleFilled(pos_pkt, pkt_radio, col_pkt, 16);
                    // Brillo centrado
                    dl->AddCircleFilled(pos_pkt, pkt_radio * 0.35f, IM_COL32(255, 255, 255, 140), 8);
                }
            }

            // ── Dibujar particula de animacion ─────────────────────────────────
            if ((self.anim_estado.activa || self.anim_estado.paso_actual >= 0) && self.anim_estado.particula.activa) {
                float t_ease = Easing::easeInOutCubic(self.anim_estado.particula.progreso);

                ImVec2 pos(
                    self.anim_estado.particula.pos_inicio.x + (self.anim_estado.particula.pos_fin.x - self.anim_estado.particula.pos_inicio.x) * t_ease,
                    self.anim_estado.particula.pos_inicio.y + (self.anim_estado.particula.pos_fin.y - self.anim_estado.particula.pos_inicio.y) * t_ease
                );

                // Halo exterior pulsante
                float halo_r = self.anim_estado.particula.radio * 2.5f * (1.0f - self.anim_estado.particula.progreso * 0.5f);
                ImU32 col_halo = (self.anim_estado.particula.color & 0x00FFFFFF) | (60 << 24);
                dl->AddCircleFilled(pos, halo_r, col_halo, 24);

                // Punto principal
                dl->AddCircleFilled(pos, self.anim_estado.particula.radio, self.anim_estado.particula.color, 24);

                // Brillo central
                dl->AddCircleFilled(pos, self.anim_estado.particula.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
            }
        }

        ImGui::End();
    }

} // namespace LienzoRed
