#pragma once
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <cmath>
#include <cstdint>

// ── Historial de uso en anillo (para sparklines) ───────────────────────────
struct HistorialUso {
    static const int MAX_MUESTRAS = 60;  // 60 frames ~ 1 segundo a 60fps
    float muestras[MAX_MUESTRAS];
    int   indice = 0;
    int   total  = 0;

    void agregar(float v) {
        muestras[indice] = v;
        indice = (indice + 1) % MAX_MUESTRAS;
        if (total < MAX_MUESTRAS) total++;
    }

    float ultimo() const {
        if (total == 0) return 0.0f;
        return muestras[(indice - 1 + MAX_MUESTRAS) % MAX_MUESTRAS];
    }

    float promedio() const {
        if (total == 0) return 0.0f;
        float s = 0.0f;
        int n = total;
        for (int i = 0; i < n; i++) s += muestras[i];
        return s / n;
    }

    float maximo() const {
        float m = 0.0f;
        for (int i = 0; i < total; i++)
            if (muestras[i] > m) m = muestras[i];
        return m;
    }
};

// Estado dinamico de la red (para simulacion en tiempo real)
struct EstadoNodo {
    float cpu_uso        = 0.0f;    // 0.0 - 1.0
    float memoria_uso    = 0.0f;    // 0.0 - 1.0
    float paquetes_rx    = 0.0f;    // paquetes/segundo recibidos
    float paquetes_tx    = 0.0f;    // paquetes/segundo enviados
    float buffer_mb      = 0.0f;    // MB ocupados actualmente
    float buffer_max_mb  = 50.0f;   // capacidad maxima del buffer
    int   paquetes_cola  = 0;       // paquetes esperando en este nodo
    bool  activo         = true;    // false = nodo caido (failover)
    float uptime         = 0.0f;    // segundos desde inicio simulacion
    float tiempo_caida   = 0.0f;    // si activo=false, cuantos seg lleva caido
    HistorialUso hist_cpu;          // sparkline CPU
    HistorialUso hist_ram;          // sparkline RAM
};

struct EstadoArista {
    float bandwidth_mbps  = 100.0f; // capacidad maxima (configurable)
    float uso_actual_mbps = 0.0f;   // trafico actual
    float latencia_ms     = 1.0f;   // latencia base
    float jitter_ms       = 0.0f;   // variacion de latencia
    float packet_loss     = 0.0f;   // probabilidad de perdida (0.0 - 1.0)
    bool  activa          = true;   // false = enlace caido
    HistorialUso hist_uso;          // sparkline de uso %
};

// Un flujo de trafico simulado entre dos nodos
struct FlujoTrafico {
    int   origen_id;
    int   destino_id;
    float mbps;                     // ancho de banda consumido
    std::string tipo;               // "HTTP", "PING", "VIDEO", "DDOS"
    float tiempo_restante;          // segundos hasta que termina el flujo
};

// Evento del log de simulacion
struct EventoRed {
    float   timestamp;              // tiempo de simulacion
    std::string mensaje;
    enum Severidad { INFO, ADVERTENCIA, ERROR_RED } severidad;
};

// ── Estadisticas acumuladas de la red ──────────────────────────────────────
struct EstadisticasRed {
    float throughput_total_mbps     = 0.0f;
    float paquetes_perdidos_total   = 0.0f;
    float latencia_promedio_ms      = 0.0f;
    float jitter_promedio_ms        = 0.0f;
    HistorialUso hist_throughput;           // sparkline throughput global
    HistorialUso hist_perdida;              // sparkline packet loss rate

    void reset() {
        throughput_total_mbps = 0.0f;
        paquetes_perdidos_total = 0.0f;
        latencia_promedio_ms = 0.0f;
        jitter_promedio_ms = 0.0f;
    }
};

// ── Evento para timeline grafico ───────────────────────────────────────────
struct TimelineEvent {
    float tiempo;
    float duracion = 0.0f;          // 0 = instante
    std::string etiqueta;
    uint32_t color;                  // ARGB (se interpreta como ImU32 en render)
    enum Tipo { INSTANTE, SPIKE, CORTE, RESTAURACION, SOBRECARGA } tipo;
};

// --- Notificacion tipo "achievement" para feedback en canvas ---
struct Notificacion {
    float tiempo_real;              // ImGui::GetTime() real, no simulado
    float duracion = 3.0f;
    std::string mensaje;
    uint32_t color;                 // ARGB
};

// Estado global de la simulacion
struct EstadoSimulacion {
    bool  activa         = false;
    float tiempo         = 0.0f;    // segundos desde inicio
    float velocidad      = 1.0f;    // multiplicador de velocidad

    std::map<int, EstadoNodo>                  nodos;
    std::map<std::pair<int,int>, EstadoArista> aristas;
    std::vector<FlujoTrafico>                  flujos;
    std::deque<EventoRed>                      log_eventos;  // ultimos 100 eventos
    std::deque<TimelineEvent>                  timeline;     // eventos para timeline grafico
    EstadisticasRed                            stats;
    std::vector<Notificacion>                  notificaciones;

    void registrarEvento(float t, const std::string& msg,
                         EventoRed::Severidad sev = EventoRed::INFO) {
        log_eventos.push_back({t, msg, sev});
        if (log_eventos.size() > 100) log_eventos.pop_front();
    }

    void registrarTimeline(float t, const std::string& etiq, uint32_t col,
                           TimelineEvent::Tipo ttype, float dur = 0.0f) {
        timeline.push_back({t, dur, etiq, col, ttype});
        if (timeline.size() > 50) timeline.pop_front();
    }
};
