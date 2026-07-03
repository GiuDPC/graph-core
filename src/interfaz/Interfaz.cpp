#include "Interfaz.hpp"

// UI module includes — moved here to break compile coupling
#include "interfaz/componentes/MenuPrincipal.h"
#include "interfaz/componentes/StatusBar.h"
#include "interfaz/componentes/Dialogos.h"
#include "interfaz/componentes/Toolbar.h"
#include "interfaz/componentes/LogPanel.h"
#include "interfaz/paneles/PanelGrafos.hpp"
#include "interfaz/paneles/PanelAeroGrafos.hpp"
#include "interfaz/paneles/PanelIsomorfismo.hpp"
#include "interfaz/paneles/Matrices.hpp"
#include "interfaz/lienzo/LienzoRed.hpp"
#include "interfaz/lienzo/LienzoAeroGrafos.hpp"
#include "interfaz/util/AnimacionUI.h"
#include "interfaz/util/AtajosTeclado.h"
#include "interfaz/ventanas/VentanaAyuda.h"

// Algorithm headers needed in dibujar()
#include "nucleo/algoritmos/Dijkstra.hpp"
#include "nucleo/algoritmos/Kruskal.hpp"
#include "nucleo/algoritmos/BFS.hpp"
#include "nucleo/algoritmos/DFS.hpp"

// imgui_internal.h needed for DockBuilder APIs and ImHashStr
#include "imgui_internal.h"

// ---------------------------------------------------------------------------
// aplica tema moderno, elegante y vibrante
// ---------------------------------------------------------------------------
void aplicarTemaCisco() {
    auto& style = ImGui::GetStyle();
    style.FrameRounding = 8.0f;
    style.GrabRounding  = 8.0f;
    style.WindowRounding = 12.0f;
    style.ChildRounding  = 8.0f;
    style.PopupRounding  = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing  = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(6, 6);
    style.WindowPadding = ImVec2(12, 12);
    style.CellPadding  = ImVec2(8, 6);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize  = 12.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize  = 1.0f;
    style.PopupBorderSize  = 1.0f;
    style.FrameBorderSize  = 0.0f;
    style.TabBorderSize    = 0.0f;

    ImVec4* colors = style.Colors;
    // Paleta moderna oscura con acentos cian y morado
    colors[ImGuiCol_Text]           = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]   = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
    colors[ImGuiCol_WindowBg]       = ImVec4(0.09f, 0.09f, 0.12f, 0.98f);
    colors[ImGuiCol_ChildBg]        = ImVec4(0.12f, 0.12f, 0.15f, 0.95f);
    colors[ImGuiCol_PopupBg]        = ImVec4(0.11f, 0.11f, 0.15f, 0.98f);
    colors[ImGuiCol_Border]         = ImVec4(0.25f, 0.25f, 0.35f, 0.50f);
    colors[ImGuiCol_BorderShadow]   = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]        = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.28f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBg]        = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]= ImVec4(0.09f, 0.09f, 0.12f, 0.80f);
    colors[ImGuiCol_MenuBarBg]      = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.25f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]= ImVec4(0.35f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]= ImVec4(0.45f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_CheckMark]      = ImVec4(0.00f, 0.90f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrab]     = ImVec4(0.00f, 0.70f, 0.60f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.00f, 0.90f, 0.70f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.18f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.24f, 0.24f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.32f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_Header]         = ImVec4(0.00f, 0.60f, 0.50f, 0.35f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.00f, 0.75f, 0.65f, 0.50f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.00f, 0.90f, 0.75f, 0.65f);
    colors[ImGuiCol_Separator]      = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]= ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.85f, 0.70f, 1.00f);
    colors[ImGuiCol_ResizeGrip]     = ImVec4(0.25f, 0.25f, 0.35f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]= ImVec4(0.00f, 0.75f, 0.65f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]= ImVec4(0.00f, 0.90f, 0.75f, 0.80f);
    colors[ImGuiCol_Tab]            = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]     = ImVec4(0.00f, 0.65f, 0.55f, 0.60f);
    colors[ImGuiCol_TabActive]      = ImVec4(0.00f, 0.75f, 0.65f, 0.80f);
    colors[ImGuiCol_TabUnfocused]   = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]= ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.85f, 0.70f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_PlotLines]      = ImVec4(0.00f, 0.90f, 0.70f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]= ImVec4(0.00f, 1.00f, 0.85f, 1.00f);
    colors[ImGuiCol_PlotHistogram]  = ImVec4(0.00f, 0.75f, 0.65f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]= ImVec4(0.00f, 0.90f, 0.70f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]  = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]= ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_TableRowBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]  = ImVec4(0.18f, 0.18f, 0.24f, 0.30f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.85f, 0.70f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.90f, 0.75f, 0.60f);
    colors[ImGuiCol_NavHighlight]   = ImVec4(0.00f, 0.85f, 0.70f, 0.60f);
    colors[ImGuiCol_NavWindowingHighlight]= ImVec4(0.00f, 0.85f, 0.70f, 0.60f);
    colors[ImGuiCol_NavWindowingDimBg]= ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_ModalWindowDimBg]= ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

