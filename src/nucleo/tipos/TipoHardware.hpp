#pragma once
#include <cstddef>

// Tipos de hardware de red
enum class TipoHardware {
    Servidor, Router, Switch, Firewall, Terminal
};

inline float latenciaHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor:  return 0.0f;
        case TipoHardware::Router:    return 5.0f;
        case TipoHardware::Switch:    return 1.0f;
        case TipoHardware::Firewall:  return 15.0f;
        case TipoHardware::Terminal:  return 0.0f;
        default:                      return 0.0f;
    }
}

inline const char* prefijoHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor:  return "SRV";
        case TipoHardware::Router:    return "RTR";
        case TipoHardware::Switch:    return "SW";
        case TipoHardware::Firewall:  return "FW";
        case TipoHardware::Terminal:  return "PC";
        default:                      return "N";
    }
}

inline const char* nombreHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor:  return "Servidor";
        case TipoHardware::Router:    return "Router";
        case TipoHardware::Switch:    return "Switch";
        case TipoHardware::Firewall:  return "Firewall";
        case TipoHardware::Terminal:  return "Terminal";
        default:                      return "Desconocido";
    }
}

// Constante para iterar sobre todos los tipos
constexpr int TOTAL_TIPOS_HARDWARE = 5;
