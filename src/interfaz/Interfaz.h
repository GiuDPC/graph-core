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
#include "IconsFontAwesome6.h"
#include "audio/Sonidos.h"
#include "interfaz/util/Animacion.h"
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

    // Nota: los wrappers de animacion (iniciar/aplicarPaso/reset)
    // se movieron a AnimacionUI.h (con side effects de log/sonido)

        // LIENZO DE RED — extraido a LienzoRed.h (namespace LienzoRed)

        // MATRICES, LOG PANEL — extraidos a Matrices.h y LogPanel.h

    };  // class Interfaz

// ── Includes de modulos de UI (despues de la definicion de Interfaz) ──────
#include "interfaz/componentes/StatusBar.h"
#include "interfaz/componentes/MenuPrincipal.h"
#include "interfaz/componentes/Dialogos.h"
#include "interfaz/paneles/PanelHardware.h"
#include "interfaz/paneles/PanelRed.h"
#include "interfaz/paneles/PanelIsomorfismo.h"
#include "interfaz/util/AnimacionUI.h"
#include "interfaz/paneles/PanelGrafos.h"
#include "interfaz/lienzo/LienzoRed.h"
#include "interfaz/paneles/Matrices.h"
#include "interfaz/componentes/LogPanel.h"

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
                AnimacionUI::aplicarPaso(*this, paso);

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
    Matrices::dibujar(red, *this);
    LogPanel::dibujar(*this);

    // Dialogos modales
    Dialogos::acercaDe();
    Dialogos::fallbackCargar(*this, red);
    Dialogos::fallbackGuardar(*this, red);

    StatusBar::dibujar(*this);
}