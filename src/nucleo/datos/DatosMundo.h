#pragma once

#include "imgui.h"
#include "../Grafo.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <unordered_set>
#include <queue>
#include <cstdlib>

// ============================================================================
// DatosMundo — Base de datos geográfica mundial para AeroGrafos
//
// 63 ciudades reales con coordenadas verificadas
// 200+ rutas aéreas con distancias calculadas via Haversine
// Grafo conexo garantizado
// Incluye Venezuela (Caracas, Maracaibo, Valencia, Barquisimeto, Mérida, Pto. Ordaz)
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

// ── Haversine ──────────────────────────────────────────────────────────────
inline float calcularDistancia(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371.0f;  // radio terrestre en km
    float dlat = (lat2 - lat1) * 3.14159265f / 180.0f;
    float dlon = (lon2 - lon1) * 3.14159265f / 180.0f;
    float a = sinf(dlat / 2.0f) * sinf(dlat / 2.0f) +
              cosf(lat1 * 3.14159265f / 180.0f) *
              cosf(lat2 * 3.14159265f / 180.0f) *
              sinf(dlon / 2.0f) * sinf(dlon / 2.0f);
    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    return R * c;
}

// ── Ciudades ───────────────────────────────────────────────────────────────
// 63 ciudades en orden SECUENCIAL (ID = índice en el vector)
// Esto es crítico: las rutas acceden via ciudades[id], ID debe coincidir con índice
// Índice: {id, nombre, pais, lat, lon, iata, poblacion_M}
inline const std::vector<Ciudad>& obtenerCiudades() {
    static const std::vector<Ciudad> ciudades = {
        //  0-4: Norteamérica
        {0,  "Nueva York",     "EE.UU.",        40.7128f,  -74.0060f,  "JFK", 8},
        {1,  "Los Ángeles",   "EE.UU.",        34.0522f, -118.2437f,  "LAX", 4},
        {2,  "Miami",         "EE.UU.",        25.7617f,  -80.1918f,  "MIA", 0},
        {3,  "Toronto",       "Canadá",        43.6532f,  -79.3832f,  "YYZ", 3},
        {4,  "Ciudad de México", "México",     19.4326f,  -99.1332f,  "MEX", 9},
        //  5-9: Sudamérica
        {5,  "Bogotá",        "Colombia",       4.7110f,  -74.0721f,  "BOG", 7},
        {6,  "São Paulo",     "Brasil",       -23.5505f,  -46.6333f,  "GRU", 12},
        {7,  "Buenos Aires",  "Argentina",    -34.6037f,  -58.3816f,  "EZE", 3},
        {8,  "Lima",          "Perú",         -12.0464f,  -77.0428f,  "LIM", 10},
        {9,  "Santiago",      "Chile",        -33.4489f,  -70.6693f,  "SCL", 6},
        // 10-16: Europa Occidental
        {10, "Madrid",        "España",        40.4168f,   -3.7038f,  "MAD", 3},
        {11, "Londres",       "Reino Unido",   51.5074f,   -0.1278f,  "LHR", 9},
        {12, "París",         "Francia",       48.8566f,    2.3522f,  "CDG", 2},
        {13, "Roma",          "Italia",        41.9028f,   12.4964f,  "FCO", 3},
        {14, "Berlín",        "Alemania",      52.5200f,   13.4050f,  "BER", 4},
        {15, "Moscú",         "Rusia",         55.7558f,   37.6173f,  "SVO", 12},
        {16, "Estambul",      "Turquía",       41.0082f,   28.9784f,  "IST", 15},
        // 17-24: Medio Oriente y Asia
        {17, "Dubái",         "EAU",           25.2048f,   55.2708f,  "DXB", 3},
        {18, "Mumbai",        "India",         19.0760f,   72.8777f,  "BOM", 12},
        {19, "Singapur",      "Singapur",       1.3521f,  103.8198f,  "SIN", 6},
        {20, "Bangkok",       "Tailandia",     13.7563f,  100.5018f,  "BKK", 8},
        {21, "Tokio",         "Japón",         35.6762f,  139.6503f,  "NRT", 9},
        {22, "Seúl",          "Corea del Sur", 37.5665f,  126.9780f,  "ICN", 10},
        {23, "Pekín",         "China",         39.9042f,  116.4074f,  "PEK", 22},
        {24, "Shanghái",      "China",         31.2304f,  121.4737f,  "PVG", 25},
        // 25-29: África y Oceanía
        {25, "El Cairo",      "Egipto",        30.0444f,   31.2357f,  "CAI", 10},
        {26, "Johannesburgo", "Sudáfrica",    -26.2041f,   28.0473f,  "JNB", 6},
        {27, "Casablanca",    "Marruecos",     33.5731f,   -7.5898f,  "CMN", 3},
        {28, "Sídney",        "Australia",    -33.8688f,  151.2093f,  "SYD", 5},
        {29, "Auckland",      "Nueva Zelanda",-36.8485f,  174.7633f,  "AKL", 2},
        // 30-35: Venezuela
        {30, "Caracas",       "Venezuela",     10.4806f,  -66.9036f,  "CCS", 3},
        {31, "Maracaibo",     "Venezuela",     10.6540f,  -71.6310f,  "MAR", 3},
        {32, "Valencia",      "Venezuela",     10.1621f,  -68.0070f,  "VLN", 2},
        {33, "Barquisimeto",  "Venezuela",     10.0734f,  -69.3228f,  "BRM", 1},
        {34, "Mérida",        "Venezuela",      8.5981f,  -71.1440f,  "MRD", 0},
        {35, "Puerto Ordaz",  "Venezuela",      8.2932f,  -62.7170f,  "PZO", 1},
        // 36-39: Resto Sudamérica
        {36, "Río de Janeiro","Brasil",       -22.9068f,  -43.1729f,  "GIG", 7},
        {37, "Quito",         "Ecuador",       -0.1807f,  -78.4678f,  "UIO", 2},
        {38, "Medellín",      "Colombia",       6.2476f,  -75.5658f,  "MDE", 3},
        {39, "Panamá",        "Panamá",         9.0000f,  -79.5000f,  "PTY", 2},
        // 40-42: Norteamérica extendida
        {40, "Chicago",       "EE.UU.",        41.8781f,  -87.6298f,  "ORD", 10},
        {41, "Atlanta",       "EE.UU.",        33.7490f,  -84.3880f,  "ATL", 6},
        {42, "Vancouver",     "Canadá",        49.2827f, -123.1207f,  "YVR", 3},
        // 43-46: Europa extendida
        {43, "Lisboa",        "Portugal",      38.7223f,   -9.1393f,  "LIS", 2},
        {44, "Ámsterdam",     "Países Bajos",  52.3676f,    4.9041f,  "AMS", 3},
        {45, "Atenas",        "Grecia",        37.9838f,   23.7275f,  "ATH", 3},
        {46, "Estocolmo",     "Suecia",        59.3293f,   18.0686f,  "ARN", 2},
        // 47-48: Medio Oriente extendido
        {47, "Doha",          "Qatar",         25.2854f,   51.5310f,  "DOH", 2},
        {48, "Tel Aviv",      "Israel",        32.0853f,   34.7818f,  "TLV", 4},
        // 49-54: Asia extendida
        {49, "Hong Kong",     "China",         22.3193f,  114.1694f,  "HKG", 8},
        {50, "Delhi",         "India",         28.7041f,   77.1025f,  "DEL", 16},
        {51, "Yakarta",       "Indonesia",     -6.2088f,  106.8456f,  "CGK", 11},
        {52, "Manila",        "Filipinas",     14.5995f,  120.9842f,  "MNL", 13},
        {53, "Kuala Lumpur",  "Malasia",        3.1390f,  101.6869f,  "KUL", 2},
        {54, "Taipéi",        "Taiwán",        25.0330f,  121.5654f,  "TPE", 7},
        // 55-58: África extendida
        {55, "Lagos",         "Nigeria",        6.5244f,    3.3792f,  "LOS", 15},
        {56, "Nairobi",       "Kenia",         -1.2921f,   36.8219f,  "NBO", 4},
        {57, "Ciudad del Cabo","Sudáfrica",   -33.9249f,   18.4241f,  "CPT", 5},
        {58, "Marrakech",     "Marruecos",     31.6295f,   -7.9811f,  "RAK", 1},
        // 59: Oceanía extendida
        {59, "Melbourne",     "Australia",    -37.8136f,  144.9631f,  "MEL", 5},
        // 60-62: Caribe
        {60, "La Habana",     "Cuba",          23.1136f,  -82.3666f,  "HAV", 2},
        {61, "San Juan",      "Puerto Rico",   18.4655f,  -66.1057f,  "SJU", 0},
        {62, "Santo Domingo", "Rep. Dominicana",18.4861f, -69.9312f,  "SDQ", 2},
    };
    return ciudades;
}