// ---------------------------------------------------------------------------
// layout: izq info, centro (lienzo + matrices + log como tabs), der herramientas
// ---------------------------------------------------------------------------
void Interfaz::construirLayout(ImGuiID dock_id, ImVec2 tamano) {
    ImGuiID main = dock_id;

    // split izquierdo (info del grafo) - compacto
    ImGuiID izq = ImGui::DockBuilderSplitNode(main, ImGuiDir_Left, 0.13f, nullptr, &main);

    // split derecho (solo herramientas)
    ImGuiID der = ImGui::DockBuilderSplitNode(main, ImGuiDir_Right, 0.25f, nullptr, &main);

    // El centro tiene todo: Lienzo (activo), Matrices y Log como pestanas
    ImGui::DockBuilderDockWindow("Info del Grafo", izq);
    ImGui::DockBuilderDockWindow("Algoritmos", der);
    ImGui::DockBuilderDockWindow("Lienzo de Red", main);
    ImGui::DockBuilderDockWindow("Matrices", main);           // pestana centro
    ImGui::DockBuilderDockWindow("Registro del Kernel", main); // pestana centro

    // Asegurar que Lienzo sea la pestana activa por defecto
    ImGui::DockBuilderGetNode(main)->SelectedTabId = ImHashStr("Lienzo de Red");

    // Layout para AeroGrafos: mapa mundial ocupa el centro, panel de control a la derecha
    if (estado_ui.modo_actual == ModoApp::AeroGrafos) {
        ImGui::DockBuilderDockWindow("Info del Grafo", izq);
        ImGui::DockBuilderDockWindow("Opciones AeroGrafos", der);
        ImGui::DockBuilderDockWindow("Mapa AeroGrafos", main);
        ImGui::DockBuilderDockWindow("Registro del Kernel", main);
        ImGui::DockBuilderGetNode(main)->SelectedTabId = ImHashStr("Mapa AeroGrafos");
    }
}

