#include "Sonidos.hpp"

bool Sonidos::abrir(float vol) {
    volumen = vol;

    ma_engine_config cfg = ma_engine_config_init();
    cfg.noAutoStart = MA_FALSE;

    if (ma_engine_init(&cfg, &motor) != MA_SUCCESS) return false;

    ma_engine_set_volume(&motor, 1.0f);

    buffers[Tipo::VISITAR_NODO]    = generarPing(880.0f, 0.10f);
    buffers[Tipo::CONFIRMAR_RUTA]  = generarAcordeAscendente({523.25, 659.25, 783.99}, 0.30f);
    buffers[Tipo::DESCARTAR]       = generarDescenso(440.0f, 220.0f, 0.18f);
    buffers[Tipo::ALGORITMO_FIN]   = generarFanfarria(0.40f);
    buffers[Tipo::PAQUETE_ENVIADO] = generarChirrido(600.0f, 1200.0f, 0.07f);
    buffers[Tipo::NODO_CAIDO]      = generarAlarmaSuave(440.0f, 550.0f, 0.22f);
    buffers[Tipo::ARISTA_SATURADA] = generarRumble(80.0f, 0.28f);
    buffers[Tipo::ERROR_SONIDO]    = generarTonoDesafinado(300.0f, 310.0f, 0.18f);
    buffers[Tipo::CLICK_MENU]      = generarClick(1000.0f, 0.03f);
    buffers[Tipo::TRIUNFO_DIJKSTRA]= generarTriunfo(0.60f);

    for (int i = 0; i < Tipo::COUNT; i++) {
        auto& buf  = buffers[i];
        auto& slot = sonidos[i];
        if (buf.datos.empty()) continue;

        ma_audio_buffer_config ab_cfg = ma_audio_buffer_config_init(
            ma_format_f32, 1,
            (ma_uint64)buf.datos.size(),
            (const void*)buf.datos.data(), NULL);

        if (ma_audio_buffer_init(&ab_cfg, &slot.buffer) != MA_SUCCESS) continue;

        ma_result r = ma_sound_init_from_data_source(
            &motor, (ma_data_source*)&slot.buffer, 0, NULL, &slot.sound);
        if (r != MA_SUCCESS) {
            ma_audio_buffer_uninit(&slot.buffer);
            continue;
        }

        ma_sound_set_volume(&slot.sound, volumen);
        slot.inicializado = true;
    }

    backend_info();
    iniciado = true;
    return true;
}

void Sonidos::reproducir(Tipo tipo) {
    if (!iniciado || tipo >= COUNT) return;
    if (buffers[tipo].datos.empty()) return;

    limpiarTerminados();

    auto& slot = sonidos[tipo];
    if (!slot.inicializado) return;

    ma_sound_stop(&slot.sound);
    ma_sound_seek_to_pcm_frame(&slot.sound, 0);
    ma_sound_start(&slot.sound);
    slot.activo        = true;
    slot.tick_arranque = tick_global;
}

void Sonidos::setVolumen(float v) {
    volumen = v;
    if (!iniciado) return;
    for (int i = 0; i < Tipo::COUNT; i++) {
        if (sonidos[i].inicializado)
            ma_sound_set_volume(&sonidos[i].sound, volumen);
    }
}

void Sonidos::setVolumenInt(int v) {
    setVolumen(v / 100.0f);
}

bool Sonidos::probar() {
    if (!iniciado) return false;
    auto& slot = sonidos[ALGORITMO_FIN];
    if (!slot.inicializado) return false;
    ma_device* pDevice = ma_engine_get_device(&motor);
    if (pDevice && pDevice->pContext && pDevice->pContext->backend == ma_backend_null)
        return false;
    reproducir(ALGORITMO_FIN);
    return true;
}

void Sonidos::cerrar() {
    if (!iniciado) return;
    for (int i = 0; i < Tipo::COUNT; i++) {
        auto& slot = sonidos[i];
        if (!slot.inicializado) continue;
        ma_sound_stop(&slot.sound);
        ma_sound_uninit(&slot.sound);
        ma_audio_buffer_uninit(&slot.buffer);
        slot.inicializado = false;
    }
    ma_engine_uninit(&motor);
    iniciado    = false;
    tick_global = 0;
}

void Sonidos::backend_info() {
    ma_device* pDevice = ma_engine_get_device(&motor);
    if (pDevice && pDevice->pContext) {
        const char* name = ma_get_backend_name(pDevice->pContext->backend);
        if (name) snprintf(nombre_backend, sizeof(nombre_backend), "%s", name);
    } else {
        snprintf(nombre_backend, sizeof(nombre_backend), "desconocido");
    }
}

void Sonidos::limpiarTerminados() {
    tick_global++;
    for (int i = 0; i < Tipo::COUNT; i++) {
        auto& slot = sonidos[i];
        if (!slot.activo || !slot.inicializado) continue;
        if (slot.tick_arranque >= tick_global - 1) continue;
        if (ma_sound_at_end(&slot.sound) == MA_TRUE) {
            slot.activo = false;
        }
    }
}

// generacion de sonido

