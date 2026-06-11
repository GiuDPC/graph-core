#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "nucleo/Grafo.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include "nucleo/tipos/TipoHardware.h"
#include "nucleo/algoritmos/Dijkstra.h"
#include "nucleo/algoritmos/Kruskal.h"
#include "nucleo/algoritmos/BFS.h"
#include "nucleo/algoritmos/DFS.h"
#include "nucleo/algoritmos/Ciclos.h"
#include "nucleo/algoritmos/Coloreo.h"
#include "nucleo/algoritmos/Isomorfismo.h"
#include "nucleo/algoritmos/Arbol.h"
#include "nucleo/tipos/EstadoRed.h"
#include "nucleo/SimuladorRed.h"
#include "nucleo/Topologias.h"
#include "persistencia/SerializadorJSON.h"
#include "IconsFontAwesome6.h"
#include "portable-file-dialogs.h"
#include "audio/Sonidos.h"
#include "interfaz/util/Easing.h"
#include "interfaz/util/Animacion.h"
#include "interfaz/util/Colores.h"
#include <cmath>
#include <set>
#include <cstdio>
#include <GLFW/glfw3.h>

extern Sonidos g_sonidos;

// Clase interfaz
class Interfaz {
public:
    // Estado de navegacion del panel de herramientas
    enum class ModoPanel {
        General,
        Dijkstra,
        Kruskal,
        BFS,
        DFS,
        Ciclos,
        Coloreo,
        Isomorfismo,
        Arbol       // Fase 3
    };
    ModoPanel modo_panel = ModoPanel::General;

    // --- Modo de aplicacion ---
    enum class ModoApp { Grafos, Redes };
    ModoApp modo_actual = ModoApp::Grafos;

    bool               dijkstra_usar_latencia = false;
    std::vector<float> dijkstra_tabla_dist;   // se llena durante la animacion

    int                         bfs_nodo_inicio = 0;
    Algoritmos::BFS::ResultadoBFS bfs_resultado;

    int                          dfs_nodo_inicio = 0;
    Algoritmos::DFS::ResultadoDFS dfs_resultado;

    Algoritmos::ResultadoCiclos resultado_ciclos;

    Algoritmos::ResultadoColoreo resultado_coloreo;

    Grafo                                       grafo_iso_g2;
    Algoritmos::Isomorfismo::ResultadoIsomorfismo resultado_iso;
    bool                                        iso_analizado  = false;
    bool                                        iso_editando_g2 = false;  // true = editar G2

    void resetGrafoIsomorfismo() {
        grafo_iso_g2.limpiar();
        resultado_iso = {};
        iso_analizado  = false;
        iso_editando_g2 = false;
    }

    // --- Arbol ---
    Algoritmos::Arbol::PropiedadesArbol arbol_props;
    bool        arbol_analizado       = false;
    int         arbol_raiz_id         = 0;
    bool        arbol_layout_aplicado = false;

    // --- Simulador ---
    SimuladorRed simulador;
    bool         sim_inicializada = false;
    bool         mostrar_modal_inicio = false;

    // Flujo pendiente
    int          flujo_origen  = 0;
    int          flujo_destino = 1;
    float        flujo_mbps    = 10.0f;
    int          flujo_tipo    = 0;  // 0=HTTP, 1=VIDEO, 2=PING, 3=DDoS
    float        flujo_dur     = 10.0f;

    // Nodo/arista seleccionado para fallo
    int          fallo_nodo_id     = -1;
    int          fallo_arista_org  = -1;
    int          fallo_arista_dst  = -1;

    // --- Seleccion ---
    int nodo_seleccionado = -1;
    int nodo_hover = -1;
    bool arrastrando = false;

    // --- Algoritmos instantaneos ---
    int dijkstra_origen = 0;
    int dijkstra_destino = 1;
    std::vector<int> ruta_optima;
    std::vector<Arista> aristas_mst;
    bool mostrar_mst = false;
    std::vector<int> colores_nodos;
    bool mostrar_coloreo = false;
    bool tiene_ciclo = false;
    bool ciclo_analizado = false;

