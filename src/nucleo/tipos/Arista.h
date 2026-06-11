#pragma once

// Arista del grafo
struct Arista {
    int   origen_id;
    int   destino_id;
    float peso;
    float peso_actual;   // puede variar con jitter en modo redes
    bool  es_dirigida;

    Arista(int _origen, int _destino, float _peso = 1.0f, bool _dirigida = false)
        : origen_id(_origen), destino_id(_destino),
          peso(_peso), peso_actual(_peso), es_dirigida(_dirigida)
    {}
};