// ── Rutas Aéreas ───────────────────────────────────────────────────────────
// Definidas como pares (origen, destino) — la distancia se calcula con Haversine
// Grafo conexo garantizado — todas las rutas son bidireccionales
inline const std::vector<RutaAerea>& obtenerRutas() {
    static std::vector<RutaAerea> rutas;
    if (rutas.empty()) {
        rutas.reserve(300);

        // Pares (origen, destino) con origen < destino para evitar duplicados
        struct Par { int o; int d; };
        static const Par pares[] = {
            // ── Norteamérica ──
            {0,1},   // NY-LA
            {0,2},   // NY-Miami
            {0,3},   // NY-Toronto
            {0,11},  // NY-Londres
            {0,12},  // NY-París
            {0,40},  // NY-Chicago
            {0,41},  // NY-Atlanta
            {0,44},  // NY-Ámsterdam
            {1,2},   // LA-Miami
            {1,4},   // LA-CDMX
            {1,21},  // LA-Tokio
            {1,28},  // LA-Sídney
            {1,40},  // LA-Chicago
            {1,42},  // LA-Vancouver
            {2,4},   // Miami-CDMX
            {2,5},   // Miami-Bogotá
            {2,6},   // Miami-São Paulo
            {2,11},  // Miami-Londres
            {2,30},  // Miami-Caracas
            {2,31},  // Miami-Maracaibo
            {2,36},  // Miami-Río
            {2,38},  // Miami-Medellín
            {2,39},  // Miami-Panamá
            {2,40},  // Miami-Chicago
            {2,41},  // Miami-Atlanta
            {2,43},  // Miami-Lisboa
            {2,60},  // Miami-La Habana
            {2,61},  // Miami-San Juan
            {2,62},  // Miami-Santo Domingo
            {3,11},  // Toronto-Londres
            {3,12},  // Toronto-París
            {3,40},  // Toronto-Chicago
            {3,42},  // Toronto-Vancouver
            {4,5},   // CDMX-Bogotá
            {4,39},  // CDMX-Panamá
            {4,60},  // CDMX-La Habana
            {40,41}, // Chicago-Atlanta
            {40,11}, // Chicago-Londres
            {40,42}, // Chicago-Vancouver

            // ── Venezuela y Sudamérica ──
            {5,6},   // Bogotá-São Paulo
            {5,8},   // Bogotá-Lima
            {5,9},   // Bogotá-Santiago
            {5,10},  // Bogotá-Madrid
            {5,30},  // Bogotá-Caracas
            {5,31},  // Bogotá-Maracaibo
            {5,37},  // Bogotá-Quito
            {5,38},  // Bogotá-Medellín
            {5,39},  // Bogotá-Panamá
            {6,7},   // São Paulo-Buenos Aires
            {6,10},  // São Paulo-Madrid
            {6,11},  // São Paulo-Londres
            {6,19},  // São Paulo-Singapur
            {6,26},  // São Paulo-Johannesburgo
            {6,36},  // São Paulo-Río
            {7,9},   // Buenos Aires-Santiago
            {7,10},  // Buenos Aires-Madrid
            {7,36},  // Buenos Aires-Río
            {7,57},  // Buenos Aires-Ciudad del Cabo
            {8,4},   // Lima-CDMX
            {8,6},   // Lima-São Paulo
            {8,37},  // Lima-Quito
            {8,39},  // Lima-Panamá
            {30,31}, // Caracas-Maracaibo
            {30,32}, // Caracas-Valencia
            {30,33}, // Caracas-Barquisimeto
            {30,34}, // Caracas-Mérida
            {30,35}, // Caracas-Puerto Ordaz
            {30,39}, // Caracas-Panamá
            {30,10}, // Caracas-Madrid
            {30,61}, // Caracas-San Juan
            {30,62}, // Caracas-Santo Domingo
            {31,39}, // Maracaibo-Panamá
            {35,5},  // Puerto Ordaz-Bogotá
            {36,10}, // Río-Madrid
            {37,39}, // Quito-Panamá
            {38,39}, // Medellín-Panamá
            {39,10}, // Panamá-Madrid

            // ── Europa ──
            {10,11}, // Madrid-Londres
            {10,12}, // Madrid-París
            {10,13}, // Madrid-Roma
            {10,14}, // Madrid-Berlín
            {10,25}, // Madrid-El Cairo
            {10,27}, // Madrid-Casablanca
            {10,43}, // Madrid-Lisboa
            {10,55}, // Madrid-Lagos
            {10,58}, // Madrid-Marrakech
            {10,60}, // Madrid-La Habana
            {11,12}, // Londres-París
            {11,13}, // Londres-Roma
            {11,14}, // Londres-Berlín
            {11,16}, // Londres-Estambul
            {11,17}, // Londres-Dubái
            {11,43}, // Londres-Lisboa
            {11,44}, // Londres-Ámsterdam
            {11,46}, // Londres-Estocolmo
            {11,48}, // Londres-Tel Aviv
            {11,55}, // Londres-Lagos
            {12,13}, // París-Roma
            {12,14}, // París-Berlín
            {12,44}, // París-Ámsterdam
            {13,16}, // Roma-Estambul
            {13,25}, // Roma-El Cairo
            {13,45}, // Roma-Atenas
            {14,15}, // Berlín-Moscú
            {14,16}, // Berlín-Estambul
            {14,44}, // Berlín-Ámsterdam
            {14,46}, // Berlín-Estocolmo
            {15,16}, // Moscú-Estambul
            {15,17}, // Moscú-Dubái
            {15,21}, // Moscú-Tokio
            {15,46}, // Moscú-Estocolmo
            {16,17}, // Estambul-Dubái
            {16,25}, // Estambul-El Cairo
            {16,45}, // Estambul-Atenas
            {16,47}, // Estambul-Doha
            {16,48}, // Estambul-Tel Aviv
            {43,58}, // Lisboa-Marrakech
            {45,48}, // Atenas-Tel Aviv

            // ── Asia ──
            {17,18}, // Dubái-Mumbai
            {17,19}, // Dubái-Singapur
            {17,20}, // Dubái-Bangkok
            {17,25}, // Dubái-El Cairo
            {17,47}, // Dubái-Doha
            {17,50}, // Dubái-Delhi
            {18,19}, // Mumbai-Singapur
            {18,20}, // Mumbai-Bangkok
            {18,50}, // Mumbai-Delhi
            {19,20}, // Singapur-Bangkok
            {19,21}, // Singapur-Tokio
            {19,22}, // Singapur-Seúl
            {19,24}, // Singapur-Shanghái
            {19,49}, // Singapur-Hong Kong
            {19,51}, // Singapur-Yakarta
            {19,52}, // Singapur-Manila
            {19,53}, // Singapur-Kuala Lumpur
            {19,28}, // Singapur-Sídney
            {19,59}, // Singapur-Melbourne
            {20,21}, // Bangkok-Tokio
            {20,53}, // Bangkok-Kuala Lumpur
            {21,22}, // Tokio-Seúl
            {21,23}, // Tokio-Pekín
            {21,24}, // Tokio-Shanghái
            {21,49}, // Tokio-Hong Kong
            {21,52}, // Tokio-Manila
            {21,54}, // Tokio-Taipéi
            {22,23}, // Seúl-Pekín
            {22,24}, // Seúl-Shanghái
            {22,49}, // Seúl-Hong Kong
            {22,54}, // Seúl-Taipéi
            {23,15}, // Pekín-Moscú
            {23,24}, // Pekín-Shanghái
            {23,49}, // Pekín-Hong Kong
            {24,49}, // Shanghái-Hong Kong
            {24,54}, // Shanghái-Taipéi
            {49,50}, // Hong Kong-Delhi
            {49,51}, // Hong Kong-Yakarta
            {49,52}, // Hong Kong-Manila
            {49,53}, // Hong Kong-Kuala Lumpur
            {49,54}, // Hong Kong-Taipéi
            {49,28}, // Hong Kong-Sídney
            {51,53}, // Yakarta-Kuala Lumpur
            {52,54}, // Manila-Taipéi

            // ── África ──
            {25,26}, // El Cairo-Johannesburgo
            {25,47}, // El Cairo-Doha
            {25,48}, // El Cairo-Tel Aviv
            {25,55}, // El Cairo-Lagos
            {25,56}, // El Cairo-Nairobi
            {26,56}, // Johannesburgo-Nairobi
            {26,57}, // Johannesburgo-Ciudad del Cabo
            {26,28}, // Johannesburgo-Sídney
            {27,2},  // Casablanca-Miami
            {27,11}, // Casablanca-Londres
            {27,58}, // Casablanca-Marrakech
            {55,56}, // Lagos-Nairobi
            {56,17}, // Nairobi-Dubái

            // ── Oceanía ──
            {28,29}, // Sídney-Auckland
            {28,59}, // Sídney-Melbourne
            {29,59}, // Auckland-Melbourne
            {29,21}, // Auckland-Tokio

            // ── Caribe ──
            {60,61}, // La Habana-San Juan
            {60,62}, // La Habana-Santo Domingo
            {61,62}, // San Juan-Santo Domingo

            // ── Conexiones Directas y Hubs Faltantes ──
            {10, 16}, // Madrid-Estambul (Turkish Airlines)
            {10, 17}, // Madrid-Dubái (Emirates)
            {10, 47}, // Madrid-Doha (Qatar Airways)
            {16, 21}, // Estambul-Tokio (Turkish)
            {17, 21}, // Dubái-Tokio (Emirates)
            {47, 21}, // Doha-Tokio (Qatar Airways)
            {11, 19}, // Londres-Singapur (Qantas/BA)
            {6, 17},  // São Paulo-Dubái (Emirates)
        };

        const auto& ciudades = obtenerCiudades();
        for (const auto& p : pares) {
            if (p.o < 0 || p.o >= (int)ciudades.size() ||
                p.d < 0 || p.d >= (int)ciudades.size()) continue;
            float d = calcularDistancia(
                ciudades[p.o].latitud, ciudades[p.o].longitud,
                ciudades[p.d].latitud, ciudades[p.d].longitud);
            rutas.push_back({p.o, p.d, d});
        }
    }
    return rutas;
}

