#pragma once

#include "miniaudio.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
    constexpr double PI = 3.14159265358979323846;
}

// Buffer PCM mono float32
struct AudioBuffer {
    std::vector<float> datos;
    ma_uint32          freq_muestreo = 48000;
};

class Sonidos {
public:
    enum Tipo {
        VISITAR_NODO,
        CONFIRMAR_RUTA,
        DESCARTAR,
        ALGORITMO_FIN,
        PAQUETE_ENVIADO,
        NODO_CAIDO,
        ARISTA_SATURADA,
        ERROR_SONIDO,
        CLICK_MENU,
        TRIUNFO_DIJKSTRA,
        COUNT
    };

    Sonidos() {}
    ~Sonidos() { cerrar(); }

    bool abrir(float vol = 0.5f);
    void reproducir(Tipo tipo);

    bool   funciona()    const { return iniciado; }
    float  getVolumen() const { return volumen; }
    int    getVolumenInt() const { return (int)(volumen * 100.0f + 0.5f); }

    void setVolumen(float v);
    void setVolumenInt(int v);

    bool probar();
    const char* backendString() const { return nombre_backend; }
    void cerrar();

private:
    struct SoundSlot {
        ma_sound        sound;
        ma_audio_buffer buffer;
        bool            inicializado = false;
        bool            activo       = false;
        int             tick_arranque = 0;
    };

    SoundSlot   sonidos[Tipo::COUNT];
    AudioBuffer buffers[Tipo::COUNT];
    ma_engine   motor;
    float       volumen     = 0.5f;
    bool        iniciado    = false;
    int         tick_global = 0;
    char        nombre_backend[32] = {};

    void backend_info();
    void limpiarTerminados();

    // funciones auxiliares de generacion de sonido
    inline double adsr(double t, double duracion,
                       double attack = 0.005, double decay = 0.010,
                       double sustain_lvl = 0.7, double release = 0.015) {
        if (t < attack) return t / attack;
        if (t < attack + decay) return 1.0 - (1.0 - sustain_lvl) * (t - attack) / decay;
        if (t > duracion - release) return sustain_lvl * (duracion - t) / release;
        return sustain_lvl;
    }

    AudioBuffer generarPing(double freq, double dur);
    AudioBuffer generarAcordeAscendente(const std::vector<double>& freqs, double dur);
    AudioBuffer generarDescenso(double freq_inicio, double freq_fin, double dur);
    AudioBuffer generarFanfarria(double dur);
    AudioBuffer generarTriunfo(double dur);
    AudioBuffer generarChirrido(double f_ini, double f_fin, double dur);
    AudioBuffer generarAlarmaSuave(double f1, double f2, double dur);
    AudioBuffer generarRumble(double freq, double dur);
    AudioBuffer generarTonoDesafinado(double f1, double f2, double dur);
    AudioBuffer generarClick(double freq, double dur);
};