    // --- Animacion paso a paso ---
    Animacion::EstadoAnimacion anim_estado;

    // --- Simulacion ---
    bool simulacion_jitter = false;
    float jitter_porcentaje = 0.15f;

    // --- Logs ---
    std::vector<std::string> system_logs;

    // --- Creacion pendiente ---
    ImVec2 pos_click_derecho = ImVec2(0, 0);
    int pendiente_arista_origen = -1;
    int pendiente_arista_destino = -1;
    float pendiente_arista_peso = 1.0f;
    char buffer_nombre[64] = {};

    // --- Fuentes ---
    ImFont* fontMono = nullptr;

    // --- Panning ---
    ImVec2 offset_lienzo = ImVec2(0, 0);

    // FUNCION PRINCIPAL (declaracion — cuerpo definido al final del archivo)
    void dibujar(Grafo& red, GLFWwindow* ventana);

    // ── Utilidades publicas (necesarias para modulos externos) ──────────────
    void registrarLog(const std::string& msg) {
        system_logs.push_back("[SYS] " + msg);
        if (system_logs.size() > 100) system_logs.erase(system_logs.begin());
    }

    const char* iconoHardware(TipoHardware tipo) {
        switch (tipo) {
            case TipoHardware::Servidor: return ICON_FA_SERVER;
            case TipoHardware::Router:   return ICON_FA_NETWORK_WIRED;
            case TipoHardware::Switch:   return ICON_FA_RIGHT_LEFT;
            case TipoHardware::Firewall: return ICON_FA_SHIELD_HALVED;
            case TipoHardware::Terminal: return ICON_FA_DESKTOP;
            default: return ICON_FA_CIRCLE;
        }
    }

    // ── Wrappers de animacion (delegan a Animacion:: + efectos secundarios) ─
    void iniciarAnimacion(std::vector<PasoAnimacion> pasos) {
        Animacion::iniciar(anim_estado, std::move(pasos));
        if (!anim_estado.pasos.empty()) {
            registrarLog("Animacion iniciada: " + std::to_string(anim_estado.pasos.size()) + " pasos");
            g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
        }
    }