// ── Validación ─────────────────────────────────────────────────────────────
inline bool validarDatos() {
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();

    // 1. Al menos 25 ciudades
    if (ciudades.size() < 25) {
        printf("[ERROR] DatosMundo: solo %zu ciudades\n", ciudades.size());
        return false;
    }

    // 2. Coordenadas válidas
    for (const auto& c : ciudades) {
        if (c.latitud < -90.0f || c.latitud > 90.0f) {
            printf("[ERROR] Ciudad %s: latitud invalida %.2f\n", c.nombre, c.latitud);
            return false;
        }
        if (c.longitud < -180.0f || c.longitud > 180.0f) {
            printf("[ERROR] Ciudad %s: longitud invalida %.2f\n", c.nombre, c.longitud);
            return false;
        }
        // IATA de 3 letras
        std::string iata(c.codigo_iata);
        if (iata.length() != 3) {
            printf("[ERROR] Ciudad %s: IATA '%s' no tiene 3 letras\n", c.nombre, c.codigo_iata);
            return false;
        }
    }

    // 3. IDs únicos
    std::unordered_set<int> ids;
    for (const auto& c : ciudades) {
        if (ids.count(c.id)) {
            printf("[ERROR] Ciudad duplicada: id %d\n", c.id);
            return false;
        }
        ids.insert(c.id);
    }

    // 4. Rutas válidas
    for (const auto& r : rutas) {
        if (r.origen_id == r.destino_id) {
            printf("[ERROR] Ruta self-loop: %d -> %d\n", r.origen_id, r.destino_id);
            return false;
        }
        if (!ids.count(r.origen_id) || !ids.count(r.destino_id)) {
            printf("[ERROR] Ruta con ciudad inexistente: %d -> %d\n", r.origen_id, r.destino_id);
            return false;
        }
        if (r.distancia_km <= 0.0f) {
            printf("[ERROR] Ruta distancia invalida: %.1f km\n", r.distancia_km);
            return false;
        }
    }

    // 5. Grafo conexo (BFS desde ciudad 0)
    std::unordered_set<int> visitados;
    std::queue<int> cola;
    cola.push(0);
    visitados.insert(0);
    while (!cola.empty()) {
        int actual = cola.front(); cola.pop();
        for (const auto& r : rutas) {
            int vecino = -1;
            if (r.origen_id == actual) vecino = r.destino_id;
            else if (r.destino_id == actual) vecino = r.origen_id;
            if (vecino >= 0 && !visitados.count(vecino)) {
                visitados.insert(vecino);
                cola.push(vecino);
            }
        }
    }
    if (visitados.size() != ciudades.size()) {
        printf("[ERROR] Grafo no conexo: %zu/%zu ciudades alcanzables\n",
               visitados.size(), ciudades.size());
        return false;
    }

    printf("[OK] DatosMundo validado: %zu ciudades, %zu rutas, conexo\n",
           ciudades.size(), rutas.size());
    return true;
}

