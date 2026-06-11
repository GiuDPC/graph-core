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

        // LIENZO DE RED — extraido a LienzoRed.h (namespace LienzoRed)

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
#include "interfaz/lienzo/LienzoRed.h"

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
    LienzoRed::dibujar(red, *this);
    dibujarMatrices(red);
    dibujarPanelLogs();

    // Dialogos modales
    Dialogos::acercaDe();
    Dialogos::fallbackCargar(*this, red);
    Dialogos::fallbackGuardar(*this, red);

    StatusBar::dibujar(*this);
}