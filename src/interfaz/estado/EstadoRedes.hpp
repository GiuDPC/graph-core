#pragma once

#include "nucleo/SimuladorRed.hpp"
#include <string>

// Estado completo de simulacion de redes
struct EstadoRedes {
    // Simulador  
    SimuladorRed simulador;
    bool         sim_inicializada     = false;
    bool         mostrar_modal_inicio = false;

    // Flujo pendiente 
    int   flujo_origen  = 0;
    int   flujo_destino = 1;
    float flujo_mbps    = 10.0f;
    int   flujo_tipo    = 0;  // 0=HTTP, 1=VIDEO, 2=PING, 3=DDOS, 4=VoIP, 5=DNS
    float flujo_dur     = 10.0f;

    // Jitter 
    bool  simulacion_jitter   = false;
    float jitter_porcentaje   = 0.15f;

    // Inspector de paquetes 
    int   paquete_inspector_id = -1;  // ID del paquete en paquetes_activos
    bool  mostrar_inspector    = false;

    // Timeline 
    bool  mostrar_timeline     = true;

    //  Stats 
    bool  mostrar_stats        = true;
    int   preset_trafico       = 0;    // 0=Personalizado, 1=Videollamada, 2=Empresarial, 3=DDoS
};
