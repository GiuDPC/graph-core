#pragma once

#include "nucleo/Grafo.h"
#include "nucleo/algoritmos/Dijkstra.h"
#include "nucleo/algoritmos/Kruskal.h"
#include "nucleo/algoritmos/BFS.h"
#include "nucleo/algoritmos/DFS.h"
#include "nucleo/algoritmos/Ciclos.h"
#include "nucleo/algoritmos/Coloreo.h"
#include "nucleo/algoritmos/Isomorfismo.h"
#include "nucleo/algoritmos/Arbol.h"
#include "nucleo/algoritmos/Planaridad.h"
#include "nucleo/algoritmos/ColorFractal.h"
#include "interfaz/util/Animacion.h"

// Estado completo de algoritmos de grafos
struct EstadoGrafos {
    // ── Modo de panel ─────────────────────────────────────────────────────
    enum class ModoPanel {
        General,
        Dijkstra,
        Kruskal,
        BFS,
        DFS,
        Ciclos,
        Coloreo,
        Isomorfismo,
        Arbol
    };
    ModoPanel modo_panel = ModoPanel::General;

    // ── Animacion paso a paso ──────────────────────────────────────────────
    Animacion::EstadoAnimacion anim_estado;

    // ── Dijkstra ───────────────────────────────────────────────────────────
    int                dijkstra_origen      = 0;
    int                dijkstra_destino     = 1;
    bool               dijkstra_usar_latencia = false;
    std::vector<float> dijkstra_tabla_dist;
    float              dijkstra_costo_total = 0.0f;

    // ── Kruskal ────────────────────────────────────────────────────────────
    std::vector<Arista> aristas_mst;
    bool                mostrar_mst = false;

    // ── BFS ────────────────────────────────────────────────────────────────
    int                            bfs_nodo_inicio = 0;
    Algoritmos::BFS::ResultadoBFS  bfs_resultado;

    // ── DFS ────────────────────────────────────────────────────────────────
    int                            dfs_nodo_inicio = 0;
    Algoritmos::DFS::ResultadoDFS  dfs_resultado;

    // ── Ciclos ─────────────────────────────────────────────────────────────
    Algoritmos::ResultadoCiclos   resultado_ciclos;
    bool                          ciclo_analizado  = false;

    // ── Coloreo ────────────────────────────────────────────────────────────
    Algoritmos::ResultadoColoreo       resultado_coloreo;
    Algoritmos::ResultadoColoreo       resultado_welsh_powell;
    Algoritmos::ResultadoColorFractal  resultado_fractal;
    std::vector<int>                   colores_nodos;       // indices de color para greedy
    std::vector<uint32_t>              colores_fractales;   // colores ARGB directos (modo fractal)
    bool                               mostrar_coloreo = false;
    bool                               modo_fractal    = false;  // false = greedy, true = fractal
    float                              fractal_tiempo  = 0.0f;   // para morphing

    // ── Planaridad ─────────────────────────────────────────────────────────
    Algoritmos::ResultadoPlanaridad resultado_planaridad;
    bool                            planar_analizado = false;
    bool                            mostrar_cruces   = true;

    // ── Isomorfismo ────────────────────────────────────────────────────────
    Grafo                                              grafo_iso_g2;
    Algoritmos::Isomorfismo::ResultadoIsomorfismo      resultado_iso;
    bool                                               iso_analizado    = false;
    bool                                               iso_editando_g2  = false;

    void resetGrafoIsomorfismo() {
        grafo_iso_g2.limpiar();
        resultado_iso = {};
        iso_analizado  = false;
        iso_editando_g2 = false;
    }

    // ── Arbol ──────────────────────────────────────────────────────────────
    Algoritmos::Arbol::PropiedadesArbol arbol_props;
    bool        arbol_analizado       = false;
    int         arbol_raiz_id         = 0;
    bool        arbol_layout_aplicado = false;

    // ── Resultado de ruta ──────────────────────────────────────────────────
    std::vector<int> ruta_optima;
};
