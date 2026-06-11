#pragma once
#include <map>
#include <vector>
#include <string>
#include <deque>

// Estado dinamico de la red (para simulacion en tiempo real)

struct EstadoNodo {
    float cpu_uso        = 0.0f;    // 0.0 - 1.0
    float memoria_uso    = 0.0f;    // 0.0 - 1.0
    float paquetes_rx    = 0.0f;    // paquetes/segundo recibidos
    float paquetes_tx    = 0.0f;    // paquetes/segundo enviados
    bool  activo         = true;    // false = nodo caido (failover)
    float uptime         = 0.0f;    // segundos desde inicio simulacion
    float tiempo_caida   = 0.0f;    // si activo=false, cuantos seg lleva caido
};

struct EstadoArista {
    float bandwidth_mbps  = 100.0f; // capacidad maxima (configurable)
    float uso_actual_mbps = 0.0f;   // trafico actual
    float latencia_ms     = 1.0f;   // latencia base
    float jitter_ms       = 0.0f;   // variacion de latencia
    float packet_loss     = 0.0f;   // probabilidad de perdida (0.0 - 1.0)
    bool  activa          = true;   // false = enlace caido
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

// Estado global de la simulacion
struct EstadoSimulacion {
    bool  activa         = false;
    float tiempo         = 0.0f;    // segundos desde inicio
    float velocidad      = 1.0f;    // multiplicador de velocidad

    std::map<int, EstadoNodo>                  nodos;
    std::map<std::pair<int,int>, EstadoArista> aristas;
    std::vector<FlujoTrafico>                  flujos;
    std::deque<EventoRed>                      log_eventos;  // ultimos 100 eventos

    void registrarEvento(float t, const std::string& msg,
                         EventoRed::Severidad sev = EventoRed::INFO) {
        log_eventos.push_back({t, msg, sev});
        if (log_eventos.size() > 100) log_eventos.pop_front();
    }
};
