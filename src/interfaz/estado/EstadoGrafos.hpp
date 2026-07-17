#pragma once

#include "nucleo/Grafo.hpp"
#include "nucleo/algoritmos/Dijkstra.hpp"
#include "nucleo/algoritmos/Kruskal.hpp"
#include "nucleo/algoritmos/BFS.hpp"
#include "nucleo/algoritmos/DFS.hpp"
#include "nucleo/algoritmos/Ciclos.hpp"
#include "nucleo/algoritmos/Coloreo.hpp"
#include "nucleo/algoritmos/Isomorfismo.hpp"
#include "nucleo/algoritmos/Arbol.hpp"
#include "nucleo/algoritmos/Planaridad.hpp"
#include "nucleo/algoritmos/Planaridad.hpp"
#include "nucleo/algoritmos/ColorFractal.hpp"
#include "interfaz/util/Animacion.hpp"

struct EstadoGrafos {
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

    // Animacion paso a paso de recorrido
    Animacion::EstadoAnimacion anim_estado;

    // Dijkstra
    int                dijkstra_origen      = 0;
    int                dijkstra_destino     = 1;
    bool               dijkstra_usar_latencia = false;
    std::vector<float> dijkstra_tabla_dist;
    float              dijkstra_costo_total = 0.0f;

    // Kruskal
    std::vector<Arista> aristas_mst;
    bool                mostrar_mst = false;

    // BFS
    int                            bfs_nodo_inicio = 0;
    Algoritmos::BFS::ResultadoBFS  bfs_resultado;

    // DFS
    int                            dfs_nodo_inicio = 0;
    Algoritmos::DFS::ResultadoDFS  dfs_resultado;

    // Ciclos
    Algoritmos::ResultadoCiclos   resultado_ciclos;
    bool                          ciclo_analizado  = false;

    // Coloreo
    Algoritmos::ResultadoColoreo       resultado_coloreo;
    Algoritmos::ResultadoColoreo       resultado_welsh_powell;
    Algoritmos::ResultadoColorFractal  resultado_fractal;
    std::vector<int>                   colores_nodos;
    std::vector<uint32_t>              colores_fractales;
    bool                               mostrar_coloreo = false;
    bool                               modo_fractal    = false;
    bool                               ocultar_vertices_fractal = false;
    float                              fractal_tiempo  = 0.0f;

    // Planaridad
    Algoritmos::ResultadoPlanaridad resultado_planaridad;
    bool                            planar_analizado = false;
    bool                            mostrar_cruces   = true;

    // Isomorfismo
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

    // Arbol
    Algoritmos::Arbol::PropiedadesArbol arbol_props;
    bool        arbol_analizado       = false;
    int         arbol_raiz_id         = 0;
    bool        arbol_layout_aplicado = false;

    // resultado de ruta
    std::vector<int> ruta_optima;
};