// ---------------------------------------------------------------------------
// dibujar: toolbar, workspace, paneles acoplables, status bar
// ---------------------------------------------------------------------------
void Interfaz::dibujar(Grafo& red, GLFWwindow* ventana) {

    static bool theme_set = false;
    if (!theme_set) { aplicarTemaCisco(); theme_set = true; }

    AtajosTeclado::procesar(red, historial, estado_ui, estado_grafos);

    if (estado_grafos.anim_estado.activa && !estado_grafos.anim_estado.pausada) {
        float dt = ImGui::GetIO().DeltaTime;
        estado_grafos.anim_estado.timer_paso += dt;
        if (estado_grafos.anim_estado.particula.activa) {
            estado_grafos.anim_estado.particula.progreso += dt / estado_grafos.anim_estado.particula.duracion;
            if (estado_grafos.anim_estado.particula.progreso >= 1.0f) {
                estado_grafos.anim_estado.particula.progreso = 1.0f;
                estado_grafos.anim_estado.particula.activa  = false;
            }
        }
        if (estado_grafos.anim_estado.timer_paso >= estado_grafos.anim_estado.velocidad_paso && estado_grafos.anim_estado.activa) {
            estado_grafos.anim_estado.timer_paso -= estado_grafos.anim_estado.velocidad_paso;
            estado_grafos.anim_estado.paso_actual++;
            if (estado_grafos.anim_estado.paso_actual >= (int)estado_grafos.anim_estado.pasos.size()) {
                estado_grafos.anim_estado.activa = false;
                AnimacionUI::finalizar(*this);
                if (estado_ui.herramienta_activa == EstadoUI::CatRutas) {
                    auto res = Algoritmos::dijkstra(red, estado_grafos.dijkstra_origen, estado_grafos.dijkstra_destino, estado_grafos.dijkstra_usar_latencia);
                    estado_grafos.ruta_optima = res.ruta;
                    estado_grafos.dijkstra_costo_total = res.costo_total;
                    estado_grafos.mostrar_mst = false;
                } else if (estado_ui.herramienta_activa == EstadoUI::CatArbol) {
                    auto res = Algoritmos::Kruskal::kruskal(red);
                    estado_grafos.aristas_mst = res.aristas_mst;
                    estado_grafos.mostrar_mst = true;
                    estado_grafos.ruta_optima.clear();
                } else if (estado_ui.herramienta_activa == EstadoUI::CatBusqueda) {
                    estado_grafos.bfs_resultado = Algoritmos::BFS::bfs(red, estado_grafos.bfs_nodo_inicio);
                    estado_grafos.dfs_resultado = Algoritmos::DFS::dfs(red, estado_grafos.dfs_nodo_inicio);
                }
            } else {
                const PasoAnimacion& paso = estado_grafos.anim_estado.pasos[estado_grafos.anim_estado.paso_actual];
                AnimacionUI::aplicarPaso(*this, paso);
                if (paso.arista_origen >= 0 && paso.arista_destino >= 0) {
                    Nodo* o = red.obtenerNodo(paso.arista_origen);
                    Nodo* d = red.obtenerNodo(paso.arista_destino);
                    if (o && d) {
                        estado_grafos.anim_estado.particula.activa     = true;
                        estado_grafos.anim_estado.particula.pos_inicio = o->posicion;
                        estado_grafos.anim_estado.particula.pos_fin    = d->posicion;
                        estado_grafos.anim_estado.particula.progreso   = 0.0f;
                        estado_grafos.anim_estado.particula.duracion   = estado_grafos.anim_estado.velocidad_paso * 0.8f;
                        switch (paso.accion) {
                            case PasoAnimacion::EXPLORAR:
                                estado_grafos.anim_estado.particula.color = IM_COL32(0, 200, 255, 255); break;
                            case PasoAnimacion::CONFIRMAR:
                                estado_grafos.anim_estado.particula.color = IM_COL32(255, 180, 0, 255); break;
                            case PasoAnimacion::DESCARTAR:
                                estado_grafos.anim_estado.particula.color = IM_COL32(255, 60, 60, 200); break;
                            default:
                                estado_grafos.anim_estado.particula.color = IM_COL32(0, 255, 180, 255); break;
                        }
                    }
                }
            }
        }
    }

    ImGuiViewport* vp = ImGui::GetMainViewport();
    float toolbar_h = 40.0f;

    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, toolbar_h));
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 5));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.11f, 1.0f));
    ImGui::Begin("##toolbar", nullptr,
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4);
    if (ImGui::Button(ICON_FA_BARS, ImVec2(30, 28))) {
        ImGui::OpenPopup("popup_archivo");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Archivo");
    if (ImGui::BeginPopup("popup_archivo")) {
        MenuPrincipal::contenido(*this, red, ventana);
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    ImGui::SameLine(0, 6);
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.4f, 1.0f), "|");
    ImGui::SameLine(0, 6);

    // Categorias  y modos
    Toolbar::dibujar(estado_ui, false);

    ImGui::End();

    float status_h  = 26.0f;
    ImVec2 dock_pos(vp->WorkPos.x, vp->WorkPos.y + toolbar_h);
    ImVec2 dock_size(vp->WorkSize.x, vp->WorkSize.y - toolbar_h - status_h);

    ImGui::SetNextWindowPos(dock_pos);
    ImGui::SetNextWindowSize(dock_size);
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

    ImGuiID dock_id = ImGui::GetID("GraphCoreDock");
    ImGui::DockSpace(dock_id);

    // reconstruir layout al cambiar modo
    if (estado_ui.modo_actual != ultimo_modo_workspace) {
        ultimo_modo_workspace = estado_ui.modo_actual;
        ImGui::DockBuilderRemoveNode(dock_id);
        ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
        construirLayout(dock_id, dock_size);
        ImGui::DockBuilderFinish(dock_id);
    }

    static bool layout_init = false;
    if (!layout_init) {
        layout_init = true;
        ImGui::DockBuilderRemoveNode(dock_id);
        ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dock_id, dock_size);
        construirLayout(dock_id, dock_size);
        ImGui::DockBuilderFinish(dock_id);
        ImGui::SetWindowFocus("Lienzo de Red");
    }
    ImGui::End();

    // ventanas acoplables
    if (estado_ui.modo_actual == ModoApp::AeroGrafos) {
        PanelAeroGrafos::dibujar(*this, red);
        LienzoAeroGrafos::dibujar(red, *this);
    } else {
        PanelGrafos::sidebarInfo(*this, red);
        PanelGrafos::panelContextual(*this, red);
        LienzoRed::dibujar(red, *this);
        Matrices::dibujar(red, *this);
    }
    LogPanel::dibujar(*this);

    // dialogos modales
    if (estado_ui.mostrar_acerca_de) {
        ImGui::OpenPopup("Acerca de");
        estado_ui.mostrar_acerca_de = false;
    }
    Dialogos::acercaDe();
    Dialogos::fallbackCargar(*this, red);
    Dialogos::fallbackGuardar(*this, red);
    VentanaAyuda::dibujar(estado_ui);
    VentanaAyuda::dibujarTutorialRapido(estado_ui);

    StatusBar::dibujar(*this);
}