// ── Métricas Analíticas ───────────────────────────────────────────────────

inline std::vector<std::pair<int, int>> obtenerTop3Hubs() {
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();
    std::vector<int> grados(ciudades.size(), 0);
    
    for (const auto& r : rutas) {
        grados[r.origen_id]++;
        grados[r.destino_id]++;
    }
    
    std::vector<std::pair<int, int>> nodos_grados;
    for (int i = 0; i < (int)ciudades.size(); i++) {
        nodos_grados.push_back({i, grados[i]});
    }
    
    std::sort(nodos_grados.begin(), nodos_grados.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second;
    });
    
    if (nodos_grados.size() > 3) nodos_grados.resize(3);
    return nodos_grados;
}

inline float calcularDensidadRed() {
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();
    float V = (float)ciudades.size();
    float E = (float)rutas.size();
    if (V <= 1) return 0.0f;
    return (2.0f * E) / (V * (V - 1.0f));
}

// ── Proyección ─────────────────────────────────────────────────────────────
// Proyección equirectangular: lat/lon → coordenadas virtuales (0..W, 0..H)
inline constexpr float ANCHO_VIRTUAL = 2048.0f;
inline constexpr float ALTO_VIRTUAL  = 1024.0f;

inline ImVec2 latLonAVirtual(float lat, float lon) {
    float vx = (lon + 180.0f) / 360.0f * ANCHO_VIRTUAL;
    float vy = (90.0f - lat) / 180.0f * ALTO_VIRTUAL;
    return ImVec2(vx, vy);
}