    void aplicarPaso(const PasoAnimacion& p) {
        Animacion::aplicarPaso(anim_estado, p);
        switch (p.accion) {
            case PasoAnimacion::VISITAR:   g_sonidos.reproducir(Sonidos::VISITAR_NODO); break;
            case PasoAnimacion::CONFIRMAR: g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA); break;
            case PasoAnimacion::EXPLORAR:  g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO); break;
            case PasoAnimacion::DESCARTAR: g_sonidos.reproducir(Sonidos::DESCARTAR); break;
        }
        if (!p.descripcion.empty()) registrarLog(p.descripcion);
    }

    void resetAnimacion() {
        Animacion::reset(anim_estado);
    }

        // LIENZO DE RED
        void dibujarLienzo(Grafo& red) {
        ImGui::Begin("Lienzo de Red");
        ImVec2 tamano = ImGui::GetContentRegionAvail();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos();

        // Grilla sutil (con offset de panning)
        float grid = 25.0f;
        float off_x = fmodf(offset_lienzo.x, grid);
        float off_y = fmodf(offset_lienzo.y, grid);
        if (off_x < 0) off_x += grid;
        if (off_y < 0) off_y += grid;
        for (float x = off_x; x < tamano.x; x += grid)
            dl->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + tamano.y), IM_COL32(255, 255, 255, 12));
        for (float y = off_y; y < tamano.y; y += grid)
            dl->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + tamano.x, origin.y + y), IM_COL32(255, 255, 255, 12));

        // Mouse
        ImVec2 mouse = ImGui::GetMousePos();
        bool en_canvas = ImGui::IsWindowHovered();

        bool editando_g2 = (modo_panel == ModoPanel::Isomorfismo && iso_editando_g2);
        Grafo& grafo_actual = editando_g2 ? grafo_iso_g2 : red;

        nodo_hover = -1;
        for (auto& n : grafo_actual.nodos) {
            float dx = mouse.x - n.posicion.x, dy = mouse.y - n.posicion.y;
            if (sqrtf(dx * dx + dy * dy) <= n.radio) nodo_hover = n.id;
        }

        // Click izquierdo — seleccion y arrastre
        if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            nodo_seleccionado = nodo_hover;
            if (nodo_seleccionado != -1) arrastrando = true;
        }
        if (arrastrando && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Nodo* n = grafo_actual.obtenerNodo(nodo_seleccionado);
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
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) arrastrando = false;

        // Panning (Arrastre con boton medio o boton izquierdo en el fondo)
        bool panning_activo = ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || 
                              (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && nodo_seleccionado == -1 && nodo_hover == -1);
        if (en_canvas && panning_activo) {
            ImVec2 d = ImGui::GetIO().MouseDelta;
            offset_lienzo.x += d.x;
            offset_lienzo.y += d.y;
            for (auto& n : red.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
            for (auto& n : grafo_iso_g2.nodos) { n.posicion.x += d.x; n.posicion.y += d.y; }
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
            for (auto& n : grafo_iso_g2.nodos) {
                n.posicion.x = mouse.x + (n.posicion.x - mouse.x) * factor;
                n.posicion.y = mouse.y + (n.posicion.y - mouse.y) * factor;
                n.radio = std::max(5.0f, std::min(60.0f, n.radio * factor));
            }
            offset_lienzo.x = mouse.x + (offset_lienzo.x - mouse.x) * factor;
            offset_lienzo.y = mouse.y + (offset_lienzo.y - mouse.y) * factor;
        }

        // Click derecho — crear nodo o arista
        if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            if (nodo_hover == -1) {
                pos_click_derecho = mouse;
                if (editando_g2) {
                    grafo_actual.agregarNodo(mouse);
                    grafo_actual.nodos.back().nombre = "U" + std::to_string(grafo_actual.nodos.back().id);
                    iso_analizado = false;
                } else if (modo_actual == ModoApp::Redes) {
                    ImGui::OpenPopup("CrearEquipo");
                } else {
                    grafo_actual.agregarNodo(mouse);
                    grafo_actual.nodos.back().nombre = "V" + std::to_string(grafo_actual.nodos.back().id);
                    registrarLog("Nodo creado: " + grafo_actual.nodos.back().nombre);
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                }
            } else if (nodo_seleccionado != -1 && nodo_seleccionado != nodo_hover) {
                pendiente_arista_origen = nodo_seleccionado;
                pendiente_arista_destino = nodo_hover;
                pendiente_arista_peso = 1.0f;
                ImGui::OpenPopup("CrearArista");
            }
        }

        // Delete
        if (nodo_seleccionado != -1 && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && !ImGui::GetIO().WantTextInput) {
            if (editando_g2) {
                grafo_actual.eliminarNodo(nodo_seleccionado);
                iso_analizado = false;
            } else {
                registrarLog("Nodo eliminado: " + grafo_actual.nombreNodo(nodo_seleccionado));
                grafo_actual.eliminarNodo(nodo_seleccionado);
                ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
            }
            nodo_seleccionado = -1; arrastrando = false;
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
                    grafo_actual.agregarNodo(pos_click_derecho, (TipoHardware)i);
                    registrarLog("Hardware desplegado: " + std::string(tipos[i]) + " " + grafo_actual.nodos.back().nombre);
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                }
            }
            ImGui::EndPopup();
        }

        // --- Popup: Crear Arista con peso ---
        if (ImGui::BeginPopup("CrearArista")) {
            ImGui::Text(ICON_FA_LINK " Nueva Conexion");
            ImGui::Text("%s -> %s", grafo_actual.nombreNodo(pendiente_arista_origen).c_str(),
                        grafo_actual.nombreNodo(pendiente_arista_destino).c_str());
            ImGui::Separator();
            ImGui::InputFloat("Peso / Latencia", &pendiente_arista_peso, 0.5f, 5.0f, "%.1f");
            if (pendiente_arista_peso < 0.1f) pendiente_arista_peso = 0.1f;

            if (ImGui::Button(ICON_FA_CHECK " Crear", ImVec2(100, 0))) {
                if (editando_g2) {
                    grafo_actual.agregarArista(pendiente_arista_origen, pendiente_arista_destino, pendiente_arista_peso);
                    iso_analizado = false;
                } else {
                    grafo_actual.agregarArista(pendiente_arista_origen, pendiente_arista_destino, pendiente_arista_peso);
                    registrarLog("Arista creada: " + grafo_actual.nombreNodo(pendiente_arista_origen) + " - " +
                        grafo_actual.nombreNodo(pendiente_arista_destino) + " (peso=" + std::to_string((int)pendiente_arista_peso) + ")");
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK " Cancelar", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        std::vector<Grafo*> grafos_a_dibujar = {&red};
        if (modo_panel == ModoPanel::Isomorfismo) {
            grafos_a_dibujar.push_back(&grafo_iso_g2);
        }

        for (Grafo* ptr_g : grafos_a_dibujar) {
            Grafo& g_dib = *ptr_g;
            bool es_g2 = (&g_dib == &grafo_iso_g2);

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
                    bool anim_activa_o_finalizada = (anim_estado.activa || anim_estado.paso_actual >= 0);
                    
                    if (anim_activa_o_finalizada) {
                        auto parR = std::make_pair(a.destino_id, a.origen_id);

                        if (anim_estado.confirmadas.count(par) || anim_estado.confirmadas.count({a.origen_id, a.destino_id}) ||
                            anim_estado.confirmadas.count(parR)) {
                            // Arista confirmada: dorada con brillo
                            col    = IM_COL32(255, 179, 0, 255);
                            grosor = 5.0f;
                            // Agregar linea de brillo encima
                            dl->AddLine(o->posicion, d->posicion,
                                IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                        } else if (anim_estado.exploradas.count(par) || anim_estado.exploradas.count({a.origen_id, a.destino_id}) ||
                                   anim_estado.exploradas.count(parR)) {
                            col    = IM_COL32(0, 188, 212, 220);
                            grosor = 3.5f;
                        } else if (anim_estado.descartadas.count(par) || anim_estado.descartadas.count({a.origen_id, a.destino_id}) ||
                                   anim_estado.descartadas.count(parR)) {
                            // Arista descartada: roja pulsante
                            float pulse_a = (sinf((float)ImGui::GetTime() * 5.0f) + 1.0f) * 0.5f;
                            col    = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60));
                            grosor = 2.5f;
                        }
                    } 
                    
                    // Mostrar resultados finales siempre que no este corriendo la animacion activamente
                    // o cuando haya terminado.
                    bool mostrar_resultados_finales = (!anim_estado.activa && anim_estado.paso_actual >= (int)anim_estado.pasos.size() - 1);
                    
                    if (!anim_activa_o_finalizada || mostrar_resultados_finales) {
                        // Dijkstra highlight
                        if (!mostrar_mst && ruta_optima.size() >= 2) {
                            for (size_t i = 0; i + 1 < ruta_optima.size(); i++) {
                                if ((a.origen_id == ruta_optima[i] && a.destino_id == ruta_optima[i + 1]) ||
                                    (!a.es_dirigida && a.origen_id == ruta_optima[i + 1] && a.destino_id == ruta_optima[i])) {
                                    col = IM_COL32(255, 179, 0, 255); 
                                    grosor = 6.0f; 
                                    // Resaltar fuerte al final
                                    dl->AddLine(o->posicion, d->posicion, IM_COL32(255, 255, 255, 150), grosor + 4.0f);
                                    break;
                                }
                            }
                        }
                        // Kruskal highlight
                        if (mostrar_mst) {
                            for (const auto& m : aristas_mst) {
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
                if (modo_actual == ModoApp::Redes && sim_inicializada && !es_g2) {
                    float uso_arista = simulador.usoArista(a.origen_id, a.destino_id);
                    if (uso_arista > 0.05f) {
                        // Linea punteada animada sobre la arista
                        float tiempo = (float)ImGui::GetTime() * 2.0f;
                        ImU32 col_flow = simulador.colorUsoArista(uso_arista);
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
            if (modo_actual == ModoApp::Redes && sim_inicializada) {
                for (const auto& a : red.aristas) {
                    Nodo* o = red.obtenerNodo(a.origen_id);
                    Nodo* d = red.obtenerNodo(a.destino_id);
                    if (!o || !d) continue;

                    float uso = simulador.usoArista(a.origen_id, a.destino_id);
                    if (uso > 0.05f) {
                        // Dibujar barra de uso sobre la arista
                        ImVec2 mid((o->posicion.x + d->posicion.x) * 0.5f,
                                   (o->posicion.y + d->posicion.y) * 0.5f);
                        ImU32 col_uso = simulador.colorUsoArista(uso);

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
                    if (simulador.estado.aristas.count(key) &&
                        !simulador.estado.aristas.at(key).activa) {
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
                bool es_anim = (!es_g2 && (anim_estado.activa || anim_estado.paso_actual >= 0));

                if (es_g2) {
                    colorFondo = IM_COL32(180, 180, 0, 255);
                    colorBorde = IM_COL32(255, 255, 100, 255);
                } else if (es_anim && anim_estado.procesando.count(n.id)) {
                    colorFondo = IM_COL32(255, 215, 0, 200);
                    colorBorde = IM_COL32(255, 235, 100, 255);
                } else if (es_anim && anim_estado.visitados.count(n.id)) {
                    colorFondo = IM_COL32(0, 230, 118, 200);
                    colorBorde = IM_COL32(100, 255, 180, 255);
                } else if (mostrar_coloreo && (int)colores_nodos.size() > n.id && colores_nodos[n.id] != -1) {
                    ImU32 paleta[] = {
                        IM_COL32(230, 60, 60, 255), IM_COL32(60, 200, 80, 255),
                        IM_COL32(60, 100, 230, 255), IM_COL32(230, 200, 50, 255),
                        IM_COL32(200, 60, 200, 255), IM_COL32(60, 200, 200, 255)
                    };
                    colorFondo = paleta[colores_nodos[n.id] % 6];
                    colorBorde = IM_COL32(255, 255, 255, 180);
                } else if (modo_actual == ModoApp::Redes) {
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

                if ((es_g2 && editando_g2 && n.id == nodo_seleccionado) ||
                    (!es_g2 && !editando_g2 && n.id == nodo_seleccionado)) {
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
                if ((es_g2 && editando_g2 && n.id == nodo_hover && n.id != nodo_seleccionado) ||
                    (!es_g2 && !editando_g2 && n.id == nodo_hover && n.id != nodo_seleccionado)) {
                    dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(0, 255, 200, 30), 32);
                }

                // Highlight de arbol (solo para G1)
                if (!es_g2 && arbol_analizado) {
                    if (n.id == arbol_props.raiz_id) {
                        dl->AddCircleFilled(n.posicion, n.radio + 6, IM_COL32(255, 200, 0, 60), 32);
                    }
                    auto& hojas_ref = arbol_props.hojas;
                    if (std::find(hojas_ref.begin(), hojas_ref.end(), n.id) != hojas_ref.end()) {
                        dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(50, 200, 50, 50), 32);
                    }
                    if (arbol_props.nivel.count(n.id)) {
                        char nivel_txt[8];
                        snprintf(nivel_txt, sizeof(nivel_txt), "L%d", arbol_props.nivel.at(n.id));
                        ImVec2 lt = ImGui::CalcTextSize(nivel_txt);
                        dl->AddText(
                            ImVec2(n.posicion.x - lt.x * 0.5f, n.posicion.y - n.radio - 18),
                            IM_COL32(180, 220, 180, 200), nivel_txt);
                    }
                }

                // Efecto de "pop" al ser visitado por primera vez
                if (es_anim && anim_estado.visitados.count(n.id) &&
                    anim_estado.tiempo_visita_nodo.count(n.id)) {
                    float t_since = (float)ImGui::GetTime() - anim_estado.tiempo_visita_nodo[n.id];
                    if (t_since < 0.4f) {
                        float scale = 1.0f + Easing::easeOutBounce(t_since / 0.4f) * 0.3f;
                        float r_extra = n.radio * (scale - 1.0f);
                        dl->AddCircleFilled(n.posicion, n.radio + r_extra,
                            IM_COL32(0, 230, 120, 80), 32);
                    }
                }

                dl->AddCircleFilled(n.posicion, n.radio, colorFondo, 32);
                dl->AddCircle(n.posicion, n.radio, colorBorde, 32, 2.5f);

                if (!es_g2 && modo_actual == ModoApp::Redes) {
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
                if (!es_g2 && modo_actual == ModoApp::Redes && sim_inicializada &&
                    simulador.estado.nodos.count(n.id)) {
                    const auto& en = simulador.estado.nodos.at(n.id);

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
            if (modo_actual == ModoApp::Redes && sim_inicializada && !es_g2) {
                for (const auto& pkt : simulador.obtenerPaquetes()) {
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
            if ((anim_estado.activa || anim_estado.paso_actual >= 0) && anim_estado.particula.activa) {
                float t_ease = Easing::easeInOutCubic(anim_estado.particula.progreso);

                ImVec2 pos(
                    anim_estado.particula.pos_inicio.x + (anim_estado.particula.pos_fin.x - anim_estado.particula.pos_inicio.x) * t_ease,
                    anim_estado.particula.pos_inicio.y + (anim_estado.particula.pos_fin.y - anim_estado.particula.pos_inicio.y) * t_ease
                );

                // Halo exterior pulsante
                float halo_r = anim_estado.particula.radio * 2.5f * (1.0f - anim_estado.particula.progreso * 0.5f);
                ImU32 col_halo = (anim_estado.particula.color & 0x00FFFFFF) | (60 << 24);
                dl->AddCircleFilled(pos, halo_r, col_halo, 24);

                // Punto principal
                dl->AddCircleFilled(pos, anim_estado.particula.radio, anim_estado.particula.color, 24);

                // Brillo central
                dl->AddCircleFilled(pos, anim_estado.particula.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
            }
        }

        ImGui::End();
    }

        // MATRICES
        void dibujarMatrices(Grafo& red) {
        ImGui::Begin("Matrices");
        if (fontMono) ImGui::PushFont(fontMono);

        if (ImGui::BeginTabBar("MatricesTabs")) {
            if (ImGui::BeginTabItem(ICON_FA_TABLE_CELLS " Adyacencia")) {
                dibujarMatrizAdyacencia(red);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(ICON_FA_TABLE_LIST " Incidencia")) {
                dibujarMatrizIncidencia(red);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (fontMono) ImGui::PopFont();
        ImGui::End();
    }

    void dibujarMatrizAdyacencia(Grafo& red) {
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
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("\\");
            for (size_t c = 0; c < red.nodos.size(); c++) {
                ImGui::TableSetColumnIndex((int)c + 1);
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), "%s", red.nodos[c].nombre.c_str());
            }

            // Rows
            for (size_t f = 0; f < red.nodos.size(); f++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), "%s", red.nodos[f].nombre.c_str());

                for (size_t c = 0; c < red.nodos.size(); c++) {
                    ImGui::TableSetColumnIndex((int)c + 1);
                    if (f == c) {
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.35f, 1.0f), "x");
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

                    if (peso >= 0) {
                        ImVec4 col;
                        if (peso <= 3.0f) col = ImVec4(0.0f, 0.9f, 0.5f, 1.0f);       // green
                        else if (peso <= 10.0f) col = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // yellow
                        else col = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);                     // red
                        ImGui::TextColored(col, "%.0f", peso);
                    } else {
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.35f, 1.0f), "-");
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }

    void dibujarMatrizIncidencia(Grafo& red) {
        if (red.nodos.empty() || red.aristas.empty()) {
            ImGui::TextDisabled("Sin datos.");
            return;
        }

        // Contenedor con scroll controlado — ESTO es lo que faltaba
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
            // Congelar primera columna (nombres de nodos) al hacer scroll horizontal
            ImGui::TableSetupScrollFreeze(1, 1);

            // Definir anchos fijos por columna
            ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, col_w_label);
            for (int e = 0; e < (int)red.aristas.size(); e++) {
                char hdr[16];
                snprintf(hdr, sizeof(hdr), "e%d", e);
                ImGui::TableSetupColumn(hdr, ImGuiTableColumnFlags_WidthFixed, col_w_data);
            }

            // Fila de headers
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("\\");
            for (int e = 0; e < (int)red.aristas.size(); e++) {
                ImGui::TableSetColumnIndex(e + 1);
                ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), "e%d", e);
            }

            // Filas de nodos
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

        // CONSOLA KERNEL
        void dibujarPanelLogs() {
        ImGui::Begin("Registro del Kernel");
        if (fontMono) ImGui::PushFont(fontMono);

        if (ImGui::Button(ICON_FA_TRASH_CAN " Limpiar")) system_logs.clear();
        ImGui::SameLine();
        ImGui::Text("%d entradas", (int)system_logs.size());
        ImGui::Separator();

        ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& log : system_logs) {
            ImVec4 col(0.5f, 0.7f, 0.5f, 1.0f);
            if (log.find("[!]") != std::string::npos) col = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            else if (log.find("[OK]") != std::string::npos) col = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
            ImGui::TextColored(col, "%s", log.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        if (fontMono) ImGui::PopFont();
        ImGui::End();
    }

    };  // class Interfaz

// ── Includes de modulos de UI (despues de la definicion de Interfaz) ──────
#include "interfaz/componentes/StatusBar.h"
#include "interfaz/componentes/MenuPrincipal.h"
#include "interfaz/componentes/Dialogos.h"
#include "interfaz/paneles/PanelHardware.h"
#include "interfaz/paneles/PanelRed.h"
#include "interfaz/paneles/PanelIsomorfismo.h"
#include "interfaz/paneles/PanelGrafos.h"

// ── Cuerpo de dibujar() — definido aqui para que los modulos esten visibles ─
inline void Interfaz::dibujar(Grafo& red, GLFWwindow* ventana) {
    // Tick de simulacion de red
    if (modo_actual == ModoApp::Redes && sim_inicializada) {
        simulador.tick(red, ImGui::GetIO().DeltaTime);
    }

    // Avanzar animacion
    if (anim_estado.activa && !anim_estado.pausada) {
        float dt = ImGui::GetIO().DeltaTime;
        anim_estado.timer_paso += dt;

        // Actualizar particula en movimiento
        if (anim_estado.particula.activa) {
            anim_estado.particula.progreso += dt / anim_estado.particula.duracion;
            if (anim_estado.particula.progreso >= 1.0f) {
                anim_estado.particula.progreso = 1.0f;
                anim_estado.particula.activa  = false;
            }
        }

        // Avanzar al siguiente paso cuando el timer vence
        if (anim_estado.timer_paso >= anim_estado.velocidad_paso && anim_estado.activa) {
            anim_estado.timer_paso -= anim_estado.velocidad_paso;
            anim_estado.paso_actual++;
            if (anim_estado.paso_actual >= (int)anim_estado.pasos.size()) {
                anim_estado.activa = false;
            } else {
                const PasoAnimacion& paso = anim_estado.pasos[anim_estado.paso_actual];
                aplicarPaso(paso);

                // Lanzar particula si el paso involucra una arista
                if (paso.arista_origen >= 0 && paso.arista_destino >= 0) {
                    Nodo* o = red.obtenerNodo(paso.arista_origen);
                    Nodo* d = red.obtenerNodo(paso.arista_destino);
                    if (o && d) {
                        anim_estado.particula.activa     = true;
                        anim_estado.particula.pos_inicio = o->posicion;
                        anim_estado.particula.pos_fin    = d->posicion;
                        anim_estado.particula.progreso   = 0.0f;
                        anim_estado.particula.duracion   = anim_estado.velocidad_paso * 0.8f;

                        // Color segun tipo de paso
                        switch (paso.accion) {
                            case PasoAnimacion::EXPLORAR:
                                anim_estado.particula.color = IM_COL32(0, 200, 255, 255);
                                break;
                            case PasoAnimacion::CONFIRMAR:
                                anim_estado.particula.color = IM_COL32(255, 180, 0, 255);
                                break;
                            case PasoAnimacion::DESCARTAR:
                                anim_estado.particula.color = IM_COL32(255, 60, 60, 200);
                                break;
                            default:
                                anim_estado.particula.color = IM_COL32(0, 255, 180, 255);
                                break;
                        }
                    }
                }
            }
        }
    }

    // Jitter
    if (simulacion_jitter && modo_actual == ModoApp::Redes) {
        red.aplicarJitter(jitter_porcentaje);
    } else {
        red.resetearPesos();
    }

    MenuPrincipal::dibujar(*this, red, ventana);

    // DockSpace manual (deja 30px para StatusBar)
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, vp->WorkSize.y - 30));
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DockArea", nullptr,
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(3);

    ImGuiID dock_id = ImGui::GetID("OptiClustersDock");
    ImGui::DockSpace(dock_id);

    static bool layout_init = false;
    if (!layout_init) {
        layout_init = true;
        std::ifstream ini_check("imgui.ini");
        if (!ini_check.good()) {
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock_id, ImVec2(vp->WorkSize.x, vp->WorkSize.y - 30));

            ImGuiID main_id = dock_id;
            ImGuiID izq_id = ImGui::DockBuilderSplitNode(main_id, ImGuiDir_Left, 0.22f, nullptr, &main_id);
            ImGuiID der_id = ImGui::DockBuilderSplitNode(main_id, ImGuiDir_Right, 0.28f, nullptr, &main_id);
            ImGuiID abajo_id = ImGui::DockBuilderSplitNode(main_id, ImGuiDir_Down, 0.25f, nullptr, &main_id);

            ImGuiID redes_id = ImGui::DockBuilderSplitNode(izq_id, ImGuiDir_Down, 0.5f, nullptr, &izq_id);
            ImGui::DockBuilderDockWindow("Panel de Red", redes_id);

            ImGui::DockBuilderDockWindow("Herramientas de Red", izq_id);
            ImGui::DockBuilderDockWindow("Matrices", der_id);
            ImGui::DockBuilderDockWindow("Registro del Kernel", abajo_id);
            ImGui::DockBuilderDockWindow("Lienzo de Red", main_id);
            ImGui::DockBuilderFinish(dock_id);
        }
    }
    ImGui::End();

    PanelGrafos::dibujar(*this, red);
    if (modo_actual == ModoApp::Redes) {
        PanelRed::dibujar(*this, red);
    }
    dibujarLienzo(red);
    dibujarMatrices(red);
    dibujarPanelLogs();

    // Dialogos modales
    Dialogos::acercaDe();
    Dialogos::fallbackCargar(*this, red);
    Dialogos::fallbackGuardar(*this, red);

    StatusBar::dibujar(*this);
}