#pragma once

#include "../Grafo.hpp"
#include <vector>
#include <string>

namespace Algoritmos {

struct PropiedadesGrafo {
    bool es_conexo = false;
    bool es_arbol = false;
    bool es_completo = false;
    bool es_bipartito = false;
    bool es_regular = false;
    bool es_euleriano = false;
    int grado_regular = -1;
};

struct AnalisisCompleto {
    int num_nodos = 0;
    int num_aristas = 0;
    float densidad = 0.0f;
    float grado_promedio = 0.0f;
    int grado_max = 0;
    int grado_min = 0;
    int id_hub_max = -1;
    int id_hub_min = -1;
    float grado_desviacion = 0.0f;
    bool es_conexo = false;
    int num_componentes = 0;
    int tamano_componente_mas_grande = 0;
    float proporcion_conectada = 1.0f;
    float diametro_aproximado = 0.0f;
    float coeficiente_clustering_global = 0.0f;
    float heterogeneidad = 0.0f;
    std::string resumen;
    bool es_arbol = false;
    bool es_completo = false;
    bool es_bipartito = false;
    bool es_regular = false;
};

struct AnalizadorGrafo {

    static PropiedadesGrafo analizar(const Grafo& g);

    static AnalisisCompleto analisisDetallado(const Grafo& g);
};

}