AudioBuffer Sonidos::generarPing(double freq, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 200;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double env = adsr(t, dur, 0.003, 0.02, 0.5, 0.03);
        double v = sin(2.0 * PI * freq * t)
                 + 0.35 * sin(2.0 * PI * freq * 3.0 * t);
        buf.datos[i] = (float)(v * env * 0.45);
    }
    return buf;
}

AudioBuffer Sonidos::generarAcordeAscendente(const std::vector<double>& freqs, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 5;
    buf.datos.resize(n);

    double dt = dur / freqs.size();
    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        int idx = (int)(t / dt);
        if (idx >= (int)freqs.size()) idx = (int)freqs.size() - 1;

        double env = adsr(fmod(t, dt), dt, 0.004, 0.02, 0.6, 0.025);
        double fade_total = 1.0 - (t / dur) * 0.25;
        double v = sin(2.0 * PI * freqs[idx] * t)
                 + 0.25 * sin(2.0 * PI * freqs[idx] * 2.0 * t);
        buf.datos[i] = (float)(v * env * fade_total * 0.40);
    }
    return buf;
}

AudioBuffer Sonidos::generarDescenso(double freq_inicio, double freq_fin, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 50;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double f = freq_inicio + (freq_fin - freq_inicio) * (t / dur);
        double env = adsr(t, dur, 0.005, 0.015, 0.7, 0.03);
        double v = sin(2.0 * PI * f * t);
        buf.datos[i] = (float)(v * env * 0.40);
    }
    return buf;
}

AudioBuffer Sonidos::generarFanfarria(double dur) {
    return generarAcordeAscendente({261.63, 329.63, 392.0, 523.25}, dur);
}

AudioBuffer Sonidos::generarTriunfo(double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 5;
    buf.datos.resize(n);

    double t_arpegio = dur * 0.30;
    double freq_arp[] = {261.63, 329.63, 392.0, 523.25};
    int n_notas = 4;
    double dt_arp = t_arpegio / n_notas;

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double env = adsr(t, dur, 0.003, 0.015, 0.8, 0.04);
        double v = 0.0;

        if (t < t_arpegio) {
            int idx = (int)(t / dt_arp);
            if (idx >= n_notas) idx = n_notas - 1;
            double env_nota = adsr(fmod(t, dt_arp), dt_arp, 0.002, 0.01, 0.7, 0.02);
            v = sin(2.0 * PI * freq_arp[idx] * t)
              + 0.25 * sin(2.0 * PI * freq_arp[idx] * 2.0 * t);
            v *= env_nota;
        } else {
            double sustain_vol = 1.0 - (t - t_arpegio) / (dur - t_arpegio) * 0.3;
            for (int k = 0; k < n_notas; k++) {
                v += sin(2.0 * PI * freq_arp[k] * t);
            }
            v = v / n_notas + 0.2 * sin(2.0 * PI * freq_arp[0] * 4.0 * t);
            v *= sustain_vol;
        }

        buf.datos[i] = (float)(v * env * 0.45);
    }
    return buf;
}

AudioBuffer Sonidos::generarChirrido(double f_ini, double f_fin, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 200;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double f = f_ini + (f_fin - f_ini) * (t / dur);
        double env = adsr(t, dur, 0.001, 0.005, 0.8, 0.01);
        double v = sin(2.0 * PI * f * t);
        buf.datos[i] = (float)(v * env * 0.30);
    }
    return buf;
}

AudioBuffer Sonidos::generarAlarmaSuave(double f1, double f2, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 50;
    buf.datos.resize(n);
    double periodo = dur / 4.0;

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double f = (fmod(t, periodo) < periodo * 0.5) ? f1 : f2;
        double env = adsr(t, dur, 0.005, 0.015, 0.7, 0.03);
        double v = sin(2.0 * PI * f * t);
        buf.datos[i] = (float)(v * env * 0.35);
    }
    return buf;
}

AudioBuffer Sonidos::generarRumble(double freq, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 20;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double env = adsr(t, dur, 0.01, 0.03, 0.5, 0.04);
        double v = sin(2.0 * PI * freq * t)
                 + 0.4 * sin(2.0 * PI * freq * 3.0 * t);
        buf.datos[i] = (float)(v * env * 0.35);
    }
    return buf;
}

AudioBuffer Sonidos::generarTonoDesafinado(double f1, double f2, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 50;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double env = adsr(t, dur, 0.004, 0.02, 0.6, 0.025);
        double v = sin(2.0 * PI * f1 * t) + sin(2.0 * PI * f2 * t);
        buf.datos[i] = (float)(v * env * 0.25);
    }
    return buf;
}

AudioBuffer Sonidos::generarClick(double freq, double dur) {
    AudioBuffer buf;
    ma_uint32 sr = buf.freq_muestreo;
    ma_uint32 n  = (ma_uint32)(sr * dur);
    if (n < 10) n = sr / 300;
    buf.datos.resize(n);

    for (ma_uint32 i = 0; i < n; i++) {
        double t = (double)i / sr;
        double env = adsr(t, dur, 0.0005, 0.003, 0.3, 0.005);
        double v = sin(2.0 * PI * freq * t);
        buf.datos[i] = (float)(v * env * 0.35);
    }
    return buf;
}
