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
    std::vector<PasoAnimacion> pasos_animacion;
    int paso_actual = -1;
    float timer_paso = 0.0f;
    float velocidad_paso = 0.5f;
    bool animacion_activa = false;
    bool animacion_pausada = false;
    std::set<int> nodos_visitados;
    std::set<int> nodos_procesando;
    std::set<std::pair<int,int>> aristas_exploradas;
    std::set<std::pair<int,int>> aristas_confirmadas;
    std::set<std::pair<int,int>>       aristas_descartadas;

    // ── NUEVO: particula de animacion ─────────────────────────────────────────
    struct Particula {
        bool   activa     = false;
        ImVec2 pos_inicio;
        ImVec2 pos_fin;
        float  progreso   = 0.0f;   // 0.0 → 1.0
        float  duracion   = 0.3f;   // segundos para cruzar la arista
        ImU32  color      = IM_COL32(0, 255, 200, 255);
        float  radio      = 6.0f;
    };

    Particula particula_activa;
    std::map<int, float> tiempo_visita_nodo;  // nodo_id → tiempo cuando fue visitado

    // ── Easing functions ──────────────────────────────────────────────────────
    static float easeInOutCubic(float t) {
        return t < 0.5f
            ? 4.0f * t * t * t
            : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    static float easeOutBounce(float t) {
        if (t < 1.0f / 2.75f) {
            return 7.5625f * t * t;
        } else if (t < 2.0f / 2.75f) {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        } else if (t < 2.5f / 2.75f) {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        } else {
            t -= 2.625f / 2.75f;
            return 7.5625f * t * t + 0.984375f;
        }
    }

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

        // FUNCION PRINCIPAL
        void dibujar(Grafo& red, GLFWwindow* ventana) {
        // Tick de simulacion de red
        if (modo_actual == ModoApp::Redes && sim_inicializada) {
            simulador.tick(red, ImGui::GetIO().DeltaTime);
        }

        // Avanzar animacion
        if (animacion_activa && !animacion_pausada) {
            float dt = ImGui::GetIO().DeltaTime;
            timer_paso += dt;

            // Actualizar particula en movimiento
            if (particula_activa.activa) {
                particula_activa.progreso += dt / particula_activa.duracion;
                if (particula_activa.progreso >= 1.0f) {
                    particula_activa.progreso = 1.0f;
                    particula_activa.activa  = false;
                }
            }

            // Avanzar al siguiente paso cuando el timer vence
            if (timer_paso >= velocidad_paso && animacion_activa) {
                timer_paso -= velocidad_paso;
                paso_actual++;
                if (paso_actual >= (int)pasos_animacion.size()) {
                    animacion_activa = false;
                } else {
                    const PasoAnimacion& paso = pasos_animacion[paso_actual];
                    aplicarPaso(paso);

                    // Lanzar particula si el paso involucra una arista
                    if (paso.arista_origen >= 0 && paso.arista_destino >= 0) {
                        Nodo* o = red.obtenerNodo(paso.arista_origen);
                        Nodo* d = red.obtenerNodo(paso.arista_destino);
                        if (o && d) {
                            particula_activa.activa     = true;
                            particula_activa.pos_inicio = o->posicion;
                            particula_activa.pos_fin    = d->posicion;
                            particula_activa.progreso   = 0.0f;
                            particula_activa.duracion   = velocidad_paso * 0.8f;

                            // Color segun tipo de paso
                            switch (paso.accion) {
                                case PasoAnimacion::EXPLORAR:
                                    particula_activa.color = IM_COL32(0, 200, 255, 255);
                                    break;
                                case PasoAnimacion::CONFIRMAR:
                                    particula_activa.color = IM_COL32(255, 180, 0, 255);
                                    break;
                                case PasoAnimacion::DESCARTAR:
                                    particula_activa.color = IM_COL32(255, 60, 60, 200);
                                    break;
                                default:
                                    particula_activa.color = IM_COL32(0, 255, 180, 255);
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

        dibujarMenuBar(red, ventana);

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
            // Solo construir layout si no hay imgui.ini guardado
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

        dibujarPanelHerramientas(red);
        if (modo_actual == ModoApp::Redes) {
            dibujarPanelRedes(red);
        }
        dibujarLienzo(red);
        dibujarMatrices(red);
        dibujarPanelLogs();
        dibujarStatusBar();
    }

private:
        // MENU BAR
        void dibujarMenuBar(Grafo& red, GLFWwindow* ventana) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu(ICON_FA_FILE " Archivo")) {
                if (ImGui::MenuItem(ICON_FA_FILE_CIRCLE_PLUS " Nuevo Proyecto")) {
                    red.limpiar();
                    resetAnimacion();
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                    registrarLog("Proyecto nuevo creado");
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Cargar...")) {
                    auto resultado = pfd::open_file("Cargar topologia", ".", {"Archivos JSON", "*.json"}).result();
                    if (resultado.empty()) {
                        registrarLog("[!] Error: no se pudo abrir el dialogo de archivos.");
                        registrarLog("[!] Verifica que 'zenity' este instalado: sudo apt install zenity");
                        ImGui::OpenPopup("FallbackCargar");
                    } else {
                        Persistencia::cargar(red, resultado[0]);
                        resetAnimacion();
                        ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                        registrarLog("[OK] Proyecto cargado: " + resultado[0]);
                    }
                }
                if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Guardar como...")) {
                    auto resultado = pfd::save_file("Guardar topologia", "topologia.json", {"Archivos JSON", "*.json"}).result();
                    if (resultado.empty()) {
                        registrarLog("[!] Error: no se pudo abrir el dialogo de guardado.");
                        registrarLog("[!] Verifica que 'zenity' este instalado: sudo apt install zenity");
                        ImGui::OpenPopup("FallbackGuardar");
                    } else {
                        std::string ruta = resultado;
                        if (ruta.find(".json") == std::string::npos) ruta += ".json";
                        Persistencia::guardar(red, ruta);
                        registrarLog("[OK] Proyecto guardado: " + ruta);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_DOOR_OPEN " Salir")) {
                    glfwSetWindowShouldClose(ventana, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(ICON_FA_SLIDERS " Opciones")) {
                if (g_sonidos.funciona()) {
                    ImGui::Text(ICON_FA_VOLUME_HIGH " Audio");
                    float vol = g_sonidos.getVolumen();
                    if (ImGui::SliderFloat("Volumen", &vol, 0.0f, 1.0f, "%.0f%%")) {
                        g_sonidos.setVolumen(vol);
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                        ICON_FA_VOLUME_XMARK " Audio no disponible");
                    ImGui::TextDisabled("No se detecto dispositivo de sonido");
                }
                ImGui::Separator();
                ImGui::TextDisabled("OptiClusters v4.0 — NetSim Pro");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(ICON_FA_CIRCLE_INFO " Acerca de")) {
                ImGui::Text("OptiClusters v4.0 — NetSim Pro");
                ImGui::Text("Motor avanzado de visualizacion de grafos");
                ImGui::Text("Con audio y simulacion mejorada");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

        // PANEL DE HERRAMIENTAS
        void dibujarSelectorModo(Grafo& red) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "MODO DE OPERACION");
        float ancho = ImGui::GetContentRegionAvail().x;
        ImVec2 btnSize(ancho * 0.48f, 32);

        bool en_grafos = (modo_actual == ModoApp::Grafos);
        if (en_grafos) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
        if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " GRAFOS", btnSize)) {
            modo_actual = ModoApp::Grafos;
            registrarLog("Modo cambiado: Grafos");
        }
        if (en_grafos) ImGui::PopStyleColor();

        ImGui::SameLine();

        bool en_redes = (modo_actual == ModoApp::Redes);
        if (en_redes) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.55f, 0.45f, 1.0f));
        if (ImGui::Button(ICON_FA_NETWORK_WIRED " REDES", btnSize)) {
            modo_actual = ModoApp::Redes;
            registrarLog("Modo cambiado: Redes");
        }
        if (en_redes) ImGui::PopStyleColor();
    }

    void dibujarControlesAnimacion() {
        float ancho = ImGui::GetContentRegionAvail().x;

        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
            ICON_FA_FILM " ANIMACION");

        // Barra de progreso
        float progreso = 0.0f;
        if (!pasos_animacion.empty())
            progreso = (float)(paso_actual + 1) / (float)pasos_animacion.size();

        ImGui::ProgressBar(progreso, ImVec2(-1, 8));
        ImGui::Text("Paso %d / %d", paso_actual + 1, (int)pasos_animacion.size());

        // Descripcion del paso actual
        if (paso_actual >= 0 && paso_actual < (int)pasos_animacion.size()) {
            const auto& paso = pasos_animacion[paso_actual];
            if (!paso.descripcion.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 230, 255, 255));
                ImGui::TextWrapped("%s", paso.descripcion.c_str());
                ImGui::PopStyleColor();
            }
        }

        ImGui::SliderFloat("Velocidad##anim", &velocidad_paso, 0.05f, 2.0f, "%.2f s/paso");

        float bw = (ancho - 16) / 3.0f;
        if (animacion_activa && !animacion_pausada) {
            if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 0)))
                animacion_pausada = true;
        } else {
            if (ImGui::Button(ICON_FA_PLAY " Play", ImVec2(bw, 0))) {
                if (!animacion_activa && paso_actual >= 0) animacion_activa = true;
                animacion_pausada = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FORWARD_STEP " Paso", ImVec2(bw, 0))) {
            if (paso_actual + 1 < (int)pasos_animacion.size()) {
                paso_actual++;
                aplicarPaso(pasos_animacion[paso_actual]);
                animacion_pausada = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reset", ImVec2(bw, 0))) {
            resetAnimacion();
        }
    }

    void dibujarPropiedadesNodo(Grafo& red) {
        if (nodo_seleccionado >= 0) {
            Nodo* n = red.obtenerNodo(nodo_seleccionado);
            if (n) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.0f, 0.72f, 0.83f, 1.0f), ICON_FA_CIRCLE_INFO " PROPIEDADES");
                ImGui::Text("ID: %d", n->id);

                snprintf(buffer_nombre, sizeof(buffer_nombre), "%s", n->nombre.c_str());
                if (ImGui::InputText("Nombre", buffer_nombre, sizeof(buffer_nombre))) {
                    n->nombre = buffer_nombre;
                }

                if (modo_actual == ModoApp::Redes) {
                    int tipo_int = (int)n->tipo;
                    const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
                    if (ImGui::Combo("Tipo", &tipo_int, tipos, 5)) {
                        n->tipo = (TipoHardware)tipo_int;
                        registrarLog("Tipo cambiado: " + n->nombre + " -> " + tipos[tipo_int]);
                    }
                    ImGui::Text("Latencia: +%.0f ms", latenciaHardware(n->tipo));
                }

                ImGui::Spacing();
                if (ImGui::Button(ICON_FA_TRASH_CAN " Eliminar Nodo", ImVec2(-1, 0))) {
                    registrarLog("Nodo eliminado: " + n->nombre);
                    red.eliminarNodo(nodo_seleccionado);
                    nodo_seleccionado = -1;
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
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
                    registrarLog("Arista eliminada: " + red.nombreNodo(a.origen_id) + " - " + red.nombreNodo(a.destino_id));
                    red.aristas.erase(red.aristas.begin() + i);
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
        }
    }

    void dibujarPanelHerramientas(Grafo& red) {
        ImGui::Begin("Herramientas de Red");

        // Selector modo app (Grafos / Redes) — siempre visible
        dibujarSelectorModo(red);

        ImGui::Separator();

        // Navegacion: si estamos en un subpanel, mostrar boton volver
        if (modo_panel != ModoPanel::General) {
            if (ImGui::Button(ICON_FA_ARROW_LEFT " Volver", ImVec2(-1, 28))) {
                modo_panel = ModoPanel::General;
            }
            ImGui::Separator();
        }

        // Contenido segun modo
        switch (modo_panel) {
            case ModoPanel::General:     dibujarMenuGeneral(red);   break;
            case ModoPanel::Dijkstra:    dibujarSubpanelDijkstra(red); break;
            case ModoPanel::Kruskal:     dibujarSubpanelKruskal(red);  break;
            case ModoPanel::BFS:         dibujarSubpanelBFS(red);   break;
            case ModoPanel::DFS:         dibujarSubpanelDFS(red);   break;
            case ModoPanel::Ciclos:      dibujarSubpanelCiclos(red); break;
            case ModoPanel::Coloreo:     dibujarSubpanelColoreo(red); break;
            case ModoPanel::Isomorfismo: dibujarSubpanelIsomorfismo(red); break;
            case ModoPanel::Arbol:       dibujarSubpanelArbol(red); break;
            default: break;
        }

        // Controles de animacion — siempre visibles cuando hay animacion activa
        if (animacion_activa || paso_actual >= 0) {
            ImGui::Separator();
            dibujarControlesAnimacion();
        }

        // Propiedades del nodo seleccionado
        dibujarPropiedadesNodo(red);

        ImGui::End();
    }

    void dibujarMenuGeneral(Grafo& red) {
        float ancho = ImGui::GetContentRegionAvail().x;

        // ── Enrutamiento ─────────────────────────────────────────────────────────
        ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
            ICON_FA_ROUTE " ENRUTAMIENTO");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.35f, 0.28f, 1.0f));
        if (ImGui::Button(ICON_FA_DIAMOND " Dijkstra — Ruta Optima", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Dijkstra;
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Arbol expansion minima ────────────────────────────────────────────────
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
            ICON_FA_CIRCLE_NODES " TOPOLOGIA OPTIMA");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
        if (ImGui::Button(ICON_FA_TREE " Kruskal — MST", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Kruskal;
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Busqueda / Recorrido ──────────────────────────────────────────────────
        ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
            ICON_FA_MAGNIFYING_GLASS " RECORRIDO");
        float bw = (ancho - 8) / 2.0f;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.18f, 0.38f, 1.0f));
        if (ImGui::Button(ICON_FA_LAYER_GROUP " BFS", ImVec2(bw, 36))) {
            modo_panel = ModoPanel::BFS;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CODE_BRANCH " DFS", ImVec2(bw, 36))) {
            modo_panel = ModoPanel::DFS;
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ── Analisis ──────────────────────────────────────────────────────────────
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.1f, 1.0f),
            ICON_FA_CIRCLE_EXCLAMATION " ANALISIS");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.15f, 0.05f, 1.0f));
        if (ImGui::Button(ICON_FA_ROTATE " Detectar Ciclos", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Ciclos;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.08f, 0.28f, 1.0f));
        if (ImGui::Button(ICON_FA_PAINTBRUSH " Coloreo Greedy", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Coloreo;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.20f, 0.25f, 1.0f));
        if (ImGui::Button(ICON_FA_OBJECT_GROUP " Isomorfismo", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Isomorfismo;
            resetGrafoIsomorfismo();
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.28f, 0.10f, 1.0f));
        if (ImGui::Button(ICON_FA_SITEMAP " Analisis de Arbol", ImVec2(-1, 36))) {
            modo_panel = ModoPanel::Arbol;
            arbol_analizado = false;
            arbol_layout_aplicado = false;
        }
        ImGui::PopStyleColor();

        // ── Simulacion (solo redes) ───────────────────────────────────────────────
        if (modo_actual == ModoApp::Redes) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                ICON_FA_WAVE_SQUARE " SIMULACION");
            ImGui::Checkbox("Activar Jitter", &simulacion_jitter);
            if (simulacion_jitter) {
                ImGui::SliderFloat("Intensidad##jitter", &jitter_porcentaje, 0.01f, 0.50f, "%.0f%%");
            }
        }
    }

    void dibujarSubpanelDijkstra(Grafo& red) {
        ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
            ICON_FA_DIAMOND " DIJKSTRA");
        ImGui::TextWrapped(
            "Encuentra la ruta de menor costo entre dos nodos. "
            "En redes: equivale al protocolo OSPF de enrutamiento.");
        ImGui::Spacing();

        ImGui::Text("Origen:");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##origen_d", red.nombreNodo(dijkstra_origen).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == dijkstra_origen);
                if (ImGui::Selectable(n.nombre.c_str(), sel))
                    dijkstra_origen = n.id;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Text("Destino:");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##destino_d", red.nombreNodo(dijkstra_destino).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == dijkstra_destino);
                if (ImGui::Selectable(n.nombre.c_str(), sel))
                    dijkstra_destino = n.id;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (modo_actual == ModoApp::Redes) {
            ImGui::Checkbox("Incluir latencia de hardware", &dijkstra_usar_latencia);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Suma la latencia de cada tipo de equipo al costo de la ruta");
        }
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_PLAY " Animacion", ImVec2(-1, 32))) {
            auto pasos = Algoritmos::generarPasos(
                red, dijkstra_origen, dijkstra_destino, dijkstra_usar_latencia);
            iniciarAnimacion(pasos);
            ruta_optima.clear(); mostrar_mst = false;
            registrarLog("[Dijkstra] Animacion iniciada: " +
                red.nombreNodo(dijkstra_origen) + " -> " + red.nombreNodo(dijkstra_destino));
        }
        if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
            auto res = Algoritmos::dijkstra(
                red, dijkstra_origen, dijkstra_destino, dijkstra_usar_latencia);
            ruta_optima = res.ruta;
            mostrar_mst = false; resetAnimacion();
            if (res.hay_ruta)
                registrarLog("[OK] Dijkstra: " + std::to_string(res.saltos) +
                    " saltos, costo=" + std::to_string((int)res.costo_total));
            else
                registrarLog("[!] Dijkstra: no existe ruta entre esos nodos");
        }

        ImGui::Separator();
        if (!ruta_optima.empty()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                ICON_FA_CHECK " Ruta encontrada:");
            std::string ruta_str;
            for (size_t i = 0; i < ruta_optima.size(); i++) {
                if (i > 0) ruta_str += " -> ";
                ruta_str += red.nombreNodo(ruta_optima[i]);
            }
            ImGui::TextWrapped("%s", ruta_str.c_str());
            ImGui::Text("Saltos: %d", (int)ruta_optima.size() - 1);
        } else if (!animacion_activa) {
            ImGui::TextDisabled("Ejecuta un algoritmo para ver resultados.");
        }

        if (animacion_activa && !dijkstra_tabla_dist.empty()) {
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
                    float d = (n.id < (int)dijkstra_tabla_dist.size())
                        ? dijkstra_tabla_dist[n.id]
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

    void dibujarSubpanelKruskal(Grafo& red) {
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
            iniciarAnimacion(pasos);
            ruta_optima.clear(); mostrar_mst = false;
            registrarLog("[Kruskal] Animacion MST iniciada");
        }
        if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
            auto res = Algoritmos::Kruskal::kruskal(red);
            aristas_mst      = res.aristas_mst;
            mostrar_mst      = true;
            ruta_optima.clear(); resetAnimacion();
            registrarLog("[OK] Kruskal MST: " + std::to_string(res.aristas_aceptadas) +
                " aristas, peso total=" + std::to_string((int)res.peso_total));
        }

        if (mostrar_mst && !aristas_mst.empty()) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f),
                ICON_FA_CHECK " MST calculado:");

            float peso_total_grafo = 0;
            for (const auto& a : red.aristas) peso_total_grafo += a.peso;

            float peso_mst = 0;
            for (const auto& a : aristas_mst) peso_mst += a.peso;

            ImGui::Text("Aristas en MST:  %d / %d",
                (int)aristas_mst.size(), (int)red.aristas.size());
            ImGui::Text("Peso MST:        %.1f", peso_mst);
            ImGui::Text("Peso total red:  %.1f", peso_total_grafo);

            float ahorro = peso_total_grafo - peso_mst;
            if (ahorro > 0) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                    "Ahorro vs. red completa: %.1f", ahorro);
            }

            ImGui::Separator();
            ImGui::Text("Aristas del MST:");
            for (const auto& a : aristas_mst) {
                ImGui::BulletText("%s - %s (%.1f)",
                    red.nombreNodo(a.origen_id).c_str(),
                    red.nombreNodo(a.destino_id).c_str(),
                    a.peso);
            }
        }
    }

    void dibujarSubpanelBFS(Grafo& red) {
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
            int idx_inicio = bfs_nodo_inicio;
            if (!red.obtenerNodo(idx_inicio) && !red.nodos.empty())
                idx_inicio = red.nodos[0].id;

            if (ImGui::BeginCombo("##inicio_bfs", red.nombreNodo(idx_inicio).c_str())) {
                for (const auto& n : red.nodos) {
                    bool sel = (n.id == idx_inicio);
                    if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                        bfs_nodo_inicio = n.id;
                        idx_inicio = n.id;
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Spacing();

            if (ImGui::Button(ICON_FA_PLAY " Animacion BFS", ImVec2(-1, 32))) {
                auto pasos = Algoritmos::BFS::generarPasos(red, bfs_nodo_inicio);
                iniciarAnimacion(pasos);
                auto res = Algoritmos::BFS::bfs(red, bfs_nodo_inicio);
                bfs_resultado = res;
                registrarLog("[BFS] Iniciado desde " + red.nombreNodo(bfs_nodo_inicio));
            }
            if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
                bfs_resultado = Algoritmos::BFS::bfs(red, bfs_nodo_inicio);
                resetAnimacion();
                registrarLog("[OK] BFS: " + std::to_string(bfs_resultado.orden_visita.size()) +
                    " nodos visitados");
            }

            if (!bfs_resultado.nivel.empty()) {
                ImGui::Separator();
                ImGui::Text("Distancias desde %s:", red.nombreNodo(bfs_nodo_inicio).c_str());
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
                        auto it = bfs_resultado.nivel.find(n.id);
                        if (it != bfs_resultado.nivel.end())
                            ImGui::Text("%d saltos", it->second);
                        else
                            ImGui::TextDisabled("inaccesible");
                    }
                    ImGui::EndTable();
                }
            }
        }
    }

    void dibujarSubpanelDFS(Grafo& red) {
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f),
            ICON_FA_CODE_BRANCH " DFS — Busqueda en Profundidad");
        ImGui::TextWrapped(
            "Explora el grafo yendo tan profundo como sea posible antes de retroceder. "
            "Las aristas que vuelven a un nodo ya visitado (back-edges) indican ciclos.");
        ImGui::Spacing();

        int dfs_inicio = dfs_nodo_inicio;
        if (!red.obtenerNodo(dfs_inicio) && !red.nodos.empty())
            dfs_inicio = red.nodos[0].id;

        ImGui::Text("Nodo de inicio:");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##inicio_dfs", red.nombreNodo(dfs_inicio).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == dfs_inicio);
                if (ImGui::Selectable(n.nombre.c_str(), sel)) dfs_nodo_inicio = n.id;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_PLAY " Animacion DFS", ImVec2(-1, 32))) {
            auto pasos = Algoritmos::DFS::generarPasos(red, dfs_nodo_inicio);
            iniciarAnimacion(pasos);
            registrarLog("[DFS] Iniciado desde " + red.nombreNodo(dfs_nodo_inicio));
        }
        if (ImGui::Button(ICON_FA_BOLT " Instantaneo", ImVec2(-1, 32))) {
            dfs_resultado = Algoritmos::DFS::dfs(red, dfs_nodo_inicio);
            resetAnimacion();
            registrarLog("[OK] DFS: " + std::to_string(dfs_resultado.orden_visita.size()) +
                " nodos, " + std::to_string(dfs_resultado.back_edges.size()) + " back-edges");
        }

        if (!dfs_resultado.orden_visita.empty()) {
            ImGui::Separator();
            ImGui::Text("Orden de visita:");
            std::string orden;
            for (size_t i = 0; i < dfs_resultado.orden_visita.size(); i++) {
                if (i > 0) orden += " -> ";
                orden += red.nombreNodo(dfs_resultado.orden_visita[i]);
            }
            ImGui::TextWrapped("%s", orden.c_str());

            if (!dfs_resultado.back_edges.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.2f, 1.0f),
                    ICON_FA_TRIANGLE_EXCLAMATION " Back-edges (ciclos):");
                for (const auto& [u, v] : dfs_resultado.back_edges) {
                    ImGui::BulletText("%s -> %s",
                        red.nombreNodo(u).c_str(), red.nombreNodo(v).c_str());
                }
            } else {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                    ICON_FA_CHECK " Sin back-edges — no hay ciclos");
            }
        }
    }

    void dibujarSubpanelCiclos(Grafo& red) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.1f, 1.0f),
            ICON_FA_ROTATE " DETECCION DE CICLOS");
        ImGui::TextWrapped(
            "Un ciclo es un camino que comienza y termina en el mismo nodo. "
            "Un grafo sin ciclos (y conexo) es un arbol. "
            "En redes: los ciclos en switches causan broadcast storms — "
            "el STP los elimina automaticamente.");
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar", ImVec2(-1, 36))) {
            resultado_ciclos = Algoritmos::detectarCiclos(red);
            ciclo_analizado = true;
            if (resultado_ciclos.tiene_ciclo)
                registrarLog("[!] Ciclo detectado: " + resultado_ciclos.descripcion);
            else
                registrarLog("[OK] Grafo aciclico: " + resultado_ciclos.descripcion);
        }

        if (ciclo_analizado) {
            ImGui::Separator();
            if (resultado_ciclos.tiene_ciclo) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                    ICON_FA_TRIANGLE_EXCLAMATION " CICLO DETECTADO");
                ImGui::TextWrapped("%s", resultado_ciclos.descripcion.c_str());
                ImGui::Spacing();
                ImGui::Text("Aristas que forman el ciclo:");
                for (const auto& [u, v] : resultado_ciclos.aristas_ciclo) {
                    ImGui::BulletText("%s - %s",
                        red.nombreNodo(u).c_str(), red.nombreNodo(v).c_str());
                }
            } else {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                    ICON_FA_CHECK " Grafo aciclico");
                ImGui::TextWrapped("%s", resultado_ciclos.descripcion.c_str());
            }
        }
    }

    void dibujarSubpanelColoreo(Grafo& red) {
        ImGui::TextColored(ImVec4(0.7f, 0.1f, 0.7f, 1.0f),
            ICON_FA_PAINTBRUSH " COLOREO GREEDY");
        ImGui::TextWrapped(
            "Asigna colores a los nodos de forma que nodos adyacentes tengan colores distintos. "
            "El numero cromatico es el minimo de colores necesarios. "
            "Aplicacion real: asignacion de canales WiFi para evitar interferencias.");
        ImGui::Spacing();

        if (ImGui::Button(mostrar_coloreo
            ? ICON_FA_EYE_SLASH " Ocultar Coloreo"
            : ICON_FA_PAINT_ROLLER " Aplicar Coloreo", ImVec2(-1, 36)))
        {
            mostrar_coloreo = !mostrar_coloreo;
            if (mostrar_coloreo) {
                resultado_coloreo = Algoritmos::coloreoGreedy(red);
                colores_nodos = resultado_coloreo.colores;
                registrarLog("[OK] Coloreo: " + std::to_string(resultado_coloreo.num_colores) +
                    " colores usados (numero cromatico greedy)");
            }
        }

        if (mostrar_coloreo && !colores_nodos.empty()) {
            ImGui::Separator();
            ImGui::Text("Numero cromatico (greedy): %d", resultado_coloreo.num_colores);
            ImGui::TextDisabled("Nota: greedy no garantiza el optimo.");
            ImGui::TextDisabled("El optimo puede ser menor o igual.");
            ImGui::Spacing();
            ImGui::Text("Asignacion de colores:");
            const char* nombres_colores[] = {
                "Rojo", "Verde", "Azul", "Amarillo", "Magenta", "Cyan"
            };
            for (const auto& n : red.nodos) {
                if (n.id < (int)colores_nodos.size() && colores_nodos[n.id] >= 0) {
                    int c = colores_nodos[n.id];
                    ImGui::BulletText("%s -> Color %d (%s)",
                        n.nombre.c_str(), c,
                        c < 6 ? nombres_colores[c] : "Color extra");
                }
            }
        }
    }

    void dibujarSubpanelIsomorfismo(Grafo& red) {
        ImGui::TextColored(ImVec4(0.1f, 0.7f, 0.8f, 1.0f),
            ICON_FA_OBJECT_GROUP " ISOMORFISMO");
        
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(190, 200, 210, 255));
        ImGui::TextWrapped(
            "¿Qué es esto?\n"
            "Dos grafos son isomorfos si tienen exactamente la misma forma y "
            "conexiones, sin importar dónde estén dibujados o cómo se llamen sus nodos.\n"
            "Es útil para saber si dos redes aparentemente distintas son en realidad la misma.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("IsoTabs")) {
            if (ImGui::BeginTabItem("G1 (Tu Grafo Principal)")) {
                ImGui::Text("Nodos: %d  |  Aristas: %d",
                    (int)red.nodos.size(), (int)red.aristas.size());
                auto degs = Algoritmos::Isomorfismo::secuenciaGrados(red);
                std::string dstr;
                for (int d : degs) dstr += std::to_string(d) + " ";
                ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("G2 (El Grafo a Comparar)")) {
                ImGui::Text("Nodos: %d  |  Aristas: %d",
                    (int)grafo_iso_g2.nodos.size(), (int)grafo_iso_g2.aristas.size());
                auto degs = Algoritmos::Isomorfismo::secuenciaGrados(grafo_iso_g2);
                std::string dstr;
                for (int d : degs) dstr += std::to_string(d) + " ";
                ImGui::TextWrapped("Secuencia de grados: %s", dstr.c_str());
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                    "Opciones para G2:");
                
                if (ImGui::Button(ICON_FA_WAND_MAGIC_SPARKLES " Generar Isomorfo Automático", ImVec2(-1, 0))) {
                    grafo_iso_g2.limpiar();
                    if (!red.nodos.empty()) {
                        std::vector<int> idx(red.nodos.size());
                        for(size_t i=0; i<idx.size(); ++i) idx[i] = i;
                        for(size_t i=0; i<idx.size(); ++i) {
                            size_t j = i + rand() % (idx.size() - i);
                            std::swap(idx[i], idx[j]);
                        }
                        std::map<int, int> mapa;
                        float min_x = 9999, max_x = -9999;
                        float min_y = 9999, max_y = -9999;
                        for (auto& n : red.nodos) {
                            if (n.posicion.x < min_x) min_x = n.posicion.x;
                            if (n.posicion.x > max_x) max_x = n.posicion.x;
                            if (n.posicion.y < min_y) min_y = n.posicion.y;
                            if (n.posicion.y > max_y) max_y = n.posicion.y;
                        }
                        if (min_x > max_x) { min_x = 0; max_x = 800; min_y = 0; max_y = 600; }
                        float cx = max_x + 350.0f; // Offset a la derecha del grafo original
                        float cy = (min_y + max_y) / 2.0f;
                        float radio = std::max(150.0f, (max_y - min_y) / 2.0f);
                        for(size_t i=0; i<idx.size(); ++i) {
                            const auto& n_orig = red.nodos[idx[i]];
                            float ang = (2.0f * 3.14159f / idx.size()) * i;
                            Nodo n(i, ImVec2(cx + cosf(ang)*radio, cy + sinf(ang)*radio), n_orig.tipo);
                            n.nombre = "Iso_" + n_orig.nombre;
                            grafo_iso_g2.nodos.push_back(n);
                            mapa[n_orig.id] = i;
                        }
                        for (const auto& a : red.aristas) {
                            grafo_iso_g2.aristas.push_back(Arista(mapa[a.origen_id], mapa[a.destino_id], a.peso, a.es_dirigida));
                        }
                        grafo_iso_g2.contador_ids = red.nodos.size();
                        iso_editando_g2 = true;
                        iso_analizado = false;
                        registrarLog("[OK] G2 generado automáticamente. ¡Están desordenados pero son idénticos!");
                    }
                }

                ImGui::Checkbox("Dibujar G2 manualmente", &iso_editando_g2);
                if (iso_editando_g2) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                        ICON_FA_PENCIL " Click derecho en el lienzo para crear nodos/aristas en G2");
                }
                if (ImGui::Button("Limpiar G2", ImVec2(-1, 0))) {
                    grafo_iso_g2.limpiar();
                    iso_analizado = false;
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Verificar Isomorfismo", ImVec2(-1, 36))) {
            if (red.nodos.empty() || grafo_iso_g2.nodos.empty()) {
                registrarLog("[!] Isomorfismo: ambos grafos deben tener nodos");
            } else {
                resultado_iso = Algoritmos::Isomorfismo::verificar(red, grafo_iso_g2);
                iso_analizado = true;
                registrarLog(resultado_iso.son_isomorfos
                    ? "[OK] Isomorfismo: ¡Genial! Los grafos SON isomorfos."
                    : "[!] Isomorfismo: Los grafos NO son isomorfos.");
            }
        }

        if (iso_analizado) {
            ImGui::Separator();

            auto icono_cond = [](bool ok) {
                return ok ? ICON_FA_CHECK : ICON_FA_XMARK;
            };
            auto col_cond = [](bool ok) -> ImVec4 {
                return ok ? ImVec4(0.0f, 1.0f, 0.5f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            };

            ImGui::Text("Condiciones necesarias:");
            ImGui::TextColored(col_cond(resultado_iso.misma_cantidad_nodos),
                "%s Misma cantidad de nodos", icono_cond(resultado_iso.misma_cantidad_nodos));
            ImGui::TextColored(col_cond(resultado_iso.misma_cantidad_aristas),
                "%s Misma cantidad de aristas", icono_cond(resultado_iso.misma_cantidad_aristas));
            ImGui::TextColored(col_cond(resultado_iso.misma_secuencia_grados),
                "%s Misma secuencia de grados", icono_cond(resultado_iso.misma_secuencia_grados));

            ImGui::Spacing();
            if (resultado_iso.son_isomorfos) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                    ICON_FA_CHECK " SON ISOMORFOS");
                ImGui::Text("Mapeo de nodos:");
                for (const auto& par : resultado_iso.mapeo) {
                    ImGui::BulletText("%s (G1) <-> %s (G2)",
                        red.nombreNodo(par.first).c_str(),
                        grafo_iso_g2.nombreNodo(par.second).c_str());
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                    ICON_FA_XMARK " NO SON ISOMORFOS");
                ImGui::TextWrapped("%s", resultado_iso.descripcion.c_str());
            }
        }
    }

        // ARBOL: layout jerarquico
        void aplicarLayoutArbol(Grafo& red, const Algoritmos::Arbol::PropiedadesArbol& props,
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

        // SUBPANEL: Arbol
        void dibujarSubpanelArbol(Grafo& red) {
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
        if (!red.nodos.empty() && !red.obtenerNodo(arbol_raiz_id))
            arbol_raiz_id = red.nodos[0].id;
        if (ImGui::BeginCombo("##raiz_arbol", red.nombreNodo(arbol_raiz_id).c_str())) {
            for (const auto& n : red.nodos) {
                bool sel = (n.id == arbol_raiz_id);
                if (ImGui::Selectable(n.nombre.c_str(), sel)) {
                    arbol_raiz_id = n.id;
                    arbol_analizado = false;
                    arbol_layout_aplicado = false;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Analizar Arbol", ImVec2(-1, 36))) {
            arbol_props    = Algoritmos::Arbol::analizar(red, arbol_raiz_id);
            arbol_analizado = true;
            arbol_layout_aplicado = false;
            registrarLog("[OK] Arbol analizado. Altura=" + std::to_string(arbol_props.altura) +
                ", Grado=" + std::to_string(arbol_props.grado_arbol));
        }

        if (arbol_analizado && !arbol_layout_aplicado) {
            if (ImGui::Button(ICON_FA_SITEMAP " Aplicar Layout Jerarquico", ImVec2(-1, 28))) {
                ImVec2 centro(800.0f, 400.0f);
                aplicarLayoutArbol(red, arbol_props, centro);
                arbol_layout_aplicado = true;
                registrarLog("[OK] Layout jerarquico aplicado");
            }
        }

        if (arbol_analizado) {
            ImGui::Separator();

            ImGui::Text("Altura:              %d", arbol_props.altura);
            ImGui::Text("Grado del arbol:     %d", arbol_props.grado_arbol);
            ImGui::Text("Numero de hojas:     %d", (int)arbol_props.hojas.size());
            ImGui::Spacing();

            if (!arbol_props.rama_mas_larga.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Rama mas larga:");
                std::string rml;
                for (size_t i = 0; i < arbol_props.rama_mas_larga.size(); i++) {
                    if (i > 0) rml += " -> ";
                    rml += red.nombreNodo(arbol_props.rama_mas_larga[i]);
                }
                ImGui::TextWrapped("%s", rml.c_str());
            }
            if (!arbol_props.rama_mas_corta.empty()) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Rama mas corta:");
                std::string rmc;
                for (size_t i = 0; i < arbol_props.rama_mas_corta.size(); i++) {
                    if (i > 0) rmc += " -> ";
                    rmc += red.nombreNodo(arbol_props.rama_mas_corta[i]);
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
                    ImGui::Text("%d", arbol_props.nivel.count(n.id) ? arbol_props.nivel.at(n.id) : -1);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", arbol_props.hijos.count(n.id) ? (int)arbol_props.hijos.at(n.id).size() : 0);
                    ImGui::TableSetColumnIndex(3);
                    if (n.id == arbol_props.raiz_id) {
                        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Raiz");
                    } else if (std::find(arbol_props.hojas.begin(), arbol_props.hojas.end(), n.id) != arbol_props.hojas.end()) {
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Hoja");
                    } else {
                        ImGui::TextDisabled("Interno");
                    }
                }
                ImGui::EndTable();
            }

            // Panel de detalles del nodo seleccionado
            if (nodo_seleccionado >= 0 && arbol_props.nivel.count(nodo_seleccionado)) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.8f, 1.0f),
                    "Detalles de %s:", red.nombreNodo(nodo_seleccionado).c_str());

                int nid = nodo_seleccionado;
                int pid = arbol_props.padre.count(nid) ? arbol_props.padre.at(nid) : -1;
                ImGui::Text("Padre:       %s", pid >= 0 ? red.nombreNodo(pid).c_str() : "(raiz)");

                const auto& hijos = arbol_props.hijos.at(nid);
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

                if (pid >= 0 && arbol_props.hermanos.count(pid)) {
                    const auto& herm = arbol_props.hermanos.at(pid);
                    std::string sh;
                    for (int h : herm) {
                        if (h == nid) continue;
                        if (!sh.empty()) sh += ", ";
                        sh += red.nombreNodo(h);
                    }
                    ImGui::Text("Hermanos:    %s", sh.empty() ? "(ninguno)" : sh.c_str());
                }

                if (arbol_props.ancestros.count(nid)) {
                    const auto& anc = arbol_props.ancestros.at(nid);
                    std::string sa;
                    for (size_t i = 0; i < anc.size(); i++) {
                        if (i > 0) sa += " -> ";
                        sa += red.nombreNodo(anc[i]);
                    }
                    ImGui::Text("Ancestros:   %s", sa.empty() ? "(raiz)" : sa.c_str());
                }

                if (arbol_props.descendientes.count(nid)) {
                    ImGui::Text("Descendientes: %d nodos",
                        (int)arbol_props.descendientes.at(nid).size());
                }

                std::vector<std::string> mis_primos;
                for (const auto& [a, b] : arbol_props.primos) {
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

            if (!arbol_props.primos.empty()) {
                ImGui::Separator();
                if (ImGui::CollapsingHeader("Pares de primos")) {
                    for (const auto& [a, b] : arbol_props.primos) {
                        ImGui::BulletText("%s y %s (nivel %d)",
                            red.nombreNodo(a).c_str(),
                            red.nombreNodo(b).c_str(),
                            arbol_props.nivel.at(a));
                    }
                }
            }
        }
    }

        // PANEL DE REDES (SIMULACION)
        void dibujarPanelRedes(Grafo& red) {
        ImGui::Begin("Panel de Red");

        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            ICON_FA_NETWORK_WIRED " MODO RED — SIMULACION");
        ImGui::Separator();

        // ── Topologias predefinidas ──────────────────────────────────────────────
        if (ImGui::CollapsingHeader(ICON_FA_SITEMAP " Topologias Predefinidas")) {
            ImVec2 centro(700.0f, 400.0f);

            if (ImGui::Button("Red Empresarial", ImVec2(-1, 0))) {
                Topologias::empresarialBasica(red, centro);
                sim_inicializada = false;
                ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                registrarLog("[OK] Topologia: Red Empresarial cargada");
            }
            if (ImGui::Button("Mesh Tolerante", ImVec2(-1, 0))) {
                Topologias::meshTolerante(red, centro);
                sim_inicializada = false;
                registrarLog("[OK] Topologia: Mesh cargada");
            }
            if (ImGui::Button("Red Estrella", ImVec2(-1, 0))) {
                Topologias::estrellaSimple(red, centro);
                sim_inicializada = false;
                registrarLog("[OK] Topologia: Estrella cargada");
            }
            if (ImGui::Button("Internet Simplificado", ImVec2(-1, 0))) {
                Topologias::internetSimple(red, centro);
                sim_inicializada = false;
                registrarLog("[OK] Topologia: Internet Simple cargada");
            }
        }
        ImGui::Separator();

        // ── Simulacion ───────────────────────────────────────────────────────────
        ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
            ICON_FA_WAVE_SQUARE " SIMULACION EN TIEMPO REAL");

        if (!sim_inicializada) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                ICON_FA_CIRCLE_EXCLAMATION " Simulacion no iniciada");
            if (ImGui::Button("Iniciar Simulacion", ImVec2(-1, 36))) {
                simulador.inicializar(red);
                sim_inicializada = true;
                registrarLog("[OK] Simulacion iniciada");
                // Generar trafico de demostracion para que se vea actividad
                int n_nodos = (int)red.nodos.size();
                if (n_nodos >= 3) {
                    int a = red.nodos[0].id;
                    int b = red.nodos[n_nodos-1].id;
                    int c = red.nodos[n_nodos/2].id;
                    simulador.enviarFlujo(a, b, 10.0f,   "HTTP", 30.0f, red);
                    simulador.enviarFlujo(b, c, 0.1f,   "PING", 30.0f, red);
                    if (n_nodos >= 5) {
                        int d = red.nodos[n_nodos/3].id;
                        simulador.enviarFlujo(d, a, 50.0f, "VIDEO", 20.0f, red);
                    }
                    registrarLog("[OK] Trafico de prueba generado (HTTP, PING, VIDEO)");
                }
                mostrar_modal_inicio = true;
            }
        } else {
            // Estado de la simulacion
            ImGui::Text("Tiempo: %.1f s", simulador.estado.tiempo);
            ImGui::SameLine();
            ImGui::Text("| Flujos: %d", (int)simulador.estado.flujos.size());
            ImGui::SameLine();
            ImGui::Text("| Pkt: %d", (int)simulador.obtenerPaquetes().size());

            ImGui::SliderFloat("Velocidad##sim", &simulador.estado.velocidad, 0.1f, 5.0f, "x%.1f");
            ImGui::Checkbox("Eventos automáticos", &simulador.eventos_automaticos);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Spikes de trafico y microcortes aleatorios");

            float bw = (ImGui::GetContentRegionAvail().x - 8) / 2.0f;
            if (simulador.estado.activa) {
                if (ImGui::Button(ICON_FA_PAUSE " Pausar", ImVec2(bw, 0)))
                    simulador.estado.activa = false;
            } else {
                if (ImGui::Button(ICON_FA_PLAY " Reanudar", ImVec2(bw, 0)))
                    simulador.estado.activa = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reiniciar", ImVec2(bw, 0))) {
                simulador.inicializar(red);
                registrarLog("[OK] Simulacion reiniciada");
            }
        }
        ImGui::Separator();

        // ── Enviar trafico ───────────────────────────────────────────────────────
        if (sim_inicializada) {
            if (ImGui::CollapsingHeader(ICON_FA_PAPER_PLANE " Enviar Trafico")) {
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Crea un flujo de datos entre dos nodos.\nHTTP=10, Video=50, Ping=0.1, DDoS=500 Mbps.\nLos puntitos de colores viajan por las aristas.");
                const char* tipos_flujo[] = {"HTTP (10Mbps)", "Video (50Mbps)", "Ping (0.1Mbps)", "DDoS (500Mbps)"};
                float mbps_flujo[] = {10.0f, 50.0f, 0.1f, 500.0f};
                const char* nombre_flujo[] = {"HTTP", "VIDEO", "PING", "DDOS"};

                ImGui::Text("Origen:");
                ImGui::SetNextItemWidth(-1);
                if (ImGui::BeginCombo("##fl_orig", red.nombreNodo(flujo_origen).c_str())) {
                    for (const auto& n : red.nodos) {
                        bool sel = (n.id == flujo_origen);
                        if (ImGui::Selectable(n.nombre.c_str(), sel)) flujo_origen = n.id;
                        if (sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::Text("Destino:");
                ImGui::SetNextItemWidth(-1);
                if (ImGui::BeginCombo("##fl_dst", red.nombreNodo(flujo_destino).c_str())) {
                    for (const auto& n : red.nodos) {
                        bool sel = (n.id == flujo_destino);
                        if (ImGui::Selectable(n.nombre.c_str(), sel)) flujo_destino = n.id;
                        if (sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::Combo("Tipo##flujo", &flujo_tipo, tipos_flujo, 4);
                ImGui::SliderFloat("Duracion (s)", &flujo_dur, 1.0f, 60.0f, "%.0f s");

                if (ImGui::Button(ICON_FA_PAPER_PLANE " Enviar", ImVec2(-1, 32))) {
                    if (flujo_origen != flujo_destino) {
                        simulador.enviarFlujo(flujo_origen, flujo_destino,
                            mbps_flujo[flujo_tipo], nombre_flujo[flujo_tipo],
                            flujo_dur, red);
                        g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
                    }
                }
            }

            // ── Simular fallos ───────────────────────────────────────────────────
            if (ImGui::CollapsingHeader(ICON_FA_TRIANGLE_EXCLAMATION " Simular Fallos")) {
                ImGui::TextDisabled("Selecciona un nodo en el lienzo y haz click aqui:");

                if (nodo_seleccionado >= 0) {
                    auto& en = simulador.estado.nodos[nodo_seleccionado];
                    if (en.activo) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                if (ImGui::Button(("Derribar " + red.nombreNodo(nodo_seleccionado)).c_str(),
                    ImVec2(-1, 0))) {
                    simulador.simularFalloNodo(nodo_seleccionado, red);
                    g_sonidos.reproducir(Sonidos::NODO_CAIDO);
                }
                        ImGui::PopStyleColor();
                    } else {
                        float tc = en.tiempo_caida;
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                            ICON_FA_TRIANGLE_EXCLAMATION " %s CAIDO (%.0fs)",
                            red.nombreNodo(nodo_seleccionado).c_str(), tc);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                        if (ImGui::Button(("Restaurar " + red.nombreNodo(nodo_seleccionado)).c_str(),
                            ImVec2(-1, 0))) {
                            simulador.restaurarNodo(nodo_seleccionado, red);
                            g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                        }
                        ImGui::PopStyleColor();
                    }
                } else {
                    ImGui::TextDisabled("Ningún nodo seleccionado");
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
                        if (!simulador.estado.nodos.count(n.id)) continue;
                        const auto& en = simulador.estado.nodos.at(n.id);
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
                for (auto it = simulador.estado.log_eventos.rbegin();
                     it != simulador.estado.log_eventos.rend(); ++it)
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
        if (mostrar_modal_inicio) {
            ImGui::OpenPopup("Simulacion iniciada");
            mostrar_modal_inicio = false;
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
                    bool anim_activa_o_finalizada = (animacion_activa || paso_actual >= 0);
                    
                    if (anim_activa_o_finalizada) {
                        auto parR = std::make_pair(a.destino_id, a.origen_id);

                        if (aristas_confirmadas.count(par) || aristas_confirmadas.count({a.origen_id, a.destino_id}) ||
                            aristas_confirmadas.count(parR)) {
                            // Arista confirmada: dorada con brillo
                            col    = IM_COL32(255, 179, 0, 255);
                            grosor = 5.0f;
                            // Agregar linea de brillo encima
                            dl->AddLine(o->posicion, d->posicion,
                                IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                        } else if (aristas_exploradas.count(par) || aristas_exploradas.count({a.origen_id, a.destino_id}) ||
                                   aristas_exploradas.count(parR)) {
                            col    = IM_COL32(0, 188, 212, 220);
                            grosor = 3.5f;
                        } else if (aristas_descartadas.count(par) || aristas_descartadas.count({a.origen_id, a.destino_id}) ||
                                   aristas_descartadas.count(parR)) {
                            // Arista descartada: roja pulsante
                            float pulse_a = (sinf((float)ImGui::GetTime() * 5.0f) + 1.0f) * 0.5f;
                            col    = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60));
                            grosor = 2.5f;
                        }
                    } 
                    
                    // Mostrar resultados finales siempre que no este corriendo la animacion activamente
                    // o cuando haya terminado.
                    bool mostrar_resultados_finales = (!animacion_activa && paso_actual >= (int)pasos_animacion.size() - 1);
                    
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
                bool es_anim = (!es_g2 && (animacion_activa || paso_actual >= 0));

                if (es_g2) {
                    colorFondo = IM_COL32(180, 180, 0, 255);
                    colorBorde = IM_COL32(255, 255, 100, 255);
                } else if (es_anim && nodos_procesando.count(n.id)) {
                    colorFondo = IM_COL32(255, 215, 0, 200);
                    colorBorde = IM_COL32(255, 235, 100, 255);
                } else if (es_anim && nodos_visitados.count(n.id)) {
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
                if (es_anim && nodos_visitados.count(n.id) &&
                    tiempo_visita_nodo.count(n.id)) {
                    float t_since = (float)ImGui::GetTime() - tiempo_visita_nodo[n.id];
                    if (t_since < 0.4f) {
                        float scale = 1.0f + easeOutBounce(t_since / 0.4f) * 0.3f;
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

                    float t = easeInOutCubic(pkt.progreso);
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
            if ((animacion_activa || paso_actual >= 0) && particula_activa.activa) {
                float t_ease = easeInOutCubic(particula_activa.progreso);

                ImVec2 pos(
                    particula_activa.pos_inicio.x + (particula_activa.pos_fin.x - particula_activa.pos_inicio.x) * t_ease,
                    particula_activa.pos_inicio.y + (particula_activa.pos_fin.y - particula_activa.pos_inicio.y) * t_ease
                );

                // Halo exterior pulsante
                float halo_r = particula_activa.radio * 2.5f * (1.0f - particula_activa.progreso * 0.5f);
                ImU32 col_halo = (particula_activa.color & 0x00FFFFFF) | (60 << 24);
                dl->AddCircleFilled(pos, halo_r, col_halo, 24);

                // Punto principal
                dl->AddCircleFilled(pos, particula_activa.radio, particula_activa.color, 24);

                // Brillo central
                dl->AddCircleFilled(pos, particula_activa.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
            }
        }

        // ─── Fallback: Cargar manualmente ───
        if (ImGui::BeginPopupModal("FallbackCargar", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("No se pudo abrir el dialogo del sistema.");
            ImGui::Text("Escribi la ruta del archivo .json manualmente:");
            static char ruta_cargar[256] = {};
            ImGui::SetNextItemWidth(400);
            ImGui::InputText("##rutaCargar", ruta_cargar, sizeof(ruta_cargar));
            ImGui::Spacing();
            if (ImGui::Button("Cargar", ImVec2(120, 0))) {
                if (strlen(ruta_cargar) > 0) {
                    Persistencia::cargar(red, std::string(ruta_cargar));
                    resetAnimacion();
                    ruta_optima.clear(); aristas_mst.clear(); mostrar_mst = false;
                    registrarLog("[OK] Cargado desde ruta manual: " + std::string(ruta_cargar));
                    memset(ruta_cargar, 0, sizeof(ruta_cargar));
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // ─── Fallback: Guardar manualmente ───
        if (ImGui::BeginPopupModal("FallbackGuardar", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("No se pudo abrir el dialogo del sistema.");
            ImGui::Text("Asegurate de que la ruta sea valida o relativa al binario:");
            static char ruta_guardar[256] = {};
            ImGui::SetNextItemWidth(400);
            ImGui::InputText("##rutaGuardar", ruta_guardar, sizeof(ruta_guardar));
            ImGui::Spacing();
            if (ImGui::Button("Guardar", ImVec2(120, 0))) {
                if (strlen(ruta_guardar) > 0) {
                    std::string ruta = ruta_guardar;
                    if (ruta.find(".json") == std::string::npos) ruta += ".json";
                    Persistencia::guardar(red, ruta);
                    registrarLog("[OK] Guardado en ruta manual: " + ruta);
                    memset(ruta_guardar, 0, sizeof(ruta_guardar));
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancelar", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
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

        // BARRA DE ESTADO
        void dibujarStatusBar() {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - 30));
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, 30));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.09f, 1.0f));
        ImGui::Begin("##StatusBar", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

        const char* modo_txt = (modo_actual == ModoApp::Grafos) ? ICON_FA_DIAGRAM_PROJECT " GRAFOS" : ICON_FA_NETWORK_WIRED " REDES";
        ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f), "%s", modo_txt);

        ImGui::SameLine(180);
        if (animacion_activa) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), ICON_FA_SPINNER " ANIMANDO");
        } else if (modo_actual == ModoApp::Redes && sim_inicializada) {
            if (simulador.estado.activa) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_PLAY " SIMULANDO x%.1f", simulador.estado.velocidad);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), ICON_FA_PAUSE " PAUSADA");
            }
            ImGui::SameLine(400);
            ImGui::Text("T: %.0fs | Flujos: %d | Paqs: %d",
                simulador.estado.tiempo,
                (int)simulador.estado.flujos.size(),
                (int)simulador.obtenerPaquetes().size());
        } else if (simulacion_jitter && modo_actual == ModoApp::Redes) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), ICON_FA_WAVE_SQUARE " JITTER ACTIVO");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.5f, 1.0f), ICON_FA_CIRCLE_CHECK " ENGINE OK");
        }

        ImGui::SameLine(650);
        ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);

        ImGui::SameLine(750);
        if (g_sonidos.funciona()) {
            float v = g_sonidos.getVolumen();
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
                ICON_FA_VOLUME_HIGH " %d%%", (int)(v*100));
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                ICON_FA_VOLUME_XMARK " SIN AUDIO");
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

        // ANIMACION
        void iniciarAnimacion(std::vector<PasoAnimacion> pasos) {
        resetAnimacion();
        pasos_animacion = std::move(pasos);
        animacion_activa = true;
        animacion_pausada = false;
        paso_actual = -1;
        timer_paso = 0.0f;
        if (!pasos_animacion.empty()) {
            registrarLog("Animacion iniciada: " + std::to_string(pasos_animacion.size()) + " pasos");
            g_sonidos.reproducir(Sonidos::ALGORITMO_FIN);
        }
    }

    void aplicarPaso(const PasoAnimacion& p) {
        switch (p.accion) {
            case PasoAnimacion::VISITAR:
                if (p.nodo_id >= 0) {
                    nodos_procesando.insert(p.nodo_id);
                    nodos_visitados.insert(p.nodo_id);
                    tiempo_visita_nodo[p.nodo_id] = (float)ImGui::GetTime();
                }
                g_sonidos.reproducir(Sonidos::VISITAR_NODO);
                break;
            case PasoAnimacion::CONFIRMAR:
                if (p.nodo_id >= 0) {
                    nodos_procesando.erase(p.nodo_id);
                    nodos_visitados.insert(p.nodo_id);
                }
                if (p.arista_origen >= 0 && p.arista_destino >= 0) {
                    aristas_confirmadas.insert({p.arista_origen, p.arista_destino});
                    aristas_exploradas.erase({p.arista_origen, p.arista_destino});
                }
                g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
                break;
            case PasoAnimacion::EXPLORAR:
                if (p.arista_origen >= 0 && p.arista_destino >= 0)
                    aristas_exploradas.insert({p.arista_origen, p.arista_destino});
                g_sonidos.reproducir(Sonidos::PAQUETE_ENVIADO);
                break;
            case PasoAnimacion::DESCARTAR:
                if (p.arista_origen >= 0 && p.arista_destino >= 0)
                    aristas_descartadas.insert({p.arista_origen, p.arista_destino});
                g_sonidos.reproducir(Sonidos::DESCARTAR);
                break;
        }
        if (!p.descripcion.empty()) registrarLog(p.descripcion);
    }

    void resetAnimacion() {
        pasos_animacion.clear();
        paso_actual = -1;
        timer_paso = 0.0f;
        animacion_activa = false;
        animacion_pausada = false;
        nodos_visitados.clear();
        nodos_procesando.clear();
        aristas_exploradas.clear();
        aristas_confirmadas.clear();
        aristas_descartadas.clear();
        particula_activa.activa = false;
        tiempo_visita_nodo.clear();
    }

        // UTILIDADES
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
};