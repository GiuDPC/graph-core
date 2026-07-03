#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.hpp"
#include "nucleo/SimuladorRed.h"
#include "interfaz/util/Easing.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

class Interfaz;

namespace LienzoRed {

// Helpers
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

// Color health
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

// ── Helpers para aristas dirigidas y curvas ──────────────────────────────

// Calcula los 3 vertices del triangulo de la flecha en el borde del nodo destino
inline void calcularPuntaFlecha(ImVec2 origen, ImVec2 destino, float radio_nodo,
                                 ImVec2& p1, ImVec2& p2, ImVec2& p3) {
    float dx = destino.x - origen.x;
    float dy = destino.y - origen.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.001f) { p1 = p2 = p3 = destino; return; }
    float nx = dx / dist;
    float ny = dy / dist;
    float bx = destino.x - nx * radio_nodo;
    float by = destino.y - ny * radio_nodo;
    float tam_flecha = 10.0f;
    float perp_x = -ny;
    float perp_y = nx;
    p1 = ImVec2(bx, by);
    p2 = ImVec2(bx - nx * tam_flecha + perp_x * tam_flecha * 0.4f,
                by - ny * tam_flecha + perp_y * tam_flecha * 0.4f);
    p3 = ImVec2(bx - nx * tam_flecha - perp_x * tam_flecha * 0.4f,
                by - ny * tam_flecha - perp_y * tam_flecha * 0.4f);
}

inline ImVec2 puntoBezierCuadratico(ImVec2 p0, ImVec2 p1, ImVec2 p2, float t) {
    float u = 1.0f - t;
    return ImVec2(
        u*u*p0.x + 2*u*t*p1.x + t*t*p2.x,
        u*u*p0.y + 2*u*t*p1.y + t*t*p2.y
    );
}

inline ImVec2 calcularPuntoControlCurva(ImVec2 o, ImVec2 d, float desplazo = 30.0f) {
    float dx = d.x - o.x;
    float dy = d.y - o.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 0.001f) return ImVec2(o.x + 30, o.y - 30);
    float perp_x = -dy / dist;
    float perp_y =  dx / dist;
    ImVec2 mid((o.x + d.x) * 0.5f, (o.y + d.y) * 0.5f);
    return ImVec2(mid.x + perp_x * desplazo, mid.y + perp_y * desplazo);
}

// Devuelve un punto sobre la arista en el parametro t (0=origen, 1=destino)
inline ImVec2 puntoEnArista(ImVec2 o, ImVec2 d, ImVec2 pc, bool es_curva, float t) {
    if (es_curva) {
        return puntoBezierCuadratico(o, pc, d, t);
    }
    return ImVec2(o.x + (d.x - o.x) * t, o.y + (d.y - o.y) * t);
}

// Dibuja una linea recta o curva segun es_curva
inline void lineaArista(ImDrawList* dl, ImVec2 o, ImVec2 d, float r_o, float r_d, ImVec2 pc, bool es_curva, ImU32 col, float grosor) {
    float dx = d.x - o.x;
    float dy = d.y - o.y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist <= (r_o + r_d + 1.0f)) return;
    
    ImVec2 o_border = o;
    ImVec2 d_border = d;
    
    if (!es_curva) {
        // Restaurar dibujo de centro a centro (sin recortar al borde)
        // Esto evita que las esquinas planas de las lineas gruesas se vean feas contra los circulos
        dl->AddLine(o, d, col, grosor);
    } else {
        // Restaurar curvas de centro a centro
        dl->PathLineTo(o);
        dl->PathBezierQuadraticCurveTo(pc, d, 20);
        dl->PathStroke(col, 0, grosor);
    }
}

// Funcion principal de dibujo
inline void dibujar(Grafo& red, Interfaz& self) {
    static int frames_init = 0;
    if (frames_init < 5) {
        ImGui::SetNextWindowFocus();
        frames_init++;
    }
    ImGui::Begin("Lienzo de Red");
    ImVec2 tamano = ImGui::GetContentRegionAvail();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float tiempo = (float)ImGui::GetTime();

    bool tooltip_mostrado = false; // Prevents overlapping tooltip flicker

    // grilla dinamica se escala y desvanece con el zoom
    float zoom_estimado = red.nodos.empty() ? 1.0f : (red.nodos[0].radio / 20.0f);
    float grid = std::max(10.0f, 25.0f * zoom_estimado);
    float off_x = fmodf(self.estado_ui.offset_lienzo.x, grid);
    float off_y = fmodf(self.estado_ui.offset_lienzo.y, grid);
    if (off_x < 0) off_x += grid;
    if (off_y < 0) off_y += grid;
    
    int alpha_grid = std::min(30, std::max(2, (int)(12 * zoom_estimado)));
    ImU32 color_grid = IM_COL32(255, 255, 255, alpha_grid);
    
    for (float x = off_x; x < tamano.x; x += grid)
        dl->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + tamano.y), color_grid);
    for (float y = off_y; y < tamano.y; y += grid)
        dl->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + tamano.x, origin.y + y), color_grid);

    // Mouse
    ImVec2 mouse = ImGui::GetMousePos();
    bool en_canvas = ImGui::IsWindowHovered();

    bool editando_g2 = (self.estado_ui.herramienta_activa == EstadoUI::CatIsomorfismo && self.estado_grafos.iso_editando_g2);
    Grafo& grafo_actual = editando_g2 ? self.estado_grafos.grafo_iso_g2 : red;

    self.estado_ui.nodo_hover = -1;
    for (auto& n : grafo_actual.nodos) {
        float screen_nx = n.posicion.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x;
        float screen_ny = n.posicion.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y;
        float screen_r  = n.radio * self.estado_ui.zoom_lienzo;
        float dx = mouse.x - screen_nx, dy = mouse.y - screen_ny;
        if (sqrtf(dx * dx + dy * dy) <= screen_r) self.estado_ui.nodo_hover = n.id;
    }

    // Click izquierdo: seleccion/arrastre + click en paquete
    bool paquete_clickeado = false;
    if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        self.estado_ui.nodo_seleccionado = self.estado_ui.nodo_hover;
        if (self.estado_ui.nodo_seleccionado != -1) {
            self.historial.capturar(grafo_actual);
            self.estado_ui.arrastrando = true;
        }
        // Detectar click en paquete modo red
        if (self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos && self.estado_redes.sim_inicializada) {
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

    // Arrastre
    if (!paquete_clickeado) {
        if (self.estado_ui.arrastrando && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Nodo* n = grafo_actual.obtenerNodo(self.estado_ui.nodo_seleccionado);
            if (n) {
                ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                if (ImGui::GetIO().KeyShift) {
                    for (auto& n_iter : grafo_actual.nodos) {
                        n_iter.posicion.x += d.x / self.estado_ui.zoom_lienzo; 
                        n_iter.posicion.y += d.y / self.estado_ui.zoom_lienzo;
                    }
                } else {
                    n->posicion.x += d.x / self.estado_ui.zoom_lienzo; 
                    n->posicion.y += d.y / self.estado_ui.zoom_lienzo;
                }
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) self.estado_ui.arrastrando = false;
    }

    // paneo
    bool panning_activo = ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
        (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && self.estado_ui.nodo_seleccionado == -1 && self.estado_ui.nodo_hover == -1);
    
    if (en_canvas && panning_activo) {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        self.estado_ui.pan_velocity.x = d.x;
        self.estado_ui.pan_velocity.y = d.y;
    }
    
    // aplicar inercia al paneo
    if (std::abs(self.estado_ui.pan_velocity.x) > 0.1f || std::abs(self.estado_ui.pan_velocity.y) > 0.1f) {
        self.estado_ui.offset_lienzo.x += self.estado_ui.pan_velocity.x; 
        self.estado_ui.offset_lienzo.y += self.estado_ui.pan_velocity.y;
        // YA NO SE MUEVEN LOS NODOS FISICAMENTE
        self.estado_ui.pan_velocity.x *= 0.85f; // friccion
        self.estado_ui.pan_velocity.y *= 0.85f;
    }

    // zoom
    float wheel = ImGui::GetIO().MouseWheel;
    if (en_canvas && wheel != 0.0f) {
        self.estado_ui.zoom_velocity += wheel * 0.1f;
    }
    
    // atajos teclado para zoom
    if (en_canvas || ImGui::IsWindowFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) self.estado_ui.zoom_velocity += 0.2f;
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) self.estado_ui.zoom_velocity -= 0.2f;
    }

    // aplicar suavizado de zoom   
    if (std::abs(self.estado_ui.zoom_velocity) > 0.001f) {
        float factor = 1.0f + self.estado_ui.zoom_velocity;
        ImVec2 centro = en_canvas ? mouse : ImVec2(origin.x + tamano.x * 0.5f, origin.y + tamano.y * 0.5f);
        
        self.estado_ui.zoom_lienzo *= factor;
        self.estado_ui.offset_lienzo.x = centro.x + (self.estado_ui.offset_lienzo.x - centro.x) * factor;
        self.estado_ui.offset_lienzo.y = centro.y + (self.estado_ui.offset_lienzo.y - centro.y) * factor;
        
        self.estado_ui.zoom_velocity *= 0.7f; 
    }

    // Físicas Fruchterman-Reingold
    if (self.estado_ui.fisicas_estado_cambiado) {
        self.estado_ui.fisicas_estado_cambiado = false;
        if (self.estado_ui.fisicas_activas) {
            // guardamos la posiciones un backup ps
            self.estado_ui.fisicas_posiciones_guardadas.clear();
            for (const auto& n : grafo_actual.nodos) {
                self.estado_ui.fisicas_posiciones_guardadas[n.id] = n.posicion;
            }
            self.registrarLog("Físicas activadas. Posiciones guardadas.");
        } else {
            // aqui restauramos posiciones
            for (auto& n : grafo_actual.nodos) {
                if (self.estado_ui.fisicas_posiciones_guardadas.count(n.id)) {
                    n.posicion = self.estado_ui.fisicas_posiciones_guardadas[n.id];
                }
            }
            self.estado_ui.fisicas_posiciones_guardadas.clear();
            self.registrarLog("Físicas desactivadas. Posiciones originales restauradas.");
        }
    }

    if (self.estado_ui.fisicas_activas) {
        // fa2: actualizar nodo siendo arrastrado para que no se mueva
        self.estado_ui.fa2.nodo_arrastrado = self.estado_ui.arrastrando
            ? self.estado_ui.nodo_seleccionado : -1;
        self.estado_ui.fa2.step(grafo_actual, self.estado_ui.fa2_params);
    }

    if (en_canvas && ImGui::IsMouseDragging(ImGuiMouseButton_Right) && self.estado_ui.nodo_hover != -1 && !self.estado_ui.creando_arista_drag) {
        self.estado_ui.creando_arista_drag = true;
        self.estado_ui.drag_arista_origen = self.estado_ui.nodo_hover;
    }

    if (self.estado_ui.creando_arista_drag) {
        Nodo* origen = grafo_actual.obtenerNodo(self.estado_ui.drag_arista_origen);
        if (origen) {
            // Fix: Usar screen coordinates para la previsualizacion de la arista
            ImVec2 screen_origen(origen->posicion.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x,
                                 origen->posicion.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y);
            dl->AddLine(screen_origen, mouse, IM_COL32(0, 255, 200, 150), 4.0f);
            dl->AddLine(screen_origen, mouse, IM_COL32(200, 255, 255, 255), 1.5f);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            if (self.estado_ui.nodo_hover != -1) {
                self.estado_ui.pendiente_arista_origen = self.estado_ui.drag_arista_origen;
                self.estado_ui.pendiente_arista_destino = self.estado_ui.nodo_hover;
                self.estado_ui.pendiente_arista_peso = 1.0f;
                self.estado_ui.pendiente_arista_dirigida = self.estado_ui.aristas_dirigidas;
                ImGui::OpenPopup("CrearArista");
            }
            self.estado_ui.creando_arista_drag = false;
            self.estado_ui.drag_arista_origen = -1;
        }
    } else if (en_canvas && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        if (self.estado_ui.nodo_hover == -1) {
            ImVec2 mouse_mundo((mouse.x - self.estado_ui.offset_lienzo.x) / self.estado_ui.zoom_lienzo, 
                               (mouse.y - self.estado_ui.offset_lienzo.y) / self.estado_ui.zoom_lienzo);
            self.estado_ui.pos_click_derecho = mouse; // Para UI se usa el real
            float current_radius = grafo_actual.nodos.empty() ? 20.0f : grafo_actual.nodos[0].radio;
            if (editando_g2) {
                self.historial.capturar(grafo_actual);
                grafo_actual.agregarNodo(mouse_mundo);
                grafo_actual.nodos.back().nombre = "U" + std::to_string(grafo_actual.nodos.back().id);
                grafo_actual.nodos.back().radio = current_radius;
                self.estado_grafos.iso_analizado = false;
            } else if (self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos) {
                ImGui::OpenPopup("CrearEquipo");
            } else {
                self.historial.capturar(grafo_actual);
                grafo_actual.agregarNodo(mouse_mundo);
                grafo_actual.nodos.back().nombre = "V" + std::to_string(grafo_actual.nodos.back().id);
                grafo_actual.nodos.back().radio = current_radius;
                self.registrarLog("Nodo creado: " + grafo_actual.nodos.back().nombre);
                self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            }
        } else if (self.estado_ui.nodo_hover != -1) {
            if (self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos && self.estado_ui.nodo_seleccionado == -1) {
                self.estado_ui.nodo_seleccionado = self.estado_ui.nodo_hover;
                ImGui::OpenPopup("MenuNodo");
            } else if (self.estado_ui.nodo_seleccionado != -1) {
                self.estado_ui.pendiente_arista_origen = self.estado_ui.nodo_seleccionado;
                self.estado_ui.pendiente_arista_destino = self.estado_ui.nodo_hover;
                self.estado_ui.pendiente_arista_peso = 1.0f;
                self.estado_ui.pendiente_arista_dirigida = self.estado_ui.aristas_dirigidas;
                ImGui::OpenPopup("CrearArista");
            } else {
                self.estado_ui.nodo_seleccionado = self.estado_ui.nodo_hover;
            }
        }
    }

    // delete
    if (en_canvas && ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().WantTextInput) {
        // Ctrl + N = Nuevo Grafo (Borrar todo)
        grafo_actual.limpiar();
        self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
        self.registrarLog("Grafo entero eliminado (Ctrl+N)");
        self.estado_ui.nodo_seleccionado = -1; self.estado_ui.arrastrando = false;
    } else if (self.estado_ui.nodo_seleccionado != -1 && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && !ImGui::GetIO().WantTextInput) {
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

    // popups
    if (ImGui::BeginPopup("CrearEquipo")) {
        ImGui::Text(ICON_FA_PLUS " Nuevo Equipamiento"); ImGui::Separator();
        static int tipo_sel = 0;
        const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
        const char* iconos[] = {ICON_FA_SERVER, ICON_FA_NETWORK_WIRED, ICON_FA_RIGHT_LEFT, ICON_FA_SHIELD_HALVED, ICON_FA_DESKTOP};
        for (int i = 0; i < 5; i++) {
            char label[64]; snprintf(label, sizeof(label), "%s %s", iconos[i], tipos[i]);
            if (ImGui::Selectable(label)) {
                self.historial.capturar(grafo_actual);
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
        bool es_bucle = (self.estado_ui.pendiente_arista_origen == self.estado_ui.pendiente_arista_destino);
        if (es_bucle) {
            self.estado_ui.pendiente_arista_dirigida = true;
            ImGui::TextDisabled(ICON_FA_ARROW_RIGHT " Bucle - dirigida forzada");
        } else {
            ImGui::Checkbox("Dirigida", &self.estado_ui.pendiente_arista_dirigida);
        }
        if (ImGui::Button(ICON_FA_CHECK " Crear", ImVec2(100, 0))) {
            self.historial.capturar(grafo_actual);
            grafo_actual.agregarArista(self.estado_ui.pendiente_arista_origen, self.estado_ui.pendiente_arista_destino, self.estado_ui.pendiente_arista_peso, self.estado_ui.pendiente_arista_dirigida);
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

    if (ImGui::BeginPopup("MenuNodo")) {
        Nodo* n = grafo_actual.obtenerNodo(self.estado_ui.nodo_seleccionado);
        if (n) {
            ImGui::TextDisabled("nodo: %s", n->nombre.c_str());
            ImGui::Separator();
            if (self.estado_redes.sim_inicializada && self.estado_redes.simulador.estado.nodos.count(n->id)) {
                auto& en = self.estado_redes.simulador.estado.nodos[n->id];
                if (en.activo) {
                    if (ImGui::Selectable(ICON_FA_BOLT " derribar nodo")) {
                        self.estado_redes.simulador.simularFalloNodo(n->id, red);
                    }
                } else {
                    if (ImGui::Selectable(ICON_FA_ROTATE_LEFT " restaurar nodo")) {
                        self.estado_redes.simulador.restaurarNodo(n->id, red);
                    }
                }
                if (ImGui::Selectable(ICON_FA_PAPER_PLANE " enviar trafico desde aqui")) {
                    self.estado_redes.flujo_origen = n->id;
                }
            }
            if (ImGui::Selectable(ICON_FA_TRASH " eliminar nodo")) {
                grafo_actual.eliminarNodo(n->id);
                self.estado_ui.nodo_seleccionado = -1;
            }
        }
        ImGui::EndPopup();
    }

    // --- TRANSFORMACION VISUAL PARA DIBUJO ---
    Grafo red_dibujo = red;
    Grafo iso_dibujo = self.estado_grafos.grafo_iso_g2;
    for(auto& n : red_dibujo.nodos) {
        n.posicion.x = n.posicion.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x;
        n.posicion.y = n.posicion.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y;
        n.radio *= self.estado_ui.zoom_lienzo;
    }
    for(auto& n : iso_dibujo.nodos) {
        n.posicion.x = n.posicion.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x;
        n.posicion.y = n.posicion.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y;
        n.radio *= self.estado_ui.zoom_lienzo;
    }

    // lista de grafos a dibujar (usando las copias visuales transformadas)
    std::vector<Grafo*> grafos_a_dibujar = {&red_dibujo};
    if (self.estado_ui.herramienta_activa == EstadoUI::CatIsomorfismo)
        grafos_a_dibujar.push_back(&iso_dibujo);

    bool modo_red = (self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos && self.estado_redes.sim_inicializada);

    if (editando_g2) {
        const char* cartel = "[ MODO ISOMORFISMO: DIBUJANDO GRAFO A COMPARAR (G2) ]";
        ImVec2 ts = ImGui::CalcTextSize(cartel);
        dl->AddRectFilled(ImVec2(origin.x + tamano.x/2 - ts.x/2 - 15, origin.y + 20),
                          ImVec2(origin.x + tamano.x/2 + ts.x/2 + 15, origin.y + 20 + ts.y + 15),
                          IM_COL32(220, 200, 30, 200), 8.0f);
        dl->AddText(ImVec2(origin.x + tamano.x/2 - ts.x/2, origin.y + 27), IM_COL32(0, 0, 0, 255), cartel);
    }

    // recorrer grafos
    for (Grafo* ptr_g : grafos_a_dibujar) {
        Grafo& g_dib = *ptr_g;
        bool es_g2 = (&g_dib == &iso_dibujo);

        // Pre-calcular aristas para busqueda O(1)
        std::unordered_set<uint64_t> edge_lookup;
        if (!es_g2) {
            for (const auto& a : g_dib.aristas) {
                edge_lookup.insert(((uint64_t)a.origen_id << 32) | (uint32_t)a.destino_id);
            }
        }

        // ====================================================
        float rank_size_min = FLT_MAX, rank_size_max = -FLT_MAX;
        float rank_color_min = FLT_MAX, rank_color_max = -FLT_MAX;
        bool usar_ranking = (!es_g2 && self.estado_ui.ranking.activo);
        
        auto getValorRank = [&](int nid, EstadoUI::EstadoRanking::Atributo attr) -> float {
            switch (attr) {
                case EstadoUI::EstadoRanking::GRADO: return (float)g_dib.gradoNodo(nid);
                default: return 0;
            }
        };

        if (usar_ranking) {
            auto& rk = self.estado_ui.ranking;
            if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                for (const auto& nn : g_dib.nodos) {
                    float v = getValorRank(nn.id, rk.atributo_size);
                    if (v < rank_size_min) rank_size_min = v;
                    if (v > rank_size_max) rank_size_max = v;
                }
            }
            if (rk.atributo_color != EstadoUI::EstadoRanking::NINGUNO) {
                for (const auto& nn : g_dib.nodos) {
                    float v = getValorRank(nn.id, rk.atributo_color);
                    if (v < rank_color_min) rank_color_min = v;
                    if (v > rank_color_max) rank_color_max = v;
                }
            }
        }

        auto get_radio_dibujo = [&](const Nodo* nd) -> float {
            if (!nd) return 0.0f;
            float r = nd->radio;
            if (usar_ranking) {
                auto& rk = self.estado_ui.ranking;
                if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                    float range = std::max(0.001f, rank_size_max - rank_size_min);
                    float val = getValorRank(nd->id, rk.atributo_size);
                    float t = (val - rank_size_min) / range;
                    if (rk.invertir_size) t = 1.0f - t;
                    t = std::max(0.0f, std::min(1.0f, t));
                    r = rk.min_size + t * (rk.max_size - rk.min_size);
                }
            }
            if ((!es_g2 && !editando_g2 && nd->id == self.estado_ui.nodo_hover) ||
                (es_g2 && editando_g2 && nd->id == self.estado_ui.nodo_hover)) {
                r += 2.0f + sinf(tiempo * 8.0f) * 1.5f;
            }
            return r;
        };


        // dibujar aristas
        for (size_t idx = 0; idx < g_dib.aristas.size(); idx++) {
            const auto& a = g_dib.aristas[idx];
            Nodo* o = g_dib.obtenerNodo(a.origen_id);
            Nodo* d = g_dib.obtenerNodo(a.destino_id);

            // Frustum Culling
            if (std::max(o->posicion.x, d->posicion.x) < origin.x - 100.0f ||
                std::min(o->posicion.x, d->posicion.x) > origin.x + tamano.x + 100.0f ||
                std::max(o->posicion.y, d->posicion.y) < origin.y - 100.0f ||
                std::min(o->posicion.y, d->posicion.y) > origin.y + tamano.y + 100.0f) {
                continue;
            }
            if (!o || !d) continue;

            // ── Self-loop: lazo Bezier simetrico arriba del nodo ─────────
            if (o == d) {
                float r = o->radio;
                float h = r * 2.8f;
                ImVec2 p0(o->posicion.x - r * 0.7f, o->posicion.y - r * 0.7f);
                ImVec2 pc1(o->posicion.x - r * 1.8f, o->posicion.y - h);
                ImVec2 pc2(o->posicion.x + r * 1.8f, o->posicion.y - h);
                ImVec2 p3(o->posicion.x + r * 0.7f, o->posicion.y - r * 0.7f);

                ImU32 col_self = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 120);
                float grosor_self = 2.0f + std::min(a.peso_actual / 10.0f, 4.0f);

                bool self_caida = false;
                float self_uso = 0.0f;
                if (modo_red) {
                    self_uso = self.estado_redes.simulador.usoArista(a.origen_id, a.destino_id);
                    auto skey = std::make_pair(a.origen_id, a.destino_id);
                    if (self.estado_redes.simulador.estado.aristas.count(skey))
                        self_caida = !self.estado_redes.simulador.estado.aristas.at(skey).activa;
                    if (self_caida) {
                        col_self = IM_COL32(255, 50, 50, 180);
                    } else if (self_uso > 0.02f) {
                        col_self = colorSaturacion(self_uso);
                        grosor_self = 2.0f + self_uso * 4.0f;
                    }
                }

                dl->AddBezierCubic(p0, pc1, pc2, p3, col_self, grosor_self);
                if (!self_caida && self_uso > 0.02f) {
                    float gi = 0.2f + self_uso * 0.5f;
                    ImU32 gc = colorSaturacion(self_uso);
                    gc = (gc & 0x00FFFFFF) | ((int)(gi * 60) << 24);
                    dl->AddBezierCubic(p0, pc1, pc2, p3, gc, grosor_self + 4.0f);
                }

                if (a.es_dirigida && !self_caida) {
                    float t1 = 0.95f, t2 = 0.99f;
                    float u1 = 1-t1, u2 = 1-t2;
                    ImVec2 prev(
                        u1*u1*u1*p0.x + 3*u1*u1*t1*pc1.x + 3*u1*t1*t1*pc2.x + t1*t1*t1*p3.x,
                        u1*u1*u1*p0.y + 3*u1*u1*t1*pc1.y + 3*u1*t1*t1*pc2.y + t1*t1*t1*p3.y);
                    ImVec2 tip(
                        u2*u2*u2*p0.x + 3*u2*u2*t2*pc1.x + 3*u2*t2*t2*pc2.x + t2*t2*t2*p3.x,
                        u2*u2*u2*p0.y + 3*u2*u2*t2*pc1.y + 3*u2*t2*t2*pc2.y + t2*t2*t2*p3.y);
                    ImVec2 fp1, fp2, fp3;
                    calcularPuntaFlecha(prev, tip, 2.0f, fp1, fp2, fp3);
                    dl->AddTriangleFilled(fp1, fp2, fp3, col_self);
                    dl->AddTriangle(fp1, fp2, fp3, IM_COL32(255, 255, 255, 80), 1.0f);
                }

                float alpha_lod = std::clamp((self.estado_ui.zoom_lienzo - 0.25f) / 0.25f, 0.0f, 1.0f);
                if (alpha_lod > 0.0f) {
                    ImVec2 peso_pos(o->posicion.x, o->posicion.y - r - h * 0.45f);
                    char peso_txt[16];
                    if (modo_red && self_uso > 0.02f)
                        snprintf(peso_txt, sizeof(peso_txt), "%.0f%%", self_uso * 100.0f);
                    else
                        snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
                    ImVec2 ts = ImGui::CalcTextSize(peso_txt);
                    dl->AddRectFilled(
                        ImVec2(peso_pos.x - ts.x*0.5f - 3, peso_pos.y - ts.y*0.5f - 1),
                        ImVec2(peso_pos.x + ts.x*0.5f + 3, peso_pos.y + ts.y*0.5f + 1),
                        IM_COL32(20, 20, 25, (int)(200 * alpha_lod)), 3.0f);
                    dl->AddText(ImVec2(peso_pos.x - ts.x*0.5f, peso_pos.y - ts.y*0.5f),
                        IM_COL32(200, 210, 220, (int)(220 * alpha_lod)), peso_txt);
                }

                continue;
            }

            ImU32 col = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 70); // Menor opacidad para mallas nítidas
            float grosor = 1.0f + std::min(a.peso_actual / 20.0f, 2.0f); // Aristas mucho más finas estilo Gephi

            auto par = std::make_pair(a.origen_id, a.destino_id);
            auto parR = std::make_pair(a.destino_id, a.origen_id);

            // ── Detectar curva (par bidireccional) ───────────────────────
            bool es_curva = false;
            ImVec2 punto_control(0, 0);
            if (!es_g2 && o != d) {
                uint64_t reverse_key = ((uint64_t)a.destino_id << 32) | (uint32_t)a.origen_id;
                if (edge_lookup.count(reverse_key)) {
                    es_curva = true;
                    float desplazo = a.es_dirigida ? 45.0f : 30.0f;
                    punto_control = calcularPuntoControlCurva(o->posicion, d->posicion, desplazo);
                }
            }

            // animacion
            if (!es_g2) {
                bool anim_activa = (self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0);
                if (anim_activa) {
                    bool check_par_c = self.estado_grafos.anim_estado.confirmadas.count(par) != 0;
                    bool check_parR_c = !a.es_dirigida && self.estado_grafos.anim_estado.confirmadas.count(parR) != 0;
                    if (check_par_c || check_parR_c) {
                        col = IM_COL32(255, 179, 0, 255); grosor = 5.0f;
                        lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                    } else {
                        bool check_par_e = self.estado_grafos.anim_estado.exploradas.count(par) != 0;
                        bool check_parR_e = !a.es_dirigida && self.estado_grafos.anim_estado.exploradas.count(parR) != 0;
                        if (check_par_e || check_parR_e) {
                            col = IM_COL32(0, 188, 212, 220); grosor = 3.5f;
                        } else {
                            bool check_par_d = self.estado_grafos.anim_estado.descartadas.count(par) != 0;
                            bool check_parR_d = !a.es_dirigida && self.estado_grafos.anim_estado.descartadas.count(parR) != 0;
                            if (check_par_d || check_parR_d) {
                                float pulse_a = (sinf(tiempo * 5.0f) + 1.0f) * 0.5f;
                                col = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60)); grosor = 2.5f;
                            }
                        }
                    }
                }
                bool mostrar_res = (!self.estado_grafos.anim_estado.activa && self.estado_grafos.anim_estado.paso_actual >= (int)self.estado_grafos.anim_estado.pasos.size() - 1);
                if (!anim_activa || mostrar_res) {
                    if (!self.estado_grafos.mostrar_mst && self.estado_grafos.ruta_optima.size() >= 2) {
                        for (size_t i = 0; i + 1 < self.estado_grafos.ruta_optima.size(); i++) {
                            if ((a.origen_id == self.estado_grafos.ruta_optima[i] && a.destino_id == self.estado_grafos.ruta_optima[i + 1]) ||
                                (!a.es_dirigida && a.origen_id == self.estado_grafos.ruta_optima[i + 1] && a.destino_id == self.estado_grafos.ruta_optima[i])) {
                                col = IM_COL32(255, 179, 0, 255); grosor = 6.0f;
                                lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(255, 255, 255, 150), grosor + 4.0f); break;
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

            // brillo de saturacion modo red
            bool modo_red_edge = (modo_red && !es_g2);
            float uso_arista = 0.0f;
            bool arista_caida = false;
            if (modo_red_edge) {
                uso_arista = self.estado_redes.simulador.usoArista(a.origen_id, a.destino_id);
                auto key = std::make_pair(a.origen_id, a.destino_id);
                if (self.estado_redes.simulador.estado.aristas.count(key))
                    arista_caida = !self.estado_redes.simulador.estado.aristas.at(key).activa;

                if (arista_caida) {
                    // arista caida
                    col = IM_COL32(255, 50, 50, 180);
                    grosor = 2.0f;
                    // dibujar linea punteada
                    int segmentos = 8;
                    for (int seg = 0; seg < segmentos; seg++) {
                        float t0 = (float)seg / segmentos;
                        float t1 = (float)(seg + 1) / segmentos;
                        ImVec2 p0 = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t0);
                        if (seg % 2 == 0) {
                            ImVec2 p1 = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t1);
                            dl->AddLine(p0, p1, col, grosor);
                        }
                    }
                } else if (uso_arista > 0.02f) {
                    // --- Fase 2: Grosor dinámico realista (capacidad + uso) ---
                    auto key_ea = std::make_pair(a.origen_id, a.destino_id);
                    float bw_factor = 1.0f;
                    if (self.estado_redes.simulador.estado.aristas.count(key_ea)) {
                        const auto& ea = self.estado_redes.simulador.estado.aristas.at(key_ea);
                        bw_factor = std::min(ea.bandwidth_mbps / 100.0f, 4.0f);
                    }
                    float grosor_fondo = 2.0f + bw_factor * 3.0f;

                    // Fondo = capacidad (linea base gris representa ancho de banda maximo)
                    lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(60, 65, 75, 100), grosor_fondo);

                    // Brillo animado
                    float glow_intensity = 0.2f + uso_arista * 0.4f;
                    ImU32 glow_col = colorSaturacion(uso_arista);
                    glow_col = (glow_col & 0x00FFFFFF) | ((int)(glow_intensity * 50) << 24);
                    lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, glow_col, grosor_fondo + 4.0f);

                    // onda viajera
                    float t_onda = fmod(tiempo * (0.5f + uso_arista * 2.0f), 1.0f);
                    ImVec2 pos_onda = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t_onda);
                    ImU32 col_onda = colorSaturacion(uso_arista);
                    dl->AddCircleFilled(pos_onda, 3.0f + uso_arista * 3.0f, col_onda, 10);

                    // Linea de uso (brillante encima)
                    col = colorSaturacion(uso_arista);
                    grosor = 1.5f + uso_arista * grosor_fondo * 0.8f;
                }

                // Si el uso es >92%, borde rojo pulsante
                if (uso_arista > 0.92f && !arista_caida) {
                    float pulse = sinf(tiempo * 4.0f) * 0.3f + 0.7f;
                    lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(255, 50, 50, (int)(80 * pulse)), grosor + 6.0f);
                }
            }

            // linea base
            if (!arista_caida) {
                if (!es_g2 && editando_g2) {
                    col = (col & 0x00FFFFFF) | (40 << 24);
                }
                lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, col, grosor);

                // Flecha para aristas dirigidas
                if (a.es_dirigida) {
                    ImVec2 fp1, fp2, fp3;
                    if (es_curva) {
                        float t_tang = 0.99f;
                        ImVec2 tan_pt = puntoBezierCuadratico(o->posicion, punto_control, d->posicion, t_tang);
                        calcularPuntaFlecha(tan_pt, d->posicion, get_radio_dibujo(d), fp1, fp2, fp3);
                    } else {
                        calcularPuntaFlecha(o->posicion, d->posicion, get_radio_dibujo(d), fp1, fp2, fp3);
                    }
                    dl->AddTriangleFilled(fp1, fp2, fp3, col);
                    dl->AddTriangle(fp1, fp2, fp3, IM_COL32(255, 255, 255, 80), 1.0f);
                }
            }

            // peso
            float alpha_lod = std::clamp((self.estado_ui.zoom_lienzo - 0.25f) / 0.25f, 0.0f, 1.0f);
            if (!arista_caida && alpha_lod > 0.0f) {
                ImVec2 mid = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, 0.5f);
                char peso_txt[16];
                if (modo_red && uso_arista > 0.02f)
                    snprintf(peso_txt, sizeof(peso_txt), "%.0f%%", uso_arista * 100.0f);
                else
                    snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
                ImVec2 ts = ImGui::CalcTextSize(peso_txt);
                dl->AddRectFilled(
                    ImVec2(mid.x - ts.x * 0.5f - 3, mid.y - ts.y * 0.5f - 1),
                    ImVec2(mid.x + ts.x * 0.5f + 3, mid.y + ts.y * 0.5f + 1),
                    IM_COL32(20, 20, 25, (int)(200 * alpha_lod)), 3.0f);
                ImU32 col_txt = (modo_red && uso_arista > 0.02f) ? colorSaturacion(uso_arista) : IM_COL32(200, 210, 220, 255);
                col_txt = (col_txt & 0x00FFFFFF) | ((int)(220 * alpha_lod) << 24);
                dl->AddText(ImVec2(mid.x - ts.x * 0.5f, mid.y - ts.y * 0.5f), col_txt, peso_txt);
            }

            // cruces
            if (!es_g2 && self.estado_grafos.mostrar_coloreo &&
                self.estado_grafos.planar_analizado &&
                self.estado_grafos.mostrar_cruces &&
                self.estado_grafos.resultado_planaridad.cruces_estimadas > 0) {
                bool cruza = false;
                for (const auto& par : self.estado_grafos.resultado_planaridad.aristas_cruce) {
                    if (par.first == (int)idx || par.second == (int)idx) { cruza = true; break; }
                }
                if (cruza) {
                    float pulse = sinf(tiempo * 3.0f + idx) * 0.3f + 0.7f;
                    ImU32 col_cruce = IM_COL32(255, 120, 0, (int)(pulse * 120));
                    lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, col_cruce, 6.0f);
                }
            }

            // x animada
            if (arista_caida) {
                ImVec2 mid2 = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, 0.5f);
                float pulse_a = (sinf(tiempo * 4.0f) + 1.0f) * 0.5f;
                float sz = 8.0f + pulse_a * 4.0f;
                ImU32 col_x = IM_COL32(255, 50, 50, (int)(150 + pulse_a * 105));
                dl->AddLine(ImVec2(mid2.x - sz, mid2.y - sz), ImVec2(mid2.x + sz, mid2.y + sz), col_x, 3.0f);
                dl->AddLine(ImVec2(mid2.x + sz, mid2.y - sz), ImVec2(mid2.x - sz, mid2.y + sz), col_x, 3.0f);
            }

            // --- Fase 3: Tooltip de arista (hover) ---
            if (modo_red && !arista_caida && !tooltip_mostrado) {
                ImVec2 mouse_edge = ImGui::GetMousePos();
                float dist_min_edge = 999.0f;
                for (int s = 0; s < 20; s++) {
                    float t_s = s / 19.0f;
                    ImVec2 pt = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, t_s);
                    float d_e = hypotf(mouse_edge.x - pt.x, mouse_edge.y - pt.y);
                    if (d_e < dist_min_edge) dist_min_edge = d_e;
                }
                if (dist_min_edge < 12.0f) {
                    auto key_tt = std::make_pair(a.origen_id, a.destino_id);
                    if (self.estado_redes.simulador.estado.aristas.count(key_tt)) {
                        const auto& ea_tt = self.estado_redes.simulador.estado.aristas.at(key_tt);
                        ImGui::BeginTooltip();
                        ImGui::Text("%s -> %s",
                            g_dib.nombreNodo(a.origen_id).c_str(),
                            g_dib.nombreNodo(a.destino_id).c_str());
                        ImGui::Text("Uso: %.0f%%", uso_arista * 100.0f);
                        ImGui::Text("BW: %.0f Mbps | Lat: %.1f ms", ea_tt.bandwidth_mbps, ea_tt.latencia_ms);
                        ImGui::Text("Jitter: %.1f ms | Perdida: %.1f%%", ea_tt.jitter_ms, ea_tt.packet_loss * 100.0f);
                        ImGui::EndTooltip();
                        tooltip_mostrado = true;
                    }
                }
            }
        }

        // dibujar paquetes
        if (modo_red && !es_g2) {
            for (const auto& pkt : self.estado_redes.simulador.obtenerPaquetes()) {
                if (pkt.paso_actual + 1 >= (int)pkt.ruta.size()) continue;
                int u = pkt.ruta[pkt.paso_actual];
                int v = pkt.ruta[pkt.paso_actual + 1];
                Nodo* no = g_dib.obtenerNodo(u);
                Nodo* nd = g_dib.obtenerNodo(v);
                if (!no || !nd) continue;

                // Verificar si esta arista es curva (existe inversa)
                bool pkt_curvo = false;
                ImVec2 pkt_pc;
                if (no != nd) {
                    for (const auto& other : g_dib.aristas) {
                        if (other.origen_id == v && other.destino_id == u) {
                            pkt_curvo = true;
                            // Si el paquete viaja en direccion inversa, invertir desplazamiento
                            float desp = (u != pkt.origen_id) ? -30.0f : 30.0f;
                            pkt_pc = calcularPuntoControlCurva(no->posicion, nd->posicion, desp);
                            break;
                        }
                    }
                }

                float t = Easing::easeInOutCubic(pkt.progreso);
                ImVec2 pos_pkt = puntoEnArista(no->posicion, nd->posicion, pkt_pc, pkt_curvo, t);

                // --- Fase 4: Render especial para TRACE ---
                if (pkt.tipo == "TRACE") {
                    float tam_trace = 6.0f + sinf(tiempo * 3.0f) * 2.0f;
                    dl->AddCircleFilled(pos_pkt, tam_trace * 6.0f,
                        IM_COL32(0, 188, 212, (int)(20 + sinf(tiempo * 2.0f) * 10)), 20);
                    dl->AddCircleFilled(pos_pkt, tam_trace, IM_COL32(0, 220, 255, 255), 20);
                    dl->AddCircleFilled(pos_pkt, tam_trace * 0.5f, IM_COL32(255, 255, 255, 200), 12);
                    dl->AddText(ImVec2(pos_pkt.x - 5, pos_pkt.y - 6), IM_COL32(255, 255, 255, 255), "T");
                } else {

                ImU32 col_pkt = imColorProtocolo(pkt.tipo);
                float tam = 2.5f + pkt.tamaño_mb * 0.3f;
                if (tam > 5.5f) tam = 5.5f;

                // estela
                for (int trail = 1; trail <= 4; trail++) {
                    float tt = std::max(0.0f, pkt.progreso - trail * 0.04f);
                    float t_trail = Easing::easeInOutCubic(tt);
                    ImVec2 pos_trail = puntoEnArista(no->posicion, nd->posicion, pkt_pc, pkt_curvo, t_trail);
                    float alpha_t = (1.0f - trail * 0.2f) * 0.6f;
                    dl->AddCircleFilled(pos_trail, tam * (1.0f - trail * 0.15f),
                        IM_COL32(
                            ((col_pkt >> IM_COL32_R_SHIFT) & 0xFF),
                            ((col_pkt >> IM_COL32_G_SHIFT) & 0xFF),
                            ((col_pkt >> IM_COL32_B_SHIFT) & 0xFF),
                            (int)(alpha_t * 255)
                        ), 10);
                }

                // halo
                dl->AddCircleFilled(pos_pkt, tam * 4.0f,
                    ((col_pkt & 0x00FFFFFF) | (18 << 24)), 14);

                // punto principal
                dl->AddCircleFilled(pos_pkt, tam, col_pkt, 20);

                // brillo central
                dl->AddCircleFilled(pos_pkt, tam * 0.35f,
                    IM_COL32(255, 255, 255, 160), 10);

                // etiqueta
                char label[2] = {ColoresProtocolo::icono(pkt.tipo)[0], '\0'};
                ImVec2 ls = ImGui::CalcTextSize(label);
                dl->AddText(ImVec2(pos_pkt.x - ls.x * 0.5f - 8, pos_pkt.y - ls.y * 0.5f - 8),
                    IM_COL32(255, 255, 255, 180), label);

                } // end else (not TRACE)

                // tooltip de inspeccion (hitbox expandido para atraparlos facil)
                if (!tooltip_mostrado && std::hypot(mouse.x - pos_pkt.x, mouse.y - pos_pkt.y) < 25.0f) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Paquete ID: #%d", pkt.id);
                    ImGui::Text("Protocolo: %s", pkt.tipo.c_str());
                    if (pkt.paso_actual + 1 < (int)pkt.ruta.size()) {
                        ImGui::Text("Ruta: %s -> %s",
                            g_dib.nombreNodo(pkt.ruta[pkt.paso_actual]).c_str(),
                            g_dib.nombreNodo(pkt.ruta[pkt.paso_actual+1]).c_str()
                        );
                    }
                    ImGui::Text("Progreso: Salto %d de %d", pkt.paso_actual + 1, (int)pkt.ruta.size());
                    ImGui::EndTooltip();
                    tooltip_mostrado = true;
                }
            }
        }
        // ====================================================
        // PRE-CALCULAR RANGOS DE RANKING PARA EVITAR O(N^2)
        // dibujar nodos
        for (auto& n : g_dib.nodos) {
            // Check if we should hide vertices in fractal mode
            if (!es_g2 && self.estado_grafos.ocultar_vertices_fractal && self.estado_ui.herramienta_activa == EstadoUI::CatFractales) {
                continue;
            }

            // Frustum Culling
            float r_cull = n.radio + 50.0f;
            if (n.posicion.x < origin.x - r_cull || n.posicion.x > origin.x + tamano.x + r_cull ||
                n.posicion.y < origin.y - r_cull || n.posicion.y > origin.y + tamano.y + r_cull) {
                continue; // no dibujar si esta fuera de pantalla
            }

            ImU32 colorFondo, colorBorde;
            bool es_anim = (!es_g2 && (self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0));

            // color del nodo
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
            } else if (self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos) {
                colorFondo = IM_COL32(30, 35, 45, 255); // Oscuro en vez de blanco
                switch (n.tipo) {
                    case TipoHardware::Servidor: colorBorde = IM_COL32(41, 128, 185, 255); break;
                    case TipoHardware::Router:   colorBorde = IM_COL32(211, 84, 0, 255); break;
                    case TipoHardware::Switch:   colorBorde = IM_COL32(39, 174, 96, 255); break;
                    case TipoHardware::Firewall: colorBorde = IM_COL32(192, 57, 43, 255); break;
                    case TipoHardware::Terminal: colorBorde = IM_COL32(142, 68, 173, 255); break;
                    default: colorBorde = IM_COL32(149, 165, 166, 255); break;
                }
            } else {
                colorFondo = IM_COL32(0, 160, 130, 255);
                colorBorde = IM_COL32(200, 220, 230, 200);
            }

            // anillo de salud
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

            // --- Fase 5: Anillo de buffer (congestion) ---
            if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
                const auto& en_buf = self.estado_redes.simulador.estado.nodos.at(n.id);
                if (en_buf.activo) {
                    float buf_ratio = std::min(1.0f, en_buf.buffer_mb / std::max(0.1f, en_buf.buffer_max_mb));
                    if (buf_ratio > 0.02f) {
                        float r_inner = n.radio * 0.65f;
                        ImU32 col_buf = IM_COL32(
                            (int)(buf_ratio * 255),
                            (int)((1.0f - buf_ratio) * 180),
                            (int)((1.0f - buf_ratio) * 100), 180);
                        dl->AddCircle(n.posicion, r_inner, col_buf, 24, 2.0f + buf_ratio * 3.0f);
                    }
                }
            }
            // --- Indicador de cola de paquetes ---
            if (modo_red && self.estado_redes.simulador.estado.nodos.count(n.id) && !es_g2) {
                const auto& en_q = self.estado_redes.simulador.estado.nodos.at(n.id);
                if (en_q.paquetes_cola > 0) {
                    char qtxt[8];
                    snprintf(qtxt, sizeof(qtxt), "%d", en_q.paquetes_cola);
                    ImVec2 qs = ImGui::CalcTextSize(qtxt);
                    dl->AddText(ImVec2(n.posicion.x - qs.x * 0.5f,
                                       n.posicion.y + n.radio + 14),
                                IM_COL32(255, 200, 100, 200), qtxt);
                }
            }

            // seleccion / hover
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

            // arbol
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


            // efecto pop al visitar
            if (es_anim && self.estado_grafos.anim_estado.visitados.count(n.id) &&
                self.estado_grafos.anim_estado.tiempo_visita_nodo.count(n.id)) {
                float t_since = tiempo - self.estado_grafos.anim_estado.tiempo_visita_nodo[n.id];
                if (t_since < 0.4f) {
                    float scale = 1.0f + Easing::easeOutBounce(t_since / 0.4f) * 0.3f;
                    dl->AddCircleFilled(n.posicion, n.radio * scale, IM_COL32(0, 230, 120, 80), 32);
                }
            }

            // radio dinamico con ranking visual aplicado
            float radio_dibujo = n.radio;
            ImU32 colorRanking = colorFondo;

            if (usar_ranking) {
                auto& rk = self.estado_ui.ranking;
                
                // Aplicar Tamaño
                if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                    float range = std::max(0.001f, rank_size_max - rank_size_min);
                    float val = getValorRank(n.id, rk.atributo_size);
                    float t = (val - rank_size_min) / range;
                    if (rk.invertir_size) t = 1.0f - t;
                    t = std::max(0.0f, std::min(1.0f, t));
                    radio_dibujo = rk.min_size + t * (rk.max_size - rk.min_size);
                }

                // Aplicar Color
                if (rk.atributo_color != EstadoUI::EstadoRanking::NINGUNO) {
                    float range = std::max(0.001f, rank_color_max - rank_color_min);
                    float val = getValorRank(n.id, rk.atributo_color);
                    float t = (val - rank_color_min) / range;
                    if (rk.invertir_color) t = 1.0f - t;
                    t = std::max(0.0f, std::min(1.0f, t));
                    // azul (50,100,255) -> rojo (255,50,50)
                    colorRanking = IM_COL32(
                        (int)(50 + t * 205),
                        (int)(100 * (1.0f - t)),
                        (int)(255 * (1.0f - t)),
                        255);
                    colorFondo = colorRanking;
                }
            }

            if ((!es_g2 && !editando_g2 && n.id == self.estado_ui.nodo_hover) ||
                (es_g2 && editando_g2 && n.id == self.estado_ui.nodo_hover)) {
                radio_dibujo += 2.0f + sinf(tiempo * 8.0f) * 1.5f;
            }

            if (!es_g2 && editando_g2) {
                colorFondo = (colorFondo & 0x00FFFFFF) | (40 << 24);
                colorBorde = (colorBorde & 0x00FFFFFF) | (40 << 24);
            }

            // circulo del nodo
            dl->AddCircleFilled(n.posicion, radio_dibujo, colorFondo, 32);
            dl->AddCircle(n.posicion, radio_dibujo, colorBorde, 32, 2.5f);

            // pulso fractal: anillo brillante cuando el modo fractal esta activo
            if (self.estado_grafos.mostrar_coloreo && self.estado_grafos.modo_fractal && !es_g2) {
                float fp = sinf(tiempo * 2.0f + n.id * 1.7f) * 0.3f + 0.7f;
                dl->AddCircle(n.posicion, radio_dibujo + 6 + fp * 4, IM_COL32(150, 60, 200, (int)(fp * 120)), 32, 1.5f);
            }

            // texto del nodo (LOD con Alpha Blending)
            float alpha_lod = 1.0f; // std::clamp((self.estado_ui.zoom_lienzo - 0.25f) / 0.25f, 0.0f, 1.0f);
            if (alpha_lod > 0.0f) {
                int alpha_base = (!es_g2 && editando_g2) ? 60 : 255;
                int alpha_texto = (int)(alpha_base * alpha_lod);
                
                if (!es_g2 && self.estado_ui.modo_actual == Interfaz::ModoApp::AeroGrafos) {
                    const char* icono = iconoHardware(n.tipo);
                    ImVec2 is = ImGui::CalcTextSize(icono);
                    
                    ImU32 iconColor = (colorBorde & 0x00FFFFFF) | (alpha_texto << 24);
                    dl->AddText(ImVec2(n.posicion.x - is.x * 0.5f, n.posicion.y - is.y * 0.5f), iconColor, icono);
                    
                    ImVec2 ns = ImGui::CalcTextSize(n.nombre.c_str());
                    int sub_alpha = (int)(((!es_g2 && editando_g2) ? 40 : 200) * alpha_lod);
                    dl->AddText(ImVec2(n.posicion.x - ns.x * 0.5f, n.posicion.y + radio_dibujo + 3), IM_COL32(190, 195, 200, sub_alpha), n.nombre.c_str());
                } else {
                    ImVec2 ts = ImGui::CalcTextSize(n.nombre.c_str());
                    dl->AddText(ImVec2(n.posicion.x - ts.x * 0.5f, n.posicion.y - ts.y * 0.5f), IM_COL32(255, 255, 255, alpha_texto), n.nombre.c_str());
                }
            }

            // nodo caido
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

            // tooltip en modo grafos: muestra metricas basicas
            if (!modo_red && !es_g2 && n.id == self.estado_ui.nodo_hover
                && !tooltip_mostrado) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", n.nombre.c_str());
                ImGui::Text("Grado: %d", g_dib.gradoNodo(n.id));
                ImGui::EndTooltip();
                tooltip_mostrado = true;
            }

            // tooltip en modo red con metricas de simulacion
            if (modo_red && n.id == self.estado_ui.nodo_hover && !es_g2 &&
                self.estado_redes.simulador.estado.nodos.count(n.id) && !tooltip_mostrado) {
                const auto& en = self.estado_redes.simulador.estado.nodos.at(n.id);
                char tooltip[256];
                snprintf(tooltip, sizeof(tooltip),
                    "CPU: %.0f%%  RAM: %.0f%%\nRX: %.0f  TX: %.0f\n%s",
                    en.cpu_uso * 100.0f, en.memoria_uso * 100.0f,
                    en.paquetes_rx, en.paquetes_tx,
                    en.activo ? "✅ Activo" : "❌ CAIDO");
                ImGui::SetTooltip("%s", tooltip);
                tooltip_mostrado = true;
            }
        }

        // particula de animacion
        if ((self.estado_grafos.anim_estado.activa || self.estado_grafos.anim_estado.paso_actual >= 0) && self.estado_grafos.anim_estado.particula.activa && !es_g2) {
            float t_ease = Easing::easeInOutCubic(self.estado_grafos.anim_estado.particula.progreso);
            // Verificar curva para la particula usando datos del paso actual
            ImVec2 p_inicio_raw = self.estado_grafos.anim_estado.particula.pos_inicio;
            ImVec2 p_fin_raw = self.estado_grafos.anim_estado.particula.pos_fin;
            ImVec2 p_inicio(
                p_inicio_raw.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x,
                p_inicio_raw.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y
            );
            ImVec2 p_fin(
                p_fin_raw.x * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.x,
                p_fin_raw.y * self.estado_ui.zoom_lienzo + self.estado_ui.offset_lienzo.y
            );
            bool p_curvo = false;
            ImVec2 p_pc;
            int paso_cur = self.estado_grafos.anim_estado.paso_actual;
            if (paso_cur >= 0 && paso_cur < (int)self.estado_grafos.anim_estado.pasos.size()) {
                int a_o = self.estado_grafos.anim_estado.pasos[paso_cur].arista_origen;
                int a_d = self.estado_grafos.anim_estado.pasos[paso_cur].arista_destino;
                if (a_o >= 0 && a_d >= 0 && a_o != a_d) {
                    for (const auto& other : g_dib.aristas) {
                        if (other.origen_id == a_d && other.destino_id == a_o) {
                            p_curvo = true;
                            p_pc = calcularPuntoControlCurva(p_inicio, p_fin);
                            break;
                        }
                    }
                }
            }
            ImVec2 pos = puntoEnArista(p_inicio, p_fin, p_pc, p_curvo, t_ease);
            float halo_r = self.estado_grafos.anim_estado.particula.radio * 2.5f * (1.0f - self.estado_grafos.anim_estado.particula.progreso * 0.5f);
            dl->AddCircleFilled(pos, halo_r, (self.estado_grafos.anim_estado.particula.color & 0x00FFFFFF) | (60 << 24), 24);
            dl->AddCircleFilled(pos, self.estado_grafos.anim_estado.particula.radio, self.estado_grafos.anim_estado.particula.color, 24);
            dl->AddCircleFilled(pos, self.estado_grafos.anim_estado.particula.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
        }
        }

    // --- Fase 1: Notificaciones emergentes en canvas ---
    {
        float now = (float)ImGui::GetTime();
        auto& notifs = self.estado_redes.simulador.estado.notificaciones;

        for (size_t i = 0; i < notifs.size(); ) {
            float edad = now - notifs[i].tiempo_real;
            if (edad > notifs[i].duracion) {
                notifs.erase(notifs.begin() + (int)i);
                continue;
            }
            float alpha = 1.0f;
            if (edad > notifs[i].duracion - 0.8f)
                alpha = (notifs[i].duracion - edad) / 0.8f;

            ImVec2 pos_n(origin.x + tamano.x * 0.5f, origin.y + 40.0f + i * 32.0f);
            ImVec2 ts_n = ImGui::CalcTextSize(notifs[i].mensaje.c_str());

            // Fondo translucido
            dl->AddRectFilled(
                ImVec2(pos_n.x - ts_n.x * 0.5f - 12, pos_n.y - 6),
                ImVec2(pos_n.x + ts_n.x * 0.5f + 12, pos_n.y + ts_n.y + 6),
                IM_COL32(10, 10, 15, (int)(180 * alpha)), 6.0f);

            // Texto
            uint32_t col_n = notifs[i].color;
            ImU32 col_final = IM_COL32(
                ((col_n >> 16) & 0xFF), ((col_n >> 8) & 0xFF), (col_n & 0xFF),
                (int)(255 * alpha));
            dl->AddText(ImVec2(pos_n.x - ts_n.x * 0.5f, pos_n.y), col_final, notifs[i].mensaje.c_str());
            i++;
        }
    }

    //overlays de resultados de algoritmo en el lienzo
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
                snprintf(buf, sizeof(buf), "coloreo fractal (colores dinamicos) | colores=%d",
                    self.estado_grafos.resultado_coloreo.num_colores);
                dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_FAN);
                dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
                oy += 22;
                dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                    "los nodos cambian de color con el tiempo segun su posicion fractal");
            } else {
                snprintf(buf, sizeof(buf), "coloreo clasico | colores usados: %d (greedy), %d (welsh-powell)",
                    self.estado_grafos.resultado_coloreo.num_colores,
                    self.estado_grafos.resultado_welsh_powell.num_colores);
                dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_PAINTBRUSH);
                dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
                oy += 22;
                dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                    "dos nodos conectados nunca comparten el mismo color");
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

    // controles de zoom flotantes
    ImVec2 pos_ventana = ImGui::GetWindowPos();
    ImVec2 tam_ventana = ImGui::GetWindowSize();
    ImVec2 zoom_pos(pos_ventana.x + tam_ventana.x - 120, pos_ventana.y + tam_ventana.y - 60);

    // dibujar el pill shape manuelamente
    ImDrawList* dl_bg = ImGui::GetWindowDrawList();
    dl_bg->AddRectFilled(zoom_pos, ImVec2(zoom_pos.x + 100, zoom_pos.y + 32), IM_COL32(30, 30, 40, 200), 16.0f);
    dl_bg->AddRect(zoom_pos, ImVec2(zoom_pos.x + 100, zoom_pos.y + 32), IM_COL32(80, 80, 90, 150), 16.0f, 0, 1.5f);
    
    ImGui::SetNextWindowPos(zoom_pos);
    if (ImGui::BeginChild("zoom_bar", ImVec2(100, 32), false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 40));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 80));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f); // Botones redondos al hacer hover

        // centrado
        ImGui::SetCursorPos(ImVec2(10, 4));
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_MINUS, ImVec2(24, 24))) self.estado_ui.zoom_velocity -= 0.3f;
        ImGui::SameLine(0, 4);
        if (ImGui::Button(ICON_FA_HOUSE, ImVec2(24, 24))) {
            float initial_radius = 20.0f;
            for (auto& n : red.nodos) n.radio = initial_radius;
            for (auto& n : self.estado_grafos.grafo_iso_g2.nodos) n.radio = initial_radius;
            self.estado_ui.offset_lienzo = ImVec2(0, 0);
            self.estado_ui.zoom_velocity = 0.0f;
            self.estado_ui.zoom_lienzo = 1.0f;
        }
        ImGui::SameLine(0, 4);
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS_PLUS, ImVec2(24, 24))) self.estado_ui.zoom_velocity += 0.3f;
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }
    ImGui::EndChild();

    ImGui::End();
}

} 