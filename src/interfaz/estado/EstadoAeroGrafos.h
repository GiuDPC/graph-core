#pragma once

#include "imgui.h"
#include "nucleo/datos/DatosMundo.h"
#include "nucleo/tipos/PasoAnimacion.h"
#include "interfaz/util/Animacion.h"
#include <vector>
#include <string>

// ============================================================================
// EstadoAeroGrafos — Estado del modo AeroGrafos
// ============================================================================

struct EstadoAeroGrafos {
    // ── Algoritmo activo ──
    enum class Algoritmo {
        RutaMasCorta,    // Dijkstra
        ConectarTodo,    // Kruskal/MST
        ExplorarNiveles, // BFS
        ExplorarTodo,    // DFS
        ColorearRegiones,// Coloreo
        RutaMantenimiento,// Euler
        VueltaAlMundo,   // Hamilton
        AnalizarRed      // Árbol/Ciclos
    };
    Algoritmo algoritmo_activo = Algoritmo::RutaMasCorta;

    // ── Ciudades seleccionadas ──
    int ciudad_origen = -1;
    int ciudad_destino = -1;

    // ── Resultados de algoritmos ──
    bool algoritmo_ejecutado = false;
    std::vector<int> ruta_resultado;          // Dijkstra/Euler/Hamilton: IDs ordenados
    float costo_total = 0.0f;                 // Dijkstra/Kruskal: peso total
    std::string descripcion_resultado;        // Texto descriptivo del resultado
    std::vector<std::pair<int,int>> aristas_mst; // Kruskal: pares (origen, destino)
    std::vector<int> orden_visita;            // BFS/DFS: orden de ciudades visitadas
    std::vector<int> colores_asignados;       // Coloreo: color index por cada ciudad (size=63)
    int num_colores_usados = 0;               // Coloreo: cuantos colores distintos

    // ── Comparativa Dijkstra vs BFS ──
    bool mostrar_comparativa = false;
    float costo_bfs = 0.0f;
    int saltos_bfs = 0;

    // ── Animación ──
    bool modo_animacion = false;
    Animacion::EstadoAnimacion animacion;

    // ── Estado del mapa ──
    ImVec2 centro_mapa = ImVec2(DatosMundo::ANCHO_VIRTUAL / 2.0f,
                                DatosMundo::ALTO_VIRTUAL / 2.0f); // centro virtual
    float zoom_mapa = 1.0f;
    
    // Cámara cinemática (Lerp)
    ImVec2 target_centro = centro_mapa;
    float target_zoom = 1.0f;
    bool interpolando_camara = false; // Solo verdadero por auto-encuadre

    bool arrastrando_mapa = false;
    ImVec2 ultimo_mouse_arrastre = ImVec2(0, 0);

    // ── Mensajes del canvas ──
    struct MensajeCanvas {
        std::string texto;
        double tiempo_restante;  // segundos hasta desaparecer
        ImU32 color;
    };
    std::vector<MensajeCanvas> mensajes;

    void agregarMensaje(const std::string& texto, ImU32 color = IM_COL32(255, 255, 255, 255),
                        float duracion = 4.0f) {
        mensajes.push_back({texto, duracion, color});
    }

    void actualizarMensajes(float dt) {
        for (auto it = mensajes.begin(); it != mensajes.end(); ) {
            it->tiempo_restante -= dt;
            if (it->tiempo_restante <= 0.0f) {
                it = mensajes.erase(it);
            } else {
                ++it;
            }
        }
    }

    // ── Tiempo global para animaciones ──
    float tiempo_reloj = 0.0f;

    // ── Simulación geopolítica ──
    bool restricciones_geopoliticas = false;
    static const int ID_MOSCU = 15; // Moscú (SVO)

    // ── Opciones de visualización ──
    bool mostrar_grid = true;
    bool mostrar_nombres = true;
    bool mostrar_todas_rutas = true;
    bool modo_noche = false;

    // ── Textura del mundo ──
    unsigned int id_textura_mundo = 0;
    int ancho_textura = 0;
    int alto_textura = 0;
    bool textura_cargada = false;

    // ── Cache de Análisis de Red (para Popup) ──
    struct AnalisisCache {
        int n = 0;
        int m = 0;
        float grado_prom = 0.0f;
        std::string hub_max_nombre;
        std::string hub_max_iata;
        int max_grado = 0;
        std::string hub_min_nombre;
        std::string hub_min_iata;
        int min_grado = 0;
    };
    AnalisisCache analisis_cache;
    bool mostrar_popup_analisis = false;
};
