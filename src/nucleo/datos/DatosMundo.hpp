#pragma once

#include "imgui.h"
#include "../Grafo.hpp"
#include <vector>
#include <string>
#include <cmath>

// ============================================================================
// DatosMundo — Base de datos geografica mundial para AeroGrafos
//
// 63 ciudades reales con coordenadas verificadas
// 200+ rutas aereas con distancias calculadas via Haversine
// Grafo conexo garantizado
// Incluye Venezuela (Caracas, Maracaibo, Valencia, Barquisimeto, Merida, Pto. Ordaz)
// ============================================================================

struct Ciudad {
    int id;
    const char* nombre;
    const char* pais;
    float latitud;      // -90..90
    float longitud;     // -180..180
    const char* codigo_iata;  // 3 letras
    int poblacion_millones;   // para escalar nodos
};

struct RutaAerea {
    int origen_id;
    int destino_id;
    float distancia_km;  // calculada con Haversine
};

namespace DatosMundo {

// Constantes
constexpr float ANCHO_VIRTUAL = 2048.0f;
constexpr float ALTO_VIRTUAL  = 1024.0f;

// Inline: proyeccion (< 3 lineas)
inline ImVec2 latLonAVirtual(float lat, float lon) {
    float vx = (lon + 180.0f) / 360.0f * ANCHO_VIRTUAL;
    float vy = (90.0f - lat) / 180.0f * ALTO_VIRTUAL;
    return ImVec2(vx, vy);
}

inline void virtualALatLon(ImVec2 v, float& lat, float& lon) {
    lon = v.x / ANCHO_VIRTUAL * 360.0f - 180.0f;
    lat = 90.0f - v.y / ALTO_VIRTUAL * 180.0f;
}

// Declaraciones (implementaciones en DatosMundo.cpp)
float calcularDistancia(float lat1, float lon1, float lat2, float lon2);
const std::vector<Ciudad>& obtenerCiudades();
const std::vector<RutaAerea>& obtenerRutas();
void aplicarRestriccionesGeopoliticas(Grafo& g);
bool validarDatos();
std::vector<std::pair<int, int>> obtenerTop3Hubs();
float calcularDensidadRed();
Grafo construirGrafoAerografos();

} // namespace DatosMundo
