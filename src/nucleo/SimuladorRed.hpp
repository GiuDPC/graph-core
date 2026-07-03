#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <deque>
#include <chrono>
#include "Grafo.hpp"
#include "tipos/EstadoRed.hpp"
#include "algoritmos/Dijkstra.hpp"
#include "audio/Sonidos.hpp"

extern Sonidos g_sonidos;

// Colores de protocolo (uint32_t ARGB)
namespace ColoresProtocolo {
    inline uint32_t paraTipo(const std::string& tipo) {
        if (tipo == "HTTP")    return 0xFF4CAF50;
        if (tipo == "PING")    return 0xFF00BCD4;
        if (tipo == "VIDEO")   return 0xFF9C27B0;
        if (tipo == "DDOS")    return 0xFFF44336;
        if (tipo == "VOIP")    return 0xFFFF9800;
        if (tipo == "DNS")     return 0xFF2196F3;
        if (tipo == "FTP")     return 0xFF795548;
        return 0xFFB0BEC5;
    }
    inline const char* icono(const std::string& tipo) {
        if (tipo == "HTTP")    return "H";
        if (tipo == "PING")    return "P";
        if (tipo == "VIDEO")   return "V";
        if (tipo == "DDOS")    return "D";
        if (tipo == "VOIP")    return "A";
        if (tipo == "DNS")     return "?";
        if (tipo == "FTP")     return "F";
        return "\u2022";
    }
}

inline int prioridadProtocolo(const std::string& tipo) {
    if (tipo == "VOIP")  return 1;
    if (tipo == "VIDEO") return 2;
    if (tipo == "HTTP")  return 3;
    if (tipo == "DNS")   return 4;
    if (tipo == "PING")  return 5;
    if (tipo == "DDOS")  return 6;
    return 5;
}

// Motor de simulacion de red mejorado
class SimuladorRed {
public:
    EstadoSimulacion estado;

    bool eventos_automaticos = true;
    float probabilidad_spike  = 0.002f;

    void inicializar(const Grafo& g);
    void tick(Grafo& g, float dt);

    // API publica
    std::vector<std::pair<int, std::string>> tablaRuteo(const Grafo& g, int nodo_id);

    struct Paquete {
        int id = 0;
        int origen_id;
        int destino_id;
        float mbps;
        std::string tipo;
        float progreso = 0.0f;
        float tamaño_mb;
        int paso_actual = 0;
        std::vector<int> ruta;
        int prioridad = 5;
    };

    const std::vector<Paquete>& obtenerPaquetes() const { return paquetes_activos; }

    int totalPaquetesEnviados() const { return total_paquetes_enviados; }
    int totalPaquetesPerdidos() const { return total_paquetes_perdidos; }
    int totalPaquetesEntregados() const { return total_paquetes_entregados; }

    void enviarFlujo(int origen, int destino, float mbps,
                     const std::string& tipo, float duracion, Grafo& g);

    float usoArista(int origen, int destino) const;
    float perdidaEnArista(int origen, int destino) const;

    // Fallos / Restauraciones
    static float tiempoReal();
    void notificar(const std::string& msg, uint32_t color, float duracion = 3.0f);
    void simularFalloNodo(int nodo_id, Grafo& g);
    void simularFalloArista(int origen, int destino, Grafo& g);
    void restaurarNodo(int nodo_id, Grafo& g);
    void restaurarArista(int origen, int destino, Grafo& g);
    void enviarFlujoPreset(Grafo& g, int preset);
    void enviarTraceroute(Grafo& g, int origen, int destino);
    void simularTormenta(Grafo& g, float porcentaje = 0.30f, float duracion = 5.0f);

private:
    std::mt19937 gen{42};
    std::unordered_map<std::string, std::vector<int>> cache_rutas;
    float timer_paquetes = 0.0f;
    float timer_stats = 0.0f;
    int contador_paquetes = 0;
    int total_paquetes_enviados = 0;
    int total_paquetes_perdidos = 0;
    int total_paquetes_entregados = 0;

    float uniformeF(float lo, float hi);
    std::vector<int> obtenerRuta(const Grafo& g, int origen, int destino);
    void colectarEstadisticas(Grafo& g);
    void actualizarNodos(Grafo& g, float dt);
    void actualizarAristas(Grafo& g, float dt);
    std::vector<Paquete> paquetes_activos;
    void procesarColas(Grafo& g, float dt);
    void generarEventos(Grafo& g, float dt);

    struct MicroCorte {
        int origen, destino;
        float tiempo_restauracion;
    };
    std::vector<MicroCorte> microcortes_pendientes;
};
