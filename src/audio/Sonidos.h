#pragma once

// Sistema de audio para graph-core
// Usa miniaudio.h (header-only) para generar sonidos sintéticos
// SIN archivos de audio externos — todo se genera por código

#include "miniaudio.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <memory>
#include <vector>

// Forward declare implementation struct
struct AudioBuffer {
    std::vector<float> datos;   // PCM mono float32
    ma_uint32          freq_muestreo = 48000;
};

// Reproducir sonido SIN bloqueo y SIN esperar a que termine
// Usa un hilo separado para no trabar el frame loop
class Sonidos {
public:
    enum Tipo {
        VISITAR_NODO,      // ping ascendente corto
        CONFIRMAR_RUTA,    // arpegio feliz
        DESCARTAR,         // buzz grave
        ALGORITMO_FIN,     // fanfarria
        PAQUETE_ENVIADO,   // whoosh
        NODO_CAIDO,        // alarma
        ARISTA_SATURADA,   // zumbido molesto
        ERROR_SONIDO,      // error buzzer
        CLICK_MENU,        // click sutil
        COUNT              // total de sonidos
    };

    Sonidos() {}
    ~Sonidos() { cerrar(); }

    bool abrir(float vol = 0.5f) {
        volumen = vol;
        ma_engine_config cfg = ma_engine_config_init();

        if (ma_engine_init(&cfg, &motor) != MA_SUCCESS) {
            return false;
        }

        // Forzar volumen global del engine para evitar que silencie nuestros sonidos
        ma_engine_set_volume(&motor, 1.0f);

        // Generar buffers de audio para cada tipo de sonido
        buffers[Tipo::VISITAR_NODO]    = generarTono(880.0f,  0.08f, ma_waveform_type_sine);
        buffers[Tipo::CONFIRMAR_RUTA]  = generarArpegio({660, 880, 1100, 1320}, 0.25f);
        buffers[Tipo::DESCARTAR]       = generarTono(220.0f,  0.15f, ma_waveform_type_sawtooth);
        buffers[Tipo::ALGORITMO_FIN]   = generarArpegio({523, 659, 784, 1047}, 0.4f);
        buffers[Tipo::PAQUETE_ENVIADO] = generarTono(1200.0f, 0.05f, ma_waveform_type_sine);
        buffers[Tipo::NODO_CAIDO]      = generarAlarma();
        buffers[Tipo::ARISTA_SATURADA] = generarTono(150.0f,  0.3f,  ma_waveform_type_square);
        buffers[Tipo::ERROR_SONIDO]    = generarTono(180.0f,  0.2f,  ma_waveform_type_sawtooth);
        buffers[Tipo::CLICK_MENU]      = generarTono(600.0f,  0.03f, ma_waveform_type_sine);

        iniciado = true;
        return true;
    }

    void reproducir(Tipo tipo) {
        if (!iniciado || tipo >= COUNT) return;

        auto& buf = buffers[tipo];
        if (buf.datos.empty()) return;

        // Crear decoder desde los datos PCM en memoria
        ma_decoder* decoder = new ma_decoder();
        ma_decoder_config dec_cfg = ma_decoder_config_init(ma_format_f32, 1, buf.freq_muestreo);
        
        if (ma_decoder_init_memory(buf.datos.data(), buf.datos.size() * sizeof(float), 
                                    &dec_cfg, decoder) != MA_SUCCESS) {
            delete decoder;
            return;
        }

        // Crear sound desde el decoder
        ma_sound* sound = new ma_sound();
        // ma_sound_init_from_data_source requiere un ma_engine como primer param
        // y un ma_data_source como segundo
        if (ma_sound_init_from_data_source(&motor, (ma_data_source*)decoder, 0, NULL, sound) != MA_SUCCESS) {
            delete decoder;
            delete sound;
            return;
        }

        ma_sound_set_volume(sound, volumen);
        ma_sound_start(sound);

        // El sound se auto-limpia cuando termina de reproducirse
        // Guardamos el puntero para cleanup posterior
        sonidos_activos.push_back({sound, decoder});
        limpiarTerminados();
    }

    bool funciona() const { return iniciado; }
    void setVolumen(float v) { volumen = v; }
    float getVolumen() const { return volumen; }

    void cerrar() {
        if (!iniciado) return;
        for (auto& s : sonidos_activos) {
            if (s.sound) {
                ma_sound_stop(s.sound);
                ma_sound_uninit(s.sound);
                delete s.sound;
            }
            if (s.decoder) delete s.decoder;
        }
        sonidos_activos.clear();
        ma_engine_uninit(&motor);
        iniciado = false;
    }

private:
    struct SoundEntry {
        ma_sound*   sound;
        ma_decoder* decoder;
    };

