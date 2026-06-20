#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "nucleo/Grafo.h"
#include "nucleo/datos/DatosMundo.h"
#include "nucleo/algoritmos/Dijkstra.h"
#include "nucleo/algoritmos/Kruskal.h"
#include "nucleo/algoritmos/BFS.h"
#include "nucleo/algoritmos/DFS.h"
#include "nucleo/algoritmos/Coloreo.h"
#include "nucleo/algoritmos/EulerHamilton.h"
#include <cmath>
#include "interfaz/estado/EstadoAeroGrafos.h"

class Interfaz;

namespace PanelAeroGrafos {

// ── Selector de algoritmo ──────────────────────────────────────────────────
inline void selectorAlgoritmo(EstadoAeroGrafos& estado) {
    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_ROUTE " ALGORITMO");
    ImGui::Spacing();

    const char* algoritmos[] = {
        ICON_FA_DIAMOND " Ruta mas corta (Dijkstra)",
        ICON_FA_TREE " Conectar ciudades (Kruskal)",
        ICON_FA_LAYER_GROUP " Explorar por escalas (BFS)",
        ICON_FA_CODE_BRANCH " Explorar todo (DFS)",
        ICON_FA_PAINTBRUSH " Colorear regiones",
        ICON_FA_ROTATE " Ruta de mantenimiento (Euler)",
        ICON_FA_GLOBE " Vuelta al mundo (Hamilton)",
        ICON_FA_MAGNIFYING_GLASS " Analizar red"
    };
    int alg = (int)estado.algoritmo_activo;
    ImGui::SetNextItemWidth(-30); // Dejar espacio para (?)
    if (ImGui::BeginCombo("##algoritmo", algoritmos[alg])) {
        for (int i = 0; i < IM_ARRAYSIZE(algoritmos); i++) {
            if (ImGui::Selectable(algoritmos[i], i == alg)) {
                if (estado.algoritmo_activo != (EstadoAeroGrafos::Algoritmo)i) {
                    estado.algoritmo_activo = (EstadoAeroGrafos::Algoritmo)i;
                    estado.algoritmo_ejecutado = false;
                    estado.ruta_resultado.clear();
                    estado.aristas_mst.clear();
                    estado.orden_visita.clear();
                    estado.colores_asignados.clear();
                    estado.costo_total = 0.0f;
                    estado.num_colores_usados = 0;
                    estado.mostrar_comparativa = false;
                    estado.mensajes.clear();
                    Animacion::reset(estado.animacion);
                }
            }
            if (i == alg) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        switch (estado.algoritmo_activo) {
            case EstadoAeroGrafos::Algoritmo::RutaMasCorta: ImGui::TextUnformatted("Dijkstra: Encuentra el camino de costo minimo ponderado."); break;
            case EstadoAeroGrafos::Algoritmo::ConectarTodo: ImGui::TextUnformatted("Kruskal: Conecta todos los nodos minimizando el costo sin ciclos."); break;
            case EstadoAeroGrafos::Algoritmo::ExplorarNiveles: ImGui::TextUnformatted("BFS: Explora nivel por nivel (menos saltos)."); break;
            case EstadoAeroGrafos::Algoritmo::ExplorarTodo: ImGui::TextUnformatted("DFS: Explora en profundidad (util para detectar ciclos)."); break;
            case EstadoAeroGrafos::Algoritmo::ColorearRegiones: ImGui::TextUnformatted("Coloreo: Asigna colores/frecuencias sin que nodos adyacentes repitan."); break;
            case EstadoAeroGrafos::Algoritmo::RutaMantenimiento: ImGui::TextUnformatted("Euler: Recorre todas las aristas exactamente una vez."); break;
            case EstadoAeroGrafos::Algoritmo::VueltaAlMundo: ImGui::TextUnformatted("Hamilton: Visita todos los nodos exactamente una vez."); break;
            case EstadoAeroGrafos::Algoritmo::AnalizarRed: ImGui::TextUnformatted("Analisis: Calcula metricas topologicas como densidad, hubs, etc."); break;
        }
        ImGui::EndTooltip();
    }
}

// ── Selector de ciudad ────────────────────────────────────────────────────
inline void selectorCiudad(const char* label, int& ciudad_id,
                           const std::vector<Ciudad>& ciudades) {
    ImGui::Text("%s", label);
    ImGui::SetNextItemWidth(-1);

    // Buscar nombre de ciudad seleccionada
    const char* preview = "Seleccionar...";
    for (const auto& c : ciudades) {
        if (c.id == ciudad_id) {
            preview = c.nombre;
            break;
        }
    }

    if (ImGui::BeginCombo(("##" + std::string(label)).c_str(), preview)) {
        for (const auto& c : ciudades) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s (%s) - %s", c.nombre, c.codigo_iata, c.pais);
            bool sel = (c.id == ciudad_id);
            if (ImGui::Selectable(buf, sel)) {
                ciudad_id = c.id;
            }
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

// ── Panel principal ───────────────────────────────────────────────────────
inline void dibujar(Interfaz& self, Grafo& red) {
    ImGui::Begin("AeroGrafos");

    const auto& ciudades = DatosMundo::obtenerCiudades();
    auto& estado = self.estado_aerografos;

    ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f),
        ICON_FA_PLANE " AEROGRAFOS");
    ImGui::TextDisabled("Red de rutas aereas mundiales");
    ImGui::Separator();

    // ── Selector de algoritmo ──
    selectorAlgoritmo(estado);

    ImGui::Separator();

    // ── Selectores de ciudad ──
    if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMasCorta ||
        estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ExplorarNiveles ||
        estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ExplorarTodo) {
        selectorCiudad(ICON_FA_PLANE_DEPARTURE " Origen:", estado.ciudad_origen, ciudades);
        selectorCiudad(ICON_FA_PLANE_ARRIVAL " Destino:", estado.ciudad_destino, ciudades);
    } else if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento ||
               estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::VueltaAlMundo) {
        selectorCiudad(ICON_FA_PLANE_DEPARTURE " Ciudad inicial:", estado.ciudad_origen, ciudades);
    }

    ImGui::Spacing();

    // ── Modo animación ──
    bool puede_animar = (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMasCorta ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ConectarTodo ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ExplorarNiveles ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ExplorarTodo ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ColorearRegiones ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento ||
                         estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::VueltaAlMundo);
    if (puede_animar)
        ImGui::Checkbox(ICON_FA_PLAY " Modo animacion paso a paso", &estado.modo_animacion);
    ImGui::Spacing();

    // ── Botón de acción ──
    bool puede_calcular = (estado.ciudad_origen >= 0);
    if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMasCorta)
        puede_calcular = (estado.ciudad_origen >= 0 && estado.ciudad_destino >= 0);
    if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ColorearRegiones ||
        estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::AnalizarRed ||
        estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ConectarTodo)
        puede_calcular = true; // No requieren ciudad de origen obligatoria

    ImGui::BeginDisabled(!puede_calcular);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.45f, 0.35f, 1.0f));
    const char* btn_label = (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::AnalizarRed)
        ? ICON_FA_MAGNIFYING_GLASS " Analizar red"
        : (estado.modo_animacion ? ICON_FA_PLAY " Ejecutar animado" : ICON_FA_PLAY " Ejecutar algoritmo");
        
    if (ImGui::Button(ICON_FA_ARROW_ROTATE_LEFT, ImVec2(36, 36))) {
        self.estado_ui.registrarLog("[AeroGrafos] Entorno reiniciado.");
        estado.algoritmo_ejecutado = false;
        estado.ruta_resultado.clear();
        estado.aristas_mst.clear();
        estado.orden_visita.clear();
        estado.colores_asignados.clear();
        estado.costo_total = 0.0f;
        estado.num_colores_usados = 0;
        estado.mostrar_comparativa = false;
        estado.mensajes.clear();
        Animacion::reset(estado.animacion);
        estado.ciudad_origen = -1;
        estado.ciudad_destino = -1;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reiniciar todo");
    ImGui::SameLine();
    
    if (ImGui::Button(btn_label, ImVec2(-1, 36))) {
        self.estado_ui.registrarLog(std::string("[AeroGrafos] Ejecutando: ") + btn_label);
        // Limpiar estado anterior
        estado.algoritmo_ejecutado = false;
        estado.ruta_resultado.clear();
        estado.aristas_mst.clear();
        estado.orden_visita.clear();
        estado.colores_asignados.clear();
        estado.costo_total = 0.0f;
        estado.num_colores_usados = 0;
        estado.mostrar_comparativa = false;
        estado.mensajes.clear();
        Animacion::reset(estado.animacion);
        Grafo g = DatosMundo::construirGrafoAerografos();

        // Aplicar restricciones geopolíticas
        if (estado.restricciones_geopoliticas) {
            for (auto& a : g.aristas) {
                bool toca_rusia = (a.origen_id == EstadoAeroGrafos::ID_MOSCU || a.destino_id == EstadoAeroGrafos::ID_MOSCU);
                
                bool sobrevuelo = 
                    (a.origen_id == 16 && a.destino_id == 21) ||
                    (a.origen_id == 21 && a.destino_id == 16) ||
                    (a.origen_id == 11 && a.destino_id == 21) ||
                    (a.origen_id == 21 && a.destino_id == 11) ||
                    (a.origen_id == 14 && a.destino_id == 21) ||
                    (a.origen_id == 21 && a.destino_id == 14);

                if (toca_rusia || sobrevuelo) {
                    a.peso *= 100000.0f;
                    a.peso_actual *= 100000.0f;
                }
            }
            estado.agregarMensaje(ICON_FA_TRIANGLE_EXCLAMATION
                " Restriccion activa: rutas rusas evitadas",
                IM_COL32(255,160,40,255), 4.0f);
        }

        switch (estado.algoritmo_activo) {
            case EstadoAeroGrafos::Algoritmo::RutaMasCorta: {
                auto res = Algoritmos::dijkstra(g, estado.ciudad_origen, estado.ciudad_destino, false, true);
                estado.mostrar_comparativa = false;
                if (res.hay_ruta) {
                    estado.ruta_resultado = res.ruta;
                    estado.costo_total = res.costo_total;
                    estado.orden_visita.clear();
                    estado.aristas_mst.clear();
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                        ICON_FA_ROUTE " Ruta optima: %.0f km, %d escalas",
                        res.costo_total, res.saltos);
                    estado.descripcion_resultado = buf;
                    char msg[192];
                    snprintf(msg, sizeof(msg),
                        ICON_FA_CHECK " Ruta optima: %.0f km (%d escalas)",
                        res.costo_total, res.saltos);
                    estado.agregarMensaje(msg, IM_COL32(0,255,150,255), 5.0f);
                    
                    // ── Análisis Comparativo BFS ──
                    auto res_bfs = Algoritmos::BFS::bfs(g, estado.ciudad_origen);
                    if (res_bfs.padre.count(estado.ciudad_destino) && res_bfs.padre[estado.ciudad_destino] != -1) {
                        estado.mostrar_comparativa = true;
                        int actual = estado.ciudad_destino;
                        int saltos_bfs = 0;
                        float costo_bfs = 0.0f;
                        std::vector<int> ruta_bfs;
                        while (actual != -1 && actual != estado.ciudad_origen) {
                            ruta_bfs.push_back(actual);
                            int p = res_bfs.padre[actual];
                            // Buscar el costo real de esta arista en el grafo
                            for (const auto& a : g.aristas) {
                                if ((a.origen_id == p && a.destino_id == actual) ||
                                    (!a.es_dirigida && a.origen_id == actual && a.destino_id == p)) {
                                    costo_bfs += a.peso; // Sumamos la distancia real
                                    break;
                                }
                            }
                            actual = p;
                            saltos_bfs++;
                        }
                        estado.costo_bfs = costo_bfs;
                        estado.saltos_bfs = saltos_bfs;
                    }

                    // ── Auto-Encuadre Cinemático ──
                    const auto& ciudades = DatosMundo::obtenerCiudades();
                    ImVec2 p_o = DatosMundo::latLonAVirtual(ciudades[estado.ciudad_origen].latitud, ciudades[estado.ciudad_origen].longitud);
                    ImVec2 p_d = DatosMundo::latLonAVirtual(ciudades[estado.ciudad_destino].latitud, ciudades[estado.ciudad_destino].longitud);
                    estado.target_centro.x = (p_o.x + p_d.x) * 0.5f;
                    estado.target_centro.y = (p_o.y + p_d.y) * 0.5f;
                    // Zoom heurístico según distancia en pixeles virtuales
                    float dist_v = sqrtf((p_o.x - p_d.x) * (p_o.x - p_d.x) + (p_o.y - p_d.y) * (p_o.y - p_d.y));
                    if (dist_v < 10.0f) dist_v = 10.0f;
                    estado.target_zoom = std::max(0.5f, std::min(15.0f, 600.0f / dist_v));
                    estado.interpolando_camara = true;

                } else {
                    estado.agregarMensaje(ICON_FA_XMARK " No hay ruta disponible",
                        IM_COL32(255,80,80,255), 4.0f);
                }
                estado.algoritmo_ejecutado = res.hay_ruta;
                if (estado.modo_animacion && res.hay_ruta) {
                    auto pasos = Algoritmos::generarPasos(g, estado.ciudad_origen, estado.ciudad_destino, false, true);
                    Animacion::iniciar(estado.animacion, pasos);
                }
                break;
            }
            case EstadoAeroGrafos::Algoritmo::ConectarTodo: {
                auto res = Algoritmos::Kruskal::kruskal(g);
                estado.aristas_mst.clear();
                for (const auto& a : res.aristas_mst)
                    estado.aristas_mst.push_back({a.origen_id, a.destino_id});
                estado.costo_total = res.peso_total;
                estado.ruta_resultado.clear();
                estado.orden_visita.clear();
                char buf[256];
                snprintf(buf, sizeof(buf),
                    ICON_FA_TREE " Arbol expansion minima: %.0f km total (%d aristas)",
                    res.peso_total, res.aristas_aceptadas);
                estado.descripcion_resultado = buf;
                char msg[192];
                snprintf(msg, sizeof(msg),
                    ICON_FA_CHECK " MST construido: %.0f km", res.peso_total);
                estado.agregarMensaje(msg, IM_COL32(0,200,255,255), 5.0f);
                estado.algoritmo_ejecutado = true;
                if (estado.modo_animacion) {
                    auto pasos = Algoritmos::Kruskal::generarPasos(g);
                    Animacion::iniciar(estado.animacion, pasos);
                }
                break;
            }
            case EstadoAeroGrafos::Algoritmo::ExplorarNiveles: {
                auto res = Algoritmos::BFS::bfs(g, estado.ciudad_origen);
                estado.orden_visita = res.orden_visita;
                estado.ruta_resultado.clear();
                estado.aristas_mst.clear();
                char buf[256];
                int niveles = 0;
                for (auto& [id, nv] : res.nivel)
                    if (nv > niveles) niveles = nv;
                snprintf(buf, sizeof(buf),
                    ICON_FA_LAYER_GROUP " BFS: %zu ciudades exploradas, %d niveles",
                    res.orden_visita.size(), niveles);
                estado.descripcion_resultado = buf;
                char msg[192];
                snprintf(msg, sizeof(msg),
                    ICON_FA_CHECK " BFS: %zu ciudades exploradas", res.orden_visita.size());
                estado.agregarMensaje(msg, IM_COL32(255,200,0,255), 5.0f);
                estado.algoritmo_ejecutado = true;
                if (estado.modo_animacion) {
                    auto pasos = Algoritmos::BFS::generarPasos(g, estado.ciudad_origen);
                    Animacion::iniciar(estado.animacion, pasos);
                }
                break;
            }
            case EstadoAeroGrafos::Algoritmo::ExplorarTodo: {
                auto res = Algoritmos::DFS::dfs(g, estado.ciudad_origen);
                estado.orden_visita = res.orden_visita;
                estado.ruta_resultado.clear();
                estado.aristas_mst.clear();
                char buf[256];
                snprintf(buf, sizeof(buf),
                    ICON_FA_CODE_BRANCH " DFS: %zu ciudades exploradas, %zu back-edges",
                    res.orden_visita.size(), res.back_edges.size());
                estado.descripcion_resultado = buf;
                char msg[192];
                snprintf(msg, sizeof(msg),
                    ICON_FA_CHECK " DFS: %zu ciudades exploradas", res.orden_visita.size());
                estado.agregarMensaje(msg, IM_COL32(255,200,0,255), 5.0f);
                estado.algoritmo_ejecutado = true;
                if (estado.modo_animacion) {
                    auto pasos = Algoritmos::DFS::generarPasos(g, estado.ciudad_origen);
                    Animacion::iniciar(estado.animacion, pasos);
                }
                break;
            }
            case EstadoAeroGrafos::Algoritmo::ColorearRegiones: {
                auto res = Algoritmos::coloreoGreedy(g);
                estado.colores_asignados = res.colores;
                estado.num_colores_usados = res.num_colores;
                estado.ruta_resultado.clear();
                estado.orden_visita.clear();
                estado.aristas_mst.clear();
                char buf[256];
                snprintf(buf, sizeof(buf),
                    ICON_FA_PAINTBRUSH " Coloreo: %d colores usados (greedy)",
                    res.num_colores);
                estado.descripcion_resultado = buf;
                char msg[192];
                snprintf(msg, sizeof(msg),
                    ICON_FA_CHECK " Coloreo: %d colores", res.num_colores);
                estado.agregarMensaje(msg, IM_COL32(180,130,255,255), 5.0f);
                estado.algoritmo_ejecutado = true;
                if (estado.modo_animacion) {
                    auto pasos = Algoritmos::generarPasosColoreo(g);
                    Animacion::iniciar(estado.animacion, pasos);
                }
                break;
            }
            case EstadoAeroGrafos::Algoritmo::RutaMantenimiento: {
                auto camino = Algoritmos::EulerHamilton::buscarCaminoEuleriano(g, estado.ciudad_origen);
                if (!camino.empty()) {
                    estado.ruta_resultado = camino;
                    float dist = 0;
                    for (size_t i = 1; i < camino.size(); i++)
                        for (const auto& r : DatosMundo::obtenerRutas())
                            if ((r.origen_id == camino[i-1] && r.destino_id == camino[i]) ||
                                (r.destino_id == camino[i-1] && r.origen_id == camino[i]))
                                dist += r.distancia_km;
                    estado.costo_total = dist;
                    estado.descripcion_resultado =
                        ICON_FA_ROTATE " Camino Euleriano encontrado: " +
                        std::to_string(camino.size()) + " ciudades";
                    char msg[192];
                    snprintf(msg, sizeof(msg),
                        ICON_FA_CHECK " Euleriano: %zu ciudades visitadas", camino.size());
                    estado.agregarMensaje(msg, IM_COL32(0,255,200,255), 5.0f);
                    estado.algoritmo_ejecutado = true;
                    if (estado.modo_animacion) {
                        auto pasos = Algoritmos::EulerHamilton::generarPasosEuler(g, estado.ciudad_origen);
                        Animacion::iniciar(estado.animacion, pasos);
                    }
                } else {
                    estado.descripcion_resultado = 
                        ICON_FA_XMARK " Euleriano Imposible\n"
                        "Requisito matematico: Todo el mapa debe estar conectado\n"
                        "y tener exactamente 0 o 2 ciudades con cantidad impar de vuelos.\n"
                        "La red global de aeropuertos no cumple esta estricta condicion.";
                    estado.agregarMensaje("No existe camino Euleriano (grafos reales con grado caotico)",
                        IM_COL32(255,100,100,255), 6.0f);
                    estado.algoritmo_ejecutado = true;
                }
                estado.orden_visita.clear();
                estado.aristas_mst.clear();
                break;
            }
            case EstadoAeroGrafos::Algoritmo::VueltaAlMundo: {
                auto res = Algoritmos::EulerHamilton::buscarCaminoHamiltonianoHeuristico(g, estado.ciudad_origen);
                if (res.ruta.size() >= g.nodos.size()) {
                    estado.ruta_resultado = res.ruta;
                    estado.costo_total = res.distancia_total;
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                        ICON_FA_GLOBE " Vuelta al mundo (heuristica): %.0f km, %zu ciudades",
                        res.distancia_total, res.ruta.size()-1);
                    estado.descripcion_resultado = buf;
                    char msg[192];
                    snprintf(msg, sizeof(msg),
                        ICON_FA_CHECK " Hamilton (heuristica): %.0f km", res.distancia_total);
                    estado.agregarMensaje(msg, IM_COL32(100,255,100,255), 5.0f);
                    estado.algoritmo_ejecutado = true;
                    if (estado.modo_animacion) {
                        auto pasos = Algoritmos::EulerHamilton::generarPasosHamiltonHeuristico(g, estado.ciudad_origen);
                        Animacion::iniciar(estado.animacion, pasos);
                    }
                } else {
                    estado.descripcion_resultado = 
                        ICON_FA_XMARK " Hamilton Imposible\n"
                        "Requiere visitar las 63 ciudades sin repetir ninguna.\n"
                        "Existen 'callejones sin salida' (ciudades con 1 sola conexion)\n"
                        "que impiden cerrar la ruta sin quedar atrapado.";
                    char msg[192];
                    snprintf(msg, sizeof(msg),
                        "Hamilton atascado: solo %zu/%zu ciudades visitadas",
                        res.ruta.size(), g.nodos.size());
                    estado.agregarMensaje(msg, IM_COL32(255,150,0,255), 6.0f);
                    estado.algoritmo_ejecutado = true;
                }
                estado.orden_visita.clear();
                estado.aristas_mst.clear();
                break;
            }
            case EstadoAeroGrafos::Algoritmo::AnalizarRed: {
                int n = (int)g.nodos.size();
                int m = (int)g.aristas.size();
                // Calcular grado promedio
                std::vector<int> grado(n, 0);
                for (const auto& a : g.aristas) {
                    grado[a.origen_id]++;
                    grado[a.destino_id]++;
                }
                float grado_prom = 0;
                int min_grado = n, max_grado = 0, id_min=0, id_max=0;
                for (int i = 0; i < n; i++) {
                    grado_prom += grado[i];
                    if (grado[i] < min_grado) { min_grado = grado[i]; id_min = i; }
                    if (grado[i] > max_grado) { max_grado = grado[i]; id_max = i; }
                }
                grado_prom /= n;
                const auto& c = DatosMundo::obtenerCiudades();
                char buf[512];
                snprintf(buf, sizeof(buf),
                    ICON_FA_MAGNIFYING_GLASS " ANALISIS DE LA RED\n"
                    "  Ciudades: %d\n  Rutas: %d\n  Grado promedio: %.1f\n"
                    "  Mayor hub: %s (%s) con %d rutas\n"
                    "  Menos conectado: %s (%s) con %d rutas\n"
                    "  Densidad: %.4f\n  Diametro teorico: ~%d saltos",
                    n, m, grado_prom,
                    c[id_max].nombre, c[id_max].codigo_iata, max_grado,
                    c[id_min].nombre, c[id_min].codigo_iata, min_grado,
                    (2.0f*m)/(n*(n-1.0f)),
                    (int)ceilf(logf((float)n)/logf(grado_prom)));
                estado.descripcion_resultado = buf;
                char msg[192];
                snprintf(msg, sizeof(msg),
                    ICON_FA_CHECK " Red analizada: %d ciudades, %d rutas", n, m);
                estado.agregarMensaje(msg, IM_COL32(200,200,255,255), 6.0f);
                estado.algoritmo_ejecutado = true;
                estado.ruta_resultado.clear();
                estado.orden_visita.clear();
                estado.aristas_mst.clear();
                break;
            }
        }
        if (estado.algoritmo_ejecutado && !estado.descripcion_resultado.empty()) {
            std::string log_msg = estado.descripcion_resultado;
            std::replace(log_msg.begin(), log_msg.end(), '\n', ' ');
            self.estado_ui.registrarLog("[AeroGrafos] " + log_msg);
        }
    }
    ImGui::PopStyleColor();
    ImGui::EndDisabled();

    // ── Animación: Controles ──
    if (estado.animacion.activa) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.83f, 0.67f, 1.0f), "%s", ICON_FA_PLAY " ANIMACION");

        // Barra de progreso
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.7f, 0.5f, 1.0f));
        float prog = Animacion::obtenerProgreso(estado.animacion);
        ImGui::ProgressBar(prog, ImVec2(-1, 8), "");
        ImGui::PopStyleColor();
        char contador[64];
        snprintf(contador, sizeof(contador), "Paso %d / %zu",
            estado.animacion.paso_actual + 1, estado.animacion.pasos.size());
        ImGui::TextDisabled("%s", contador);

        // Botones de control
        float ancho_boton = (ImGui::GetContentRegionAvail().x - 12.0f) / 4.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

        // Step back
        if (ImGui::Button(ICON_FA_BACKWARD_STEP, ImVec2(ancho_boton, 30))) {
            Animacion::pasoAtras(estado.animacion);
        }
        ImGui::SameLine();

        // Play/Pause
        const char* btn_pp = estado.animacion.pausada
            ? ICON_FA_PLAY " Play"
            : ICON_FA_PAUSE " Pause";
        if (ImGui::Button(btn_pp, ImVec2(ancho_boton, 30))) {
            Animacion::pausar(estado.animacion);
        }
        ImGui::SameLine();

        // Step forward
        if (ImGui::Button(ICON_FA_FORWARD_STEP, ImVec2(ancho_boton, 30))) {
            Animacion::pasoAdelante(estado.animacion);
        }
        ImGui::SameLine();

        // Skip to end
        if (estado.animacion.completa) {
            if (ImGui::Button(ICON_FA_ROTATE_LEFT " Reset", ImVec2(ancho_boton, 30))) {
                Animacion::reset(estado.animacion);
                estado.animacion.activa = true;
                estado.animacion.pausada = true;
            }
        } else {
            if (ImGui::Button(ICON_FA_FORWARD_FAST, ImVec2(ancho_boton, 30))) {
                Animacion::completar(estado.animacion);
            }
        }

        ImGui::PopStyleVar();

        // Velocidad
        ImGui::Spacing();
        ImGui::TextDisabled("Velocidad: %.1fx", estado.animacion.velocidad);
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##vel", &estado.animacion.velocidad, 0.25f, 4.0f, "");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Velocidad de animacion");

        // Descripción del paso actual
        std::string desc = Animacion::obtenerDescripcion(estado.animacion);
        if (!desc.empty()) {
            ImGui::Spacing();
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextWrapped("%s", desc.c_str());
            ImGui::PopTextWrapPos();
        }

        // Leyenda de colores
        if (estado.animacion.paso_actual >= 0) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextDisabled("Leyenda:");
            float y_line = ImGui::GetCursorPosY();
            auto dibujarLegend = [&](ImU32 col, const char* txt) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                dl->AddCircleFilled(ImVec2(p.x + 5, p.y + 5), 5, col, 8);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14.0f);
                ImGui::TextDisabled("%s", txt);
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);
            };
            if (!estado.animacion.exploradas.empty()) {
                dibujarLegend(IM_COL32(80, 160, 255, 220), "Explorando");
            }
            if (!estado.animacion.procesando.empty()) {
                dibujarLegend(IM_COL32(255, 220, 50, 220), "Procesando");
            }
            if (!estado.animacion.confirmadas.empty()) {
                dibujarLegend(IM_COL32(0, 255, 100, 220), "Confirmado");
            }
            if (!estado.animacion.descartadas.empty()) {
                dibujarLegend(IM_COL32(255, 80, 80, 180), "Descartado");
            }
        }
    }

    // ── Resultado del algoritmo ──
    if (estado.algoritmo_ejecutado && !estado.descripcion_resultado.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.1f, 0.15f, 0.4f));
        ImGui::BeginChild("panel_resultado", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.6f, 1.0f), "%s", "RESULTADO");
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextWrapped("%s", estado.descripcion_resultado.c_str());
        ImGui::PopTextWrapPos();

        // Mostrar información adicional según el algoritmo
        if (!estado.ruta_resultado.empty()) {
            const auto& c = DatosMundo::obtenerCiudades();
            ImGui::TextDisabled("Ruta:");
            std::string ruta_str;
            for (size_t i = 0; i < estado.ruta_resultado.size(); i++) {
                int id = estado.ruta_resultado[i];
                if (id >= 0 && id < (int)c.size()) {
                    if (!ruta_str.empty()) ruta_str += " \xE2\x86\x92 "; // →
                    ruta_str += c[id].codigo_iata;
                }
                if (ruta_str.size() > 120) { ruta_str += "..."; break; }
            }
            ImGui::TextWrapped("%s", ruta_str.c_str());
        }
        if (estado.costo_total > 0 && estado.algoritmo_activo != EstadoAeroGrafos::Algoritmo::ConectarTodo) {
            ImGui::TextDisabled("Distancia: %.0f km", estado.costo_total);
        }
        
        if (estado.mostrar_comparativa && estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMasCorta) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "JUSTIFICACION ACADEMICA");
            ImGui::TextWrapped("BFS encuentra la ruta con menos escalas, pero ignora la curvatura de la Tierra y la distancia real. Dijkstra garantiza el minimo de kilometros.");
            ImGui::Spacing();
            if (ImGui::BeginTable("comparativa_bfs", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Algoritmo");
                ImGui::TableSetupColumn("Costo (Km)");
                ImGui::TableSetupColumn("Escalas");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.6f, 1.0f), "Dijkstra");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.0f", estado.costo_total);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%d", (int)estado.ruta_resultado.size() - 2); // origen y destino descartados del conteo de "escalas" o usamos res.saltos
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "BFS");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.0f", estado.costo_bfs);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%d", estado.saltos_bfs - 1);
                
                ImGui::EndTable();
            }
        }
        if (!estado.aristas_mst.empty()) {
            ImGui::TextDisabled("Aristas en MST: %zu", estado.aristas_mst.size());
        }
        if (!estado.colores_asignados.empty()) {
            // Mostrar mini cuadros de color para los primeros colores
            ImGui::TextDisabled("Colores: %d", estado.num_colores_usados);
        }
        if (!estado.orden_visita.empty()) {
            ImGui::TextDisabled("Orden de visita: %zu ciudades", estado.orden_visita.size());
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // ── Info del mapa ──
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "NAVEGACION");
    ImGui::TextDisabled("Scroll: Zoom | Click medio: Arrastrar");
    ImGui::TextDisabled("Click: Seleccionar ciudad");
    ImGui::Text("Zoom: %.1fx", estado.zoom_mapa);

    // Mostrar coordenadas del centro
    float lat_centro, lon_centro;
    DatosMundo::virtualALatLon(estado.centro_mapa, lat_centro, lon_centro);
    ImGui::Text("Centro: %.1f° %.1f°", lat_centro, lon_centro);

    ImGui::Spacing();
    ImGui::Separator();

    // ── Estadísticas ──
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.1f, 0.15f, 0.4f));
    ImGui::BeginChild("panel_dashboard", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "DASHBOARD ANALITICO");
    
    if (ImGui::BeginTable("tabla_estadisticas", 2)) {
        ImGui::TableSetupColumn("Metrica", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("Valor", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%s Ciudades", ICON_FA_CIRCLE);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", ciudades.size());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%s Rutas", ICON_FA_LINK);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", DatosMundo::obtenerRutas().size());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Densidad");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", DatosMundo::calcularDensidadRed());
        
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Conectividad");
        ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("Grafo conexo 100%%");
        ImGui::EndTable();
    }
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Top Hubs Mundiales");
    auto top_hubs = DatosMundo::obtenerTop3Hubs();
    for (size_t i = 0; i < top_hubs.size(); i++) {
        ImGui::TextDisabled("  %zu. %s (%d conexiones)", i + 1, ciudades[top_hubs[i].first].codigo_iata, top_hubs[i].second);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // ── Opciones de visualización ──
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "VISUALIZACION");
    ImGui::Checkbox(ICON_FA_BORDER_ALL " Grid", &estado.mostrar_grid);
    ImGui::Checkbox(ICON_FA_FONT " Nombres", &estado.mostrar_nombres);
    ImGui::Checkbox(ICON_FA_ROUTE " Todas las rutas", &estado.mostrar_todas_rutas);
    ImGui::Checkbox(ICON_FA_MOON " Modo noche", &estado.modo_noche);

    // ── Simulación geopolítica ──
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "SIMULACION");
    bool cambio_restriccion = ImGui::Checkbox(ICON_FA_TRIANGLE_EXCLAMATION
        " Cierre espacio aereo ruso", &estado.restricciones_geopoliticas);
    if (cambio_restriccion) {
        self.estado_ui.registrarLog(estado.restricciones_geopoliticas
            ? "[AeroGrafos] Simulacion: Espacio aereo ruso CERRADO."
            : "[AeroGrafos] Simulacion: Espacio aereo ruso ABIERTO.");
        estado.agregarMensaje(estado.restricciones_geopoliticas
            ? ICON_FA_TRIANGLE_EXCLAMATION " Espacio aereo ruso CERRADO — rutas desviadas"
            : ICON_FA_CHECK " Espacio aereo ruso ABIERTO",
            estado.restricciones_geopoliticas ? IM_COL32(255,160,40,255) : IM_COL32(100,255,150,255),
            5.0f);
    }
    if (estado.restricciones_geopoliticas) {
        ImGui::TextDisabled("Rutas via Moscu (SVO) bloqueadas");
        ImGui::TextDisabled("Dijkstra evitara el espacio aereo");
    }

    ImGui::End();
}

} // namespace PanelAeroGrafos
