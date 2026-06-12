#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/SimuladorRed.h"
#include "interfaz/util/Easing.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

class Interfaz;

namespace LienzoRed {

// ── Helpers ─────────────────────────────────────────────────────────────────
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

// Color de saturacion: verde→amarillo→rojo
inline ImU32 colorSaturacion(float uso) {
    if (uso < 0.5f) {
        float t = uso * 2.0f;
        return IM_COL32((int)(t * 255), 200, (int)((1.0f - t) * 100), 220);
    } else {
        float t = (uso - 0.5f) * 2.0f;
        return IM_COL32(255, (int)((1.0f - t) * 200), (int)((1.0f - t) * 50), 220);
    }
}

// Color health: verde→amarillo→rojo segun valor 0-1
inline ImU32 colorHealth(float v) {
    if (v < 0.5f) {
        float t = v * 2.0f;
        return IM_COL32((int)(t * 255), 200, (int)((1.0f - t) * 80), 200);
    } else {
        float t = (v - 0.5f) * 2.0f;
        return IM_COL32(255, (int)((1.0f - t) * 200), (int)((1.0f - t) * 50), 200);
    }
}

inline uint32_t uint32ColorProtocolo(const std::string& tipo) {
    return ColoresProtocolo::paraTipo(tipo);
}

inline ImU32 imColorProtocolo(const std::string& tipo) {
    uint32_t c = ColoresProtocolo::paraTipo(tipo);
    // ARGB to RGBA for ImGui
    return IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, (c >> 24) & 0xFF);
}