    ma_engine motor;
    AudioBuffer buffers[Tipo::COUNT];
    std::vector<SoundEntry> sonidos_activos;
    float volumen = 0.3f;
    bool iniciado = false;

    // Generar un tono simple (sine/square/sawtooth)
    AudioBuffer generarTono(double freq_hz, double duracion_seg, ma_waveform_type forma) {
        AudioBuffer buf;
        ma_uint32 sr = buf.freq_muestreo;
        ma_uint32 total_frames = (ma_uint32)(sr * duracion_seg);
        if (total_frames < 10) total_frames = sr / 100; // minimo 10ms

        buf.datos.resize(total_frames);

        for (ma_uint32 i = 0; i < total_frames; i++) {
            double t = (double)i / sr;
            double fase = 2.0 * M_PI * freq_hz * t;
            double valor = 0.0;

            switch (forma) {
                case ma_waveform_type_sine:
                    valor = sin(fase);
                    break;
                case ma_waveform_type_square:
                    valor = sin(fase) >= 0 ? 1.0 : -1.0;
                    break;
                case ma_waveform_type_sawtooth:
                    valor = 2.0 * (fmod(freq_hz * t, 1.0)) - 1.0;
                    break;
                default:
                    valor = sin(fase);
                    break;
            }

            // Envelope ADSR simple: fade in/out para evitar clicks
            double envelope = 1.0;
            double fade_len = 0.005; // 5ms fade
            if (t < fade_len) envelope = t / fade_len;
            if (t > duracion_seg - fade_len) envelope = (duracion_seg - t) / fade_len;

            buf.datos[i] = (float)(valor * envelope * 0.8);
        }

        return buf;
    }

    // Generar arpegio (secuencia de notas)
    AudioBuffer generarArpegio(const std::vector<double>& frecuencias, double duracion_total) {
        AudioBuffer buf;
        ma_uint32 sr = buf.freq_muestreo;
        double dt = duracion_total / frecuencias.size();
        ma_uint32 total_frames = (ma_uint32)(sr * duracion_total);
        if (total_frames < 10) total_frames = sr / 10;
        buf.datos.resize(total_frames);

        for (ma_uint32 i = 0; i < total_frames; i++) {
            double t = (double)i / sr;
            int idx_nota = (int)(t / dt);
            if (idx_nota >= (int)frecuencias.size()) idx_nota = (int)frecuencias.size() - 1;
            double freq = frecuencias[idx_nota];
            double fase = 2.0 * M_PI * freq * t;
            double valor = sin(fase);

            // Envelope
            double envelope = 1.0;
            double fade = 0.003;
            double t_nota = fmod(t, dt);
            if (t_nota < fade) envelope = t_nota / fade;
            if (t_nota > dt - fade) envelope = (dt - t_nota) / fade;

            // Decaimiento suave del arpegio completo
            double fade_total = 1.0 - (t / duracion_total) * 0.3;

            buf.datos[i] = (float)(valor * envelope * fade_total * 0.8);
        }

        return buf;
    }

    // Alarma alternante (como un teléfono)
    AudioBuffer generarAlarma() {
        AudioBuffer buf;
        ma_uint32 sr = buf.freq_muestreo;
        double duracion = 0.3;
        ma_uint32 total_frames = (ma_uint32)(sr * duracion);
        buf.datos.resize(total_frames);

        for (ma_uint32 i = 0; i < total_frames; i++) {
            double t = (double)i / sr;
            double freq = (fmod(t, 0.15) < 0.075) ? 880.0 : 660.0;
            double valor = sin(2.0 * M_PI * freq * t);

            double envelope = 1.0;
            if (t < 0.005) envelope = t / 0.005;
            if (t > duracion - 0.005) envelope = (duracion - t) / 0.005;

            buf.datos[i] = (float)(valor * envelope * 0.8);
        }

        return buf;
    }

    void limpiarTerminados() {
        auto it = sonidos_activos.begin();
        while (it != sonidos_activos.end()) {
            if (ma_sound_is_playing(it->sound) == MA_FALSE) {
                ma_sound_uninit(it->sound);
                delete it->sound;
                delete it->decoder;
                it = sonidos_activos.erase(it);
            } else {
                ++it;
            }
        }
    }
};