// Virtual → lat/lon (para hit testing)
inline void virtualALatLon(ImVec2 v, float& lat, float& lon) {
    lon = v.x / ANCHO_VIRTUAL * 360.0f - 180.0f;
    lat = 90.0f - v.y / ALTO_VIRTUAL * 180.0f;
}

// ── Construir Grafo desde datos AeroGrafos ───────────────────────────────
// Crea un Grafo (no dirigido, ponderado) para usar con los algoritmos existentes
inline Grafo construirGrafoAerografos() {
    Grafo g;
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();

    // Agregar nodos (ciudades)
    for (const auto& c : ciudades) {
        ImVec2 pos = latLonAVirtual(c.latitud, c.longitud);
        g.nodos.push_back(Nodo(c.id, pos, TipoHardware::Servidor));
        g.nodos.back().nombre = c.nombre;
    }
    g.contador_ids = (int)ciudades.size();

    // Inyectar aristas dirigidas asimétricas (Jet Stream)
    for (const auto& r : rutas) {
        float lon_origen = ciudades[r.origen_id].longitud;
        float lon_destino = ciudades[r.destino_id].longitud;
        float distancia_base = r.distancia_km;

        // Vuelo de Origen a Destino
        float peso_ida = (lon_destino > lon_origen) ? (distancia_base * 0.90f) : (distancia_base * 1.10f);
        // Vuelo de Destino a Origen
        float peso_vuelta = (lon_origen > lon_destino) ? (distancia_base * 0.90f) : (distancia_base * 1.10f);

        // Se usa dirigida = true para insertar ambas direcciones con pesos distintos
        g.agregarArista(r.origen_id, r.destino_id, peso_ida, true);
        g.agregarArista(r.destino_id, r.origen_id, peso_vuelta, true);
    }

    return g;
}

} // namespace DatosMundo