// ── Funcion principal de dibujo ─────────────────────────────────────────────
inline void dibujar(Grafo& red, Interfaz& self) {
    ImGui::Begin("Lienzo de Red");
    ImVec2 tamano = ImGui::GetContentRegionAvail();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float tiempo = (float)ImGui::GetTime();

    // ── Grilla ──────────────────────────────────────────────────────────────
    float grid = 25.0f;
    float off_x = fmodf(self.estado_ui.offset_lienzo.x, grid);
    float off_y = fmodf(self.estado_ui.offset_lienzo.y, grid);
    if (off_x < 0) off_x += grid;
    if (off_y < 0) off_y += grid;
    for (float x = off_x; x < tamano.x; x += grid)
        dl->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + tamano.y), IM_COL32(255, 255, 255, 12));
    for (float y = off_y; y < tamano.y; y += grid)
        dl->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + tamano.x, origin.y + y), IM_COL32(255, 255, 255, 12));

    // ── Mouse ───────────────────────────────────────────────────────────────
    ImVec2 mouse = ImGui::GetMousePos();
    bool en_canvas = ImGui::IsWindowHovered();

    bool editando_g2 = (self.estado_ui.herramienta_activa == EstadoUI::CatIsomorfismo && self.estado_grafos.iso_editando_g2);
    Grafo& grafo_actual = editando_g2 ? self.estado_grafos.grafo_iso_g2 : red;

    self.estado_ui.nodo_hover = -1;
    for (auto& n : grafo_actual.nodos) {
        float dx = mouse.x - n.posicion.x, dy = mouse.y - n.posicion.y;
        if (sqrtf(dx * dx + dy * dy) <= n.radio) self.estado_ui.nodo_hover = n.id;
    }

    // ── Click izquierdo: seleccion/arrastre + click en paquete ──────────────
    bool paquete_clickeado = false;
    if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        self.estado_ui.nodo_seleccionado = self.estado_ui.nodo_hover;
        if (self.estado_ui.nodo_seleccionado != -1) self.estado_ui.arrastrando = true;

        // Detectar click en paquete (modo red)
        if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes && self.estado_redes.sim_inicializada) {
            int pkt_id = -1;
            float dist_min = 20.0f;
            for (const auto& pkt : self.estado_redes.simulador.obtenerPaquetes()) {
                if (pkt.paso_actual + 1 >= (int)pkt.ruta.size()) continue;
                int u = pkt.ruta[pkt.paso_actual];
                int v = pkt.ruta[pkt.paso_actual + 1];
                Nodo* no = red.obtenerNodo(u);
                Nodo* nd = red.obtenerNodo(v);
                if (!no || !nd) continue;
                float t = Easing::easeInOutCubic(pkt.progreso);
                ImVec2 pos_pkt(
                    no->posicion.x + (nd->posicion.x - no->posicion.x) * t,
                    no->posicion.y + (nd->posicion.y - no->posicion.y) * t
                );
                float d = sqrtf((mouse.x - pos_pkt.x) * (mouse.x - pos_pkt.x) + (mouse.y - pos_pkt.y) * (mouse.y - pos_pkt.y));
                if (d < dist_min) {
                    dist_min = d;
                    pkt_id = pkt.id;
                }
            }
            if (pkt_id >= 0) {
                self.estado_redes.paquete_inspector_id = pkt_id;
                self.estado_redes.mostrar_inspector = true;
                paquete_clickeado = true;
            }
        }
    }

    // ── Arrastre ────────────────────────────────────────────────────────────
    if (!paquete_clickeado) {
        if (self.estado_ui.arrastrando && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Nodo* n = grafo_actual.obtenerNodo(self.estado_ui.nodo_seleccionado);
            if (n) {
                ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                if (ImGui::GetIO().KeyShift) {
                    for (auto& n_iter : grafo_actual.nodos) {
                        n_iter.posicion.x += d.x; n_iter.posicion.y += d.y;
                    }
                } else {
                    n->posicion.x += d.x; n->posicion.y += d.y;
                }
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) self.estado_ui.arrastrando = false;
    }

    // ── Panning ─────────────────────────────────────────────────────────────
    bool panning_activo = ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
        (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && self.estado_ui.nodo_seleccionado == -1 && self.estado_ui.nodo_hover == -1);
    if (en_canvas && panning_activo) {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        self.estado_ui.offset_lienzo.x += d.x; self.estado_ui.offset_lienzo.y += d.y;
        for (auto& n : red.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
        for (auto& n : self.estado_grafos.grafo_iso_g2.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
    }

    // ── Zoom ────────────────────────────────────────────────────────────────
    float wheel = ImGui::GetIO().MouseWheel;
    if (en_canvas && wheel != 0.0f) {
        float factor = (wheel > 0) ? 1.1f : 0.9f;
        for (auto& n : red.nodos) {
            n.posicion.x = mouse.x + (n.posicion.x - mouse.x) * factor;
            n.posicion.y = mouse.y + (n.posicion.y - mouse.y) * factor;
            n.radio = std::max(5.0f, std::min(60.0f, n.radio * factor));
        }
        for (auto& n : self.estado_grafos.grafo_iso_g2.nodos) {
            n.posicion.x = mouse.x + (n.posicion.x - mouse.x) * factor;
            n.posicion.y = mouse.y + (n.posicion.y - mouse.y) * factor;
            n.radio = std::max(5.0f, std::min(60.0f, n.radio * factor));
        }
        self.estado_ui.offset_lienzo.x = mouse.x + (self.estado_ui.offset_lienzo.x - mouse.x) * factor;
        self.estado_ui.offset_lienzo.y = mouse.y + (self.estado_ui.offset_lienzo.y - mouse.y) * factor;
    }

    // ── Click derecho ───────────────────────────────────────────────────────
    if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        if (self.estado_ui.nodo_hover == -1) {
            self.estado_ui.pos_click_derecho = mouse;
            if (editando_g2) {
                grafo_actual.agregarNodo(mouse);
                grafo_actual.nodos.back().nombre = "U" + std::to_string(grafo_actual.nodos.back().id);
                self.estado_grafos.iso_analizado = false;
            } else if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
                ImGui::OpenPopup("CrearEquipo");
            } else {
                grafo_actual.agregarNodo(mouse);
                grafo_actual.nodos.back().nombre = "V" + std::to_string(grafo_actual.nodos.back().id);
                self.registrarLog("Nodo creado: " + grafo_actual.nodos.back().nombre);
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            }
        } else if (self.estado_ui.nodo_seleccionado != -1 && self.estado_ui.nodo_seleccionado != self.estado_ui.nodo_hover) {
            self.estado_ui.pendiente_arista_origen = self.estado_ui.nodo_seleccionado;
            self.estado_ui.pendiente_arista_destino = self.estado_ui.nodo_hover;
            self.estado_ui.pendiente_arista_peso = 1.0f;
            ImGui::OpenPopup("CrearArista");
        }
    }

    // ── Delete ──────────────────────────────────────────────────────────────
    if (self.estado_ui.nodo_seleccionado != -1 && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && !ImGui::GetIO().WantTextInput) {
        if (editando_g2) {
            grafo_actual.eliminarNodo(self.estado_ui.nodo_seleccionado);
            self.estado_grafos.iso_analizado = false;
        } else {
            self.registrarLog("Nodo eliminado: " + grafo_actual.nombreNodo(self.estado_ui.nodo_seleccionado));
            grafo_actual.eliminarNodo(self.estado_ui.nodo_seleccionado);
            self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
        }
        self.estado_ui.nodo_seleccionado = -1; self.estado_ui.arrastrando = false;
    }

    // ── Popups (sin cambios) ────────────────────────────────────────────────
    if (ImGui::BeginPopup("CrearEquipo")) {
        ImGui::Text(ICON_FA_PLUS " Nuevo Equipamiento"); ImGui::Separator();
        static int tipo_sel = 0;
        const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
        const char* iconos[] = {ICON_FA_SERVER, ICON_FA_NETWORK_WIRED, ICON_FA_RIGHT_LEFT, ICON_FA_SHIELD_HALVED, ICON_FA_DESKTOP};
        for (int i = 0; i < 5; i++) {
            char label[64]; snprintf(label, sizeof(label), "%s %s", iconos[i], tipos[i]);
            if (ImGui::Selectable(label)) {
                grafo_actual.agregarNodo(self.estado_ui.pos_click_derecho, (TipoHardware)i);
                self.registrarLog("Hardware desplegado: " + std::string(tipos[i]) + " " + grafo_actual.nodos.back().nombre);
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            }
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("CrearArista")) {
        ImGui::Text(ICON_FA_LINK " Nueva Conexion");
        ImGui::Text("%s -> %s", grafo_actual.nombreNodo(self.estado_ui.pendiente_arista_origen).c_str(),
                    grafo_actual.nombreNodo(self.estado_ui.pendiente_arista_destino).c_str());
        ImGui::Separator();
        ImGui::InputFloat("Peso / Latencia", &self.estado_ui.pendiente_arista_peso, 0.5f, 5.0f, "%.1f");
        if (self.estado_ui.pendiente_arista_peso < 0.1f) self.estado_ui.pendiente_arista_peso = 0.1f;
        if (ImGui::Button(ICON_FA_CHECK " Crear", ImVec2(100, 0))) {
            grafo_actual.agregarArista(self.estado_ui.pendiente_arista_origen, self.estado_ui.pendiente_arista_destino, self.estado_ui.pendiente_arista_peso);
            if (!editando_g2) {
                self.registrarLog("Arista creada: " + grafo_actual.nombreNodo(self.estado_ui.pendiente_arista_origen) + " - " +
                    grafo_actual.nombreNodo(self.estado_ui.pendiente_arista_destino) + " (peso=" + std::to_string((int)self.estado_ui.pendiente_arista_peso) + ")");
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK " Cancelar", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // ── Lista de grafos a dibujar ───────────────────────────────────────────
    std::vector<Grafo*> grafos_a_dibujar = {&red};
    if (self.estado_ui.herramienta_activa == EstadoUI::CatIsomorfismo)
        grafos_a_dibujar.push_back(&self.estado_grafos.grafo_iso_g2);

    // ── Recorrer grafos y dibujar ───────────────────────────────────────────
    bool modo_red = (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes && self.estado_redes.sim_inicializada);
    for (Grafo* ptr_g : grafos_a_dibujar) {
        Grafo& g_dib = *ptr_g;
        bool es_g2 = (&g_dib == &self.estado_grafos.grafo_iso_g2);

        // ── DIBUJAR ARISTAS ─────────────────────────────────────────────────
        for (size_t idx = 0; idx < g_dib.aristas.size(); idx++) {
            const auto& a = g_dib.aristas[idx];
            Nodo* o = g_dib.obtenerNodo(a.origen_id);
            Nodo* d = g_dib.obtenerNodo(a.destino_id);
            if (!o || !d) continue;

            ImU32 col = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 120);
            float grosor = 2.0f + std::min(a.peso_actual / 10.0f, 4.0f);

            auto par = std::make_pair(std::min(a.origen_id, a.destino_id), std::max(a.origen_id, a.destino_id));

            // ── Animacion de algoritmo (sin cambios) ────────────────────────
            if (!es_g2) {
                bool anim_activa = (self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0);
                if (anim_activa) {
                    auto parR = std::make_pair(a.destino_id, a.origen_id);
                    if (self.estado_grafos.anim_estado.confirmadas.count(par) || self.estado_grafos.anim_estado.confirmadas.count(parR)) {
                        col = IM_COL32(255, 179, 0, 255); grosor = 5.0f;
                        dl->AddLine(o->posicion, d->posicion, IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                    } else if (self.estado_grafos.anim_estado.exploradas.count(par) || self.estado_grafos.anim_estado.exploradas.count(parR)) {
                        col = IM_COL32(0, 188, 212, 220); grosor = 3.5f;
                    } else if (self.estado_grafos.anim_estado.descartadas.count(par) || self.estado_grafos.anim_estado.descartadas.count(parR)) {
                        float pulse_a = (sinf(tiempo * 5.0f) + 1.0f) * 0.5f;
                        col = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60)); grosor = 2.5f;
                    }
                }
                bool mostrar_res = (!self.estado_grafos.anim_estado.activa && self.estado_grafos.anim_estado.paso_actual >= (int)self.estado_grafos.anim_estado.pasos.size() - 1);
                if (!anim_activa || mostrar_res) {
                    if (!self.estado_grafos.mostrar_mst && self.estado_grafos.ruta_optima.size() >= 2) {
                        for (size_t i = 0; i + 1 < self.estado_grafos.ruta_optima.size(); i++) {
                            if ((a.origen_id == self.estado_grafos.ruta_optima[i] && a.destino_id == self.estado_grafos.ruta_optima[i + 1]) ||
                                (!a.es_dirigida && a.origen_id == self.estado_grafos.ruta_optima[i + 1] && a.destino_id == self.estado_grafos.ruta_optima[i])) {
                                col = IM_COL32(255, 179, 0, 255); grosor = 6.0f;
                                dl->AddLine(o->posicion, d->posicion, IM_COL32(255, 255, 255, 150), grosor + 4.0f); break;
                            }
                        }
                    }
                    if (self.estado_grafos.mostrar_mst) {
                        for (const auto& m : self.estado_grafos.aristas_mst) {
                            if ((a.origen_id == m.origen_id && a.destino_id == m.destino_id) ||
                                (!a.es_dirigida && a.origen_id == m.destino_id && a.destino_id == m.origen_id))
                                { col = IM_COL32(200, 50, 200, 255); grosor = 5.0f; break; }
                        }
                    }
                }
            }

            // ── Brillo de saturacion en modo red ────────────────────────────
            bool modo_red_edge = (modo_red && !es_g2);
            float uso_arista = 0.0f;
            bool arista_caida = false;
            if (modo_red_edge) {
                uso_arista = self.estado_redes.simulador.usoArista(a.origen_id, a.destino_id);
                auto key = std::make_pair(a.origen_id, a.destino_id);
                if (self.estado_redes.simulador.estado.aristas.count(key))
                    arista_caida = !self.estado_redes.simulador.estado.aristas.at(key).activa;

                if (arista_caida) {
                    // Arista caida: rojo punteado
                    col = IM_COL32(255, 50, 50, 180);
                    grosor = 2.0f;
                    // Dibujar linea punteada
                    int segmentos = 8;
                    for (int seg = 0; seg < segmentos; seg++) {
                        float t0 = (float)seg / segmentos;
                        float t1 = (float)(seg + 1) / segmentos;
                        ImVec2 p0(o->posicion.x + (d->posicion.x - o->posicion.x) * t0,
                                  o->posicion.y + (d->posicion.y - o->posicion.y) * t0);
                        if (seg % 2 == 0) {
                            ImVec2 p1(o->posicion.x + (d->posicion.x - o->posicion.x) * t1,
                                      o->posicion.y + (d->posicion.y - o->posicion.y) * t1);
                            dl->AddLine(p0, p1, col, grosor);
                        }
                    }
                } else if (uso_arista > 0.02f) {
                    // Brillo de fondo animado
                    float glow_intensity = 0.2f + uso_arista * 0.5f;
                    ImU32 glow_col = colorSaturacion(uso_arista);
                    glow_col = (glow_col & 0x00FFFFFF) | ((int)(glow_intensity * 60) << 24);
                    dl->AddLine(o->posicion, d->posicion, glow_col, grosor + 4.0f);

                    // Onda viajera: punto brillante que se mueve por la linea
                    float t_onda = fmod(tiempo * (0.5f + uso_arista * 2.0f), 1.0f);
                    ImVec2 pos_onda(
                        o->posicion.x + (d->posicion.x - o->posicion.x) * t_onda,
                        o->posicion.y + (d->posicion.y - o->posicion.y) * t_onda
                    );
                    ImU32 col_onda = colorSaturacion(uso_arista);
                    dl->AddCircleFilled(pos_onda, 3.0f + uso_arista * 3.0f, col_onda, 10);

                    // Color de arista base segun saturacion
                    col = colorSaturacion(uso_arista);
                    grosor = 2.0f + uso_arista * 4.0f;
                }
            }

            // Dibujar linea base de la arista
            if (!arista_caida) {
                dl->AddLine(o->posicion, d->posicion, col, grosor);
            }

            // ── Peso sobre la arista ────────────────────────────────────────
            if (!arista_caida) {
                ImVec2 mid((o->posicion.x + d->posicion.x) * 0.5f, (o->posicion.y + d->posicion.y) * 0.5f);
                char peso_txt[16];
                if (modo_red && uso_arista > 0.02f)
                    snprintf(peso_txt, sizeof(peso_txt), "%.0f%%", uso_arista * 100.0f);
                else
                    snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
                ImVec2 ts = ImGui::CalcTextSize(peso_txt);
                dl->AddRectFilled(
                    ImVec2(mid.x - ts.x * 0.5f - 3, mid.y - ts.y * 0.5f - 1),
                    ImVec2(mid.x + ts.x * 0.5f + 3, mid.y + ts.y * 0.5f + 1),
                    IM_COL32(20, 20, 25, 200), 3.0f);
                ImU32 col_txt = (modo_red && uso_arista > 0.02f) ? colorSaturacion(uso_arista) : IM_COL32(200, 210, 220, 220);
                dl->AddText(ImVec2(mid.x - ts.x * 0.5f, mid.y - ts.y * 0.5f), col_txt, peso_txt);
            }

            // ── Resaltado de cruces (modo coloreo) ──────────────────────────
            if (!es_g2 && self.estado_grafos.mostrar_coloreo &&
                self.estado_grafos.planar_analizado &&
                self.estado_grafos.mostrar_cruces &&
                self.estado_grafos.resultado_planaridad.cruces_estimadas > 0) {
                // Buscar si esta arista esta en la lista de cruces
                bool cruza = false;
                for (const auto& par : self.estado_grafos.resultado_planaridad.aristas_cruce) {
                    if (par.first == (int)idx || par.second == (int)idx) { cruza = true; break; }
                }
                if (cruza) {
                    float pulse = sinf(tiempo * 3.0f + idx) * 0.3f + 0.7f;
                    ImU32 col_cruce = IM_COL32(255, 120, 0, (int)(pulse * 120));
                    dl->AddLine(o->posicion, d->posicion, col_cruce, 6.0f);
                }
            }

            // ── X animada para arista caida ─────────────────────────────────
            if (arista_caida) {
                ImVec2 mid2((o->posicion.x + d->posicion.x) * 0.5f, (o->posicion.y + d->posicion.y) * 0.5f);
                float pulse_a = (sinf(tiempo * 4.0f) + 1.0f) * 0.5f;
                float sz = 8.0f + pulse_a * 4.0f;
                ImU32 col_x = IM_COL32(255, 50, 50, (int)(150 + pulse_a * 105));
                dl->AddLine(ImVec2(mid2.x - sz, mid2.y - sz), ImVec2(mid2.x + sz, mid2.y + sz), col_x, 3.0f);
                dl->AddLine(ImVec2(mid2.x + sz, mid2.y - sz), ImVec2(mid2.x - sz, mid2.y + sz), col_x, 3.0f);
            }
        }

        // ── DIBUJAR PAQUETES DE SIMULACION (mejorados) ──────────────────────
        if (modo_red && !es_g2) {
            for (const auto& pkt : self.estado_redes.simulador.obtenerPaquetes()) {
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

                ImU32 col_pkt = imColorProtocolo(pkt.tipo);
                float tam = 2.5f + pkt.tamaño_mb * 0.3f;
                if (tam > 5.5f) tam = 5.5f;

                // ── Estela (trail): 4 puntos fantasmas detras ───────────────
                for (int trail = 1; trail <= 4; trail++) {
                    float tt = std::max(0.0f, pkt.progreso - trail * 0.04f);
                    float t_trail = Easing::easeInOutCubic(tt);
                    ImVec2 pos_trail(
                        no->posicion.x + (nd->posicion.x - no->posicion.x) * t_trail,
                        no->posicion.y + (nd->posicion.y - no->posicion.y) * t_trail
                    );
                    float alpha_t = (1.0f - trail * 0.2f) * 0.6f;
                    dl->AddCircleFilled(pos_trail, tam * (1.0f - trail * 0.15f),
                        IM_COL32(
                            ((col_pkt >> IM_COL32_R_SHIFT) & 0xFF),
                            ((col_pkt >> IM_COL32_G_SHIFT) & 0xFF),
                            ((col_pkt >> IM_COL32_B_SHIFT) & 0xFF),
                            (int)(alpha_t * 255)
                        ), 10);
                }

                // ── Halo ────────────────────────────────────────────────────
                dl->AddCircleFilled(pos_pkt, tam * 4.0f,
                    ((col_pkt & 0x00FFFFFF) | (18 << 24)), 14);

                // ── Punto principal ─────────────────────────────────────────
                dl->AddCircleFilled(pos_pkt, tam, col_pkt, 20);

                // ── Brillo central ──────────────────────────────────────────
                dl->AddCircleFilled(pos_pkt, tam * 0.35f,
                    IM_COL32(255, 255, 255, 160), 10);

                // Etiqueta de protocolo
                char label[2] = {ColoresProtocolo::icono(pkt.tipo)[0], '\0'};
                ImVec2 ls = ImGui::CalcTextSize(label);
                dl->AddText(ImVec2(pos_pkt.x - ls.x * 0.5f - 8, pos_pkt.y - ls.y * 0.5f - 8),
                    IM_COL32(255, 255, 255, 180), label);
            }
        }

        // ── DIBUJAR NODOS ───────────────────────────────────────────────────
        for (auto& n : g_dib.nodos) {
            ImU32 colorFondo, colorBorde;
            bool es_anim = (!es_g2 && (self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0));

            // ── Color del nodo ─────────────────────────────────────────────
            if (es_g2) {
                colorFondo = IM_COL32(180, 180, 0, 255);
                colorBorde = IM_COL32(255, 255, 100, 255);
            } else if (es_anim && self.estado_grafos.anim_estado.procesando.count(n.id)) {
                colorFondo = IM_COL32(255, 215, 0, 200);
                colorBorde = IM_COL32(255, 235, 100, 255);
            } else if (es_anim && self.estado_grafos.anim_estado.visitados.count(n.id)) {
                colorFondo = IM_COL32(0, 230, 118, 200);
                colorBorde = IM_COL32(100, 255, 180, 255);
            } else if (self.estado_grafos.mostrar_coloreo) {
                if (self.estado_grafos.modo_fractal &&
                    (int)self.estado_grafos.colores_fractales.size() > n.id &&
                    self.estado_grafos.colores_fractales[n.id] != 0) {
                    // Color fractal directo (ARGB)
                    uint32_t fc = self.estado_grafos.colores_fractales[n.id];
                    colorFondo = IM_COL32(
                        (fc >> 16) & 0xFF, (fc >> 8) & 0xFF, fc & 0xFF, 255);
                    colorBorde = IM_COL32(255, 255, 255, 180);
                } else if ((int)self.estado_grafos.colores_nodos.size() > n.id &&
                           self.estado_grafos.colores_nodos[n.id] >= 0) {
                    ImU32 paleta[] = {
                        IM_COL32(230, 60, 60, 255), IM_COL32(60, 200, 80, 255),
                        IM_COL32(60, 100, 230, 255), IM_COL32(230, 200, 50, 255),
                        IM_COL32(200, 60, 200, 255), IM_COL32(60, 200, 200, 255)
                    };
                    colorFondo = paleta[self.estado_grafos.colores_nodos[n.id] % 6];
                    colorBorde = IM_COL32(255, 255, 255, 180);
                } else {
                    colorFondo = IM_COL32(40, 40, 48, 255);
                    colorBorde = IM_COL32(100, 100, 100, 200);
                }
            } else if (self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
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

            // ── Anillo de salud (modo red, sim activa) ──────────────────────
            if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
                const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
                if (en.activo) {
                    float health = std::max(en.cpu_uso, en.memoria_uso);
                    ImU32 col_ring = colorHealth(health);
                    float pulse = sinf(tiempo * 2.0f + n.id) * 0.3f + 1.0f;
                    float ring_r = n.radio + 4.0f + health * 4.0f * pulse;
                    dl->AddCircle(n.posicion, ring_r, col_ring, 32, 2.5f + health * 2.0f);
                }
            }

            // ── Seleccion / hover ──────────────────────────────────────────
            if ((es_g2 && editando_g2 && n.id == self.estado_ui.nodo_seleccionado) ||
                (!es_g2 && !editando_g2 && n.id == self.estado_ui.nodo_seleccionado)) {
                float t_sel = tiempo;
                float pulse1 = sinf(t_sel * 3.0f) * 3.0f + 5.0f;
                float pulse2 = sinf(t_sel * 3.0f + 1.0f) * 2.0f + 3.0f;
                float alpha1 = (sinf(t_sel * 3.0f) + 1.0f) * 0.5f;
                dl->AddCircle(n.posicion, n.radio + pulse1 + 4, IM_COL32(255, 200, 80, (int)(40 + alpha1 * 40)), 32, 2.0f);
                dl->AddCircleFilled(n.posicion, n.radio + pulse2 + 2, IM_COL32(255, 180, 50, 30), 32);
                colorBorde = IM_COL32(255, 210, 90, 255);
            }
            if ((es_g2 && editando_g2 && n.id == self.estado_ui.nodo_hover && n.id != self.estado_ui.nodo_seleccionado) ||
                (!es_g2 && !editando_g2 && n.id == self.estado_ui.nodo_hover && n.id != self.estado_ui.nodo_seleccionado)) {
                dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(0, 255, 200, 30), 32);
            }

            // ── Arbol highlights ───────────────────────────────────────────
            if (!es_g2 && self.estado_grafos.arbol_analizado) {
                if (n.id == self.estado_grafos.arbol_props.raiz_id)
                    dl->AddCircleFilled(n.posicion, n.radio + 6, IM_COL32(255, 200, 0, 60), 32);
                auto& hojas_ref = self.estado_grafos.arbol_props.hojas;
                if (std::find(hojas_ref.begin(), hojas_ref.end(), n.id) != hojas_ref.end())
                    dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(50, 200, 50, 50), 32);
                if (self.estado_grafos.arbol_props.nivel.count(n.id)) {
                    char nivel_txt[8]; snprintf(nivel_txt, sizeof(nivel_txt), "L%d", self.estado_grafos.arbol_props.nivel.at(n.id));
                    ImVec2 lt = ImGui::CalcTextSize(nivel_txt);
                    dl->AddText(ImVec2(n.posicion.x - lt.x * 0.5f, n.posicion.y - n.radio - 18), IM_COL32(180, 220, 180, 200), nivel_txt);
                }
            }

            // ── Efecto pop al visitar ──────────────────────────────────────
            if (es_anim && self.estado_grafos.anim_estado.visitados.count(n.id) &&
                self.estado_grafos.anim_estado.tiempo_visita_nodo.count(n.id)) {
                float t_since = tiempo - self.estado_grafos.anim_estado.tiempo_visita_nodo[n.id];
                if (t_since < 0.4f) {
                    float scale = 1.0f + Easing::easeOutBounce(t_since / 0.4f) * 0.3f;
                    dl->AddCircleFilled(n.posicion, n.radio * scale, IM_COL32(0, 230, 120, 80), 32);
                }
            }

            // ── Circulo del nodo ───────────────────────────────────────────
            dl->AddCircleFilled(n.posicion, n.radio, colorFondo, 32);
            dl->AddCircle(n.posicion, n.radio, colorBorde, 32, 2.5f);

            // pulso fractal: anillo brillante cuando el modo fractal esta activo
            if (self.estado_grafos.mostrar_coloreo && self.estado_grafos.modo_fractal && !es_g2) {
                float fp = sinf(tiempo * 2.0f + n.id * 1.7f) * 0.3f + 0.7f;
                dl->AddCircle(n.posicion, n.radio + 6 + fp * 4, IM_COL32(150, 60, 200, (int)(fp * 120)), 32, 1.5f);
            }

            // ── Texto del nodo ─────────────────────────────────────────────
            if (!es_g2 && self.estado_ui.modo_actual == Interfaz::ModoApp::Redes) {
                const char* icono = iconoHardware(n.tipo);
                ImVec2 is = ImGui::CalcTextSize(icono);
                dl->AddText(ImVec2(n.posicion.x - is.x * 0.5f, n.posicion.y - is.y * 0.5f), IM_COL32(255, 255, 255, 230), icono);
                ImVec2 ns = ImGui::CalcTextSize(n.nombre.c_str());
                dl->AddText(ImVec2(n.posicion.x - ns.x * 0.5f, n.posicion.y + n.radio + 3), IM_COL32(190, 195, 200, 200), n.nombre.c_str());
            } else {
                ImVec2 ts = ImGui::CalcTextSize(n.nombre.c_str());
                dl->AddText(ImVec2(n.posicion.x - ts.x * 0.5f, n.posicion.y - ts.y * 0.5f), IM_COL32(255, 255, 255, 255), n.nombre.c_str());
            }

            // ── Nodo caido ─────────────────────────────────────────────────
            if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
                const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
                if (!en.activo) {
                    float pulse_a = (sinf(tiempo * 3.0f) + 1.0f) * 0.5f;
                    dl->AddCircle(n.posicion, n.radio + 4, IM_COL32(255, 30, 30, (int)(150 + pulse_a * 105)), 32, 3.0f);
                    dl->AddLine(
                        ImVec2(n.posicion.x - n.radio, n.posicion.y - n.radio),
                        ImVec2(n.posicion.x + n.radio, n.posicion.y + n.radio),
                        IM_COL32(255, 30, 30, (int)(150 + pulse_a * 105)), 3.0f);
                    // Icono caida
                    dl->AddText(ImVec2(n.posicion.x - 6, n.posicion.y - n.radio - 16),
                        IM_COL32(255, 80, 80, 200), ICON_FA_TRIANGLE_EXCLAMATION);
                }
            }

            // ── Hover tooltip: metricas del nodo ───────────────────────────
            if (modo_red && n.id == self.estado_ui.nodo_hover && !es_g2 &&
                self.estado_redes.simulador.estado.nodos.count(n.id)) {
                const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
                char tooltip[256];
                snprintf(tooltip, sizeof(tooltip),
                    "CPU: %.0f%%  RAM: %.0f%%\nRX: %.0f  TX: %.0f\n%s",
                    en.cpu_uso * 100.0f, en.memoria_uso * 100.0f,
                    en.paquetes_rx, en.paquetes_tx,
                    en.activo ? "✅ Activo" : "❌ CAIDO");
                ImGui::SetTooltip("%s", tooltip);
            }
        } // end for nodos

        // ── PARTICULA DE ANIMACION ──────────────────────────────────────────
        if ((self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0) && self.estado_grafos.anim_estado.particula.activa && !es_g2) {
            float t_ease = Easing::easeInOutCubic(self.estado_grafos.anim_estado.particula.progreso);
            ImVec2 pos(
                self.estado_grafos.anim_estado.particula.pos_inicio.x + (self.estado_grafos.anim_estado.particula.pos_fin.x - self.estado_grafos.anim_estado.particula.pos_inicio.x) * t_ease,
                self.estado_grafos.anim_estado.particula.pos_inicio.y + (self.estado_grafos.anim_estado.particula.pos_fin.y - self.estado_grafos.anim_estado.particula.pos_inicio.y) * t_ease
            );
            float halo_r = self.estado_grafos.anim_estado.particula.radio * 2.5f * (1.0f - self.estado_grafos.anim_estado.particula.progreso * 0.5f);
            dl->AddCircleFilled(pos, halo_r, (self.estado_grafos.anim_estado.particula.color & 0x00FFFFFF) | (60 << 24), 24);
            dl->AddCircleFilled(pos, self.estado_grafos.anim_estado.particula.radio, self.estado_grafos.anim_estado.particula.color, 24);
            dl->AddCircleFilled(pos, self.estado_grafos.anim_estado.particula.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
        }
        } // end for grafos

    // -- overlays de resultados de algoritmo en el canvas --
    if (self.estado_ui.modo_actual == Interfaz::ModoApp::Grafos) {
        float ox = origin.x + 12, oy = origin.y + 12;

        // dijkstra: resultado de ruta
        if (!self.estado_grafos.ruta_optima.empty()) {
            char buf[128];
            snprintf(buf, sizeof(buf), "ruta optima: %d saltos, costo %.1f",
                (int)self.estado_grafos.ruta_optima.size() - 1,
                self.estado_grafos.dijkstra_costo_total);
            dl->AddText(ImVec2(ox, oy), IM_COL32(0, 255, 180, 240), ICON_FA_ROUTE);
            dl->AddText(ImVec2(ox + 22, oy), IM_COL32(0, 255, 180, 240), buf);
            oy += 22;
        }

        // bfs/dfs: resultado de recorrido
        if (self.estado_ui.herramienta_activa == EstadoUI::CatBusqueda) {
            if (!self.estado_grafos.bfs_resultado.orden_visita.empty()) {
                char buf[64];
                snprintf(buf, sizeof(buf), "BFS: %d nodos visitados",
                    (int)self.estado_grafos.bfs_resultado.orden_visita.size());
                dl->AddText(ImVec2(ox, oy), IM_COL32(100, 200, 255, 240), buf);
                oy += 22;
            }
            if (!self.estado_grafos.dfs_resultado.orden_visita.empty()) {
                char buf[64];
                snprintf(buf, sizeof(buf), "DFS: %d pasos, %d back-edges",
                    (int)self.estado_grafos.dfs_resultado.orden_visita.size(),
                    (int)self.estado_grafos.dfs_resultado.back_edges.size());
                dl->AddText(ImVec2(ox, oy), IM_COL32(180, 130, 255, 240), buf);
                oy += 22;
            }
        }

        // coloreo
        if (self.estado_grafos.mostrar_coloreo) {
            char buf[128];
            if (self.estado_grafos.modo_fractal) {
                snprintf(buf, sizeof(buf), "coloreo fractal: mandelbrot modula colores greedy | χ=%d",
                    self.estado_grafos.resultado_coloreo.num_colores);
                dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_FAN);
                dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
                oy += 22;
                dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                    "cada nodo hereda color greedy + fractal segun posicion en canvas");
            } else {
                snprintf(buf, sizeof(buf), "coloreo greedy | χ(greedy)=%d  χ(welsh-powell)=%d",
                    self.estado_grafos.resultado_coloreo.num_colores,
                    self.estado_grafos.resultado_welsh_powell.num_colores);
                dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_PAINTBRUSH);
                dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
                oy += 22;
                dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                    "adyacentes tienen colores distintos (greedy = cota superior)");
            }
            oy += 22;

            // badge de planaridad
            if (self.estado_grafos.planar_analizado) {
                ImU32 col_planar = self.estado_grafos.resultado_planaridad.es_planar
                    ? IM_COL32(0, 255, 120, 240) : IM_COL32(255, 150, 0, 240);
                const char* txt = self.estado_grafos.resultado_planaridad.es_planar
                    ? "grafo planar (χ<=4 por teorema 4-colores)"
                    : "no planar";
                dl->AddText(ImVec2(ox, oy), col_planar, txt);
                oy += 22;
            }
        }

        // kruskal/mst
        if (self.estado_grafos.mostrar_mst && !self.estado_grafos.aristas_mst.empty()) {
            float peso_total = 0;
            for (const auto& a : self.estado_grafos.aristas_mst) peso_total += a.peso;
            char buf[64];
            snprintf(buf, sizeof(buf), "MST: %d aristas, peso total %.1f",
                (int)self.estado_grafos.aristas_mst.size(), peso_total);
            dl->AddText(ImVec2(ox, oy), IM_COL32(200, 50, 200, 240), buf);
            oy += 22;
        }

        // ciclo
        if (self.estado_grafos.ciclo_analizado) {
            ImU32 col = self.estado_grafos.resultado_ciclos.tiene_ciclo
                ? IM_COL32(255, 180, 0, 240) : IM_COL32(0, 255, 120, 240);
            const char* txt = self.estado_grafos.resultado_ciclos.tiene_ciclo
                ? "ciclo detectado" : "grafo aciclico (arbol)";
            dl->AddText(ImVec2(ox, oy), col, txt);
            oy += 22;
        }

        // animacion en progreso
        if (self.estado_grafos.anim_estado.activa && self.estado_grafos.anim_estado.paso_actual >= 0 &&
            self.estado_grafos.anim_estado.paso_actual < (int)self.estado_grafos.anim_estado.pasos.size()) {
            const auto& paso = self.estado_grafos.anim_estado.pasos[self.estado_grafos.anim_estado.paso_actual];
            if (!paso.descripcion.empty()) {
                dl->AddText(ImVec2(ox, oy), IM_COL32(180, 230, 255, 240), paso.descripcion.c_str());
                oy += 22;
            }
        }
    }

    ImGui::End();
} // end dibujar()

} // namespace LienzoRed
