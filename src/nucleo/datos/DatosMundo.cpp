#include "DatosMundo.hpp"
#include <algorithm>
#include <cstdio>
#include <unordered_set>
#include <queue>
#include <cstdlib>

namespace DatosMundo {

// Haversine
float calcularDistancia(float lat1, float lon1, float lat2, float lon2) {
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

// Ciudades
const std::vector<Ciudad>& obtenerCiudades() {
    static const std::vector<Ciudad> ciudades = {
        // 0-4: Norteamerica
        {0,  "Nueva York",     "EE.UU.",        40.7128f,  -74.0060f,  "JFK", 8},
        {1,  "Los Ángeles",   "EE.UU.",        34.0522f, -118.2437f,  "LAX", 4},
        {2,  "Miami",         "EE.UU.",        25.7617f,  -80.1918f,  "MIA", 0},
        {3,  "Toronto",       "Canadá",        43.6532f,  -79.3832f,  "YYZ", 3},
        {4,  "Ciudad de México", "México",     19.4326f,  -99.1332f,  "MEX", 9},
        // 5-9: Sudamerica
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
        // 25-29: Africa y Oceania
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
        // 36-39: Resto Sudamerica
        {36, "Río de Janeiro","Brasil",       -22.9068f,  -43.1729f,  "GIG", 7},
        {37, "Quito",         "Ecuador",       -0.1807f,  -78.4678f,  "UIO", 2},
        {38, "Medellín",      "Colombia",       6.2476f,  -75.5658f,  "MDE", 3},
        {39, "Panamá",        "Panamá",         9.0000f,  -79.5000f,  "PTY", 2},
        // 40-42: Norteamerica extendida
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
        // 55-58: Africa extendida
        {55, "Lagos",         "Nigeria",        6.5244f,    3.3792f,  "LOS", 15},
        {56, "Nairobi",       "Kenia",         -1.2921f,   36.8219f,  "NBO", 4},
        {57, "Ciudad del Cabo","Sudáfrica",   -33.9249f,   18.4241f,  "CPT", 5},
        {58, "Marrakech",     "Marruecos",     31.6295f,   -7.9811f,  "RAK", 1},
        // 59: Oceania extendida
        {59, "Melbourne",     "Australia",    -37.8136f,  144.9631f,  "MEL", 5},
        // 60-62: Caribe
        {60, "La Habana",     "Cuba",          23.1136f,  -82.3666f,  "HAV", 2},
        {61, "San Juan",      "Puerto Rico",   18.4655f,  -66.1057f,  "SJU", 0},
        {62, "Santo Domingo", "Rep. Dominicana",18.4861f, -69.9312f,  "SDQ", 2},
    };
    return ciudades;
}

// Rutas Aereas
const std::vector<RutaAerea>& obtenerRutas() {
    static std::vector<RutaAerea> rutas;
    if (rutas.empty()) {
        rutas.reserve(300);

        struct Par { int o; int d; };
        static const Par pares[] = {
            {0,1},   {0,2},   {0,3},   {0,11},  {0,12},
            {0,40},  {0,41},  {0,44},  {1,2},   {1,4},
            {1,21},  {1,28},  {1,40},  {1,42},  {2,4},
            {2,5},   {2,6},   {2,11},  {2,30},  {2,31},
            {2,36},  {2,38},  {2,39},  {2,40},  {2,41},
            {2,43},  {2,60},  {2,61},  {2,62},  {3,11},
            {3,12},  {3,40},  {3,42},  {4,5},   {4,39},
            {4,60},  {40,41}, {40,11}, {40,42},
            {5,6},   {5,8},   {5,9},   {5,10},  {5,30},
            {5,31},  {5,37},  {5,38},  {5,39},  {6,7},
            {6,10},  {6,11},  {6,19},  {6,26},  {6,36},
            {7,9},   {7,10},  {7,36},  {7,57},  {8,4},
            {8,6},   {8,37},  {8,39},  {30,31}, {30,32},
            {30,33}, {30,34}, {30,35}, {30,39}, {30,10},
            {30,61}, {30,62}, {31,39}, {35,5},  {36,10},
            {37,39}, {38,39}, {39,10},
            {10,11}, {10,12}, {10,13}, {10,14}, {10,25},
            {10,27}, {10,43}, {10,55}, {10,58}, {10,60},
            {11,12}, {11,13}, {11,14}, {11,16}, {11,17},
            {11,43}, {11,44}, {11,46}, {11,48}, {11,55},
            {12,13}, {12,14}, {12,44}, {13,16}, {13,25},
            {13,45}, {14,15}, {14,16}, {14,44}, {14,46},
            {15,16}, {15,17}, {15,21}, {15,46}, {16,17},
            {16,25}, {16,45}, {16,47}, {16,48}, {43,58},
            {45,48},
            {17,18}, {17,19}, {17,20}, {17,25}, {17,47},
            {17,50}, {18,19}, {18,20}, {18,50},
            {19,20}, {19,21}, {19,22}, {19,24}, {19,49},
            {19,51}, {19,52}, {19,53}, {19,28}, {19,59},
            {20,21}, {20,53},
            {21,22}, {21,23}, {21,24}, {21,49}, {21,52}, {21,54},
            {22,23}, {22,24}, {22,49}, {22,54},
            {23,15}, {23,24}, {23,49},
            {24,49}, {24,54},
            {49,50}, {49,51}, {49,52}, {49,53}, {49,54}, {49,28},
            {51,53}, {52,54},
            {25,26}, {25,47}, {25,48}, {25,55}, {25,56},
            {26,56}, {26,57}, {26,28},
            {27,2},  {27,11}, {27,58},
            {55,56}, {56,17},
            {28,29}, {28,59}, {29,59}, {29,21},
            {60,61}, {60,62}, {61,62},
            {10,16}, {10,17}, {10,47},
            {16,21}, {17,21}, {47,21},
            {11,19}, {6,17},
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

// Restricciones Geopoliticas
void aplicarRestriccionesGeopoliticas(Grafo& g) {
    for (auto& a : g.aristas) {
        bool toca_rusia = (a.origen_id == 15 || a.destino_id == 15);
        bool sobrevuelo =
            (a.origen_id == 16 && a.destino_id == 21) ||
            (a.origen_id == 21 && a.destino_id == 16) ||
            (a.origen_id == 11 && a.destino_id == 21) ||
            (a.origen_id == 21 && a.destino_id == 11) ||
            (a.origen_id == 14 && a.destino_id == 21) ||
            (a.origen_id == 21 && a.destino_id == 14);
        if (toca_rusia || sobrevuelo) {
            a.peso *= 100000.0f;
            a.peso_actual *= 100000.0f;
        }
    }
}

// Validacion
bool validarDatos() {
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();

    if (ciudades.size() < 25) {
        printf("[ERROR] DatosMundo: solo %zu ciudades\n", ciudades.size());
        return false;
    }

    for (const auto& c : ciudades) {
        if (c.latitud < -90.0f || c.latitud > 90.0f) {
            printf("[ERROR] Ciudad %s: latitud invalida %.2f\n", c.nombre, c.latitud);
            return false;
        }
        if (c.longitud < -180.0f || c.longitud > 180.0f) {
            printf("[ERROR] Ciudad %s: longitud invalida %.2f\n", c.nombre, c.longitud);
            return false;
        }
        std::string iata(c.codigo_iata);
        if (iata.length() != 3) {
            printf("[ERROR] Ciudad %s: IATA '%s' no tiene 3 letras\n", c.nombre, c.codigo_iata);
            return false;
        }
    }

    std::unordered_set<int> ids;
    for (const auto& c : ciudades) {
        if (ids.count(c.id)) {
            printf("[ERROR] Ciudad duplicada: id %d\n", c.id);
            return false;
        }
        ids.insert(c.id);
    }

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

// Metricas Analiticas
std::vector<std::pair<int, int>> obtenerTop3Hubs() {
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

    std::sort(nodos_grados.begin(), nodos_grados.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return a.second > b.second;
              });

    if (nodos_grados.size() > 3) nodos_grados.resize(3);
    return nodos_grados;
}

float calcularDensidadRed() {
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();
    float V = (float)ciudades.size();
    float E = (float)rutas.size();
    if (V <= 1) return 0.0f;
    return (2.0f * E) / (V * (V - 1.0f));
}

// Construir Grafo desde datos AeroGrafos
Grafo construirGrafoAerografos() {
    Grafo g;
    const auto& ciudades = obtenerCiudades();
    const auto& rutas = obtenerRutas();

    for (const auto& c : ciudades) {
        ImVec2 pos = latLonAVirtual(c.latitud, c.longitud);
        g.nodos.push_back(Nodo(c.id, pos, TipoHardware::Servidor));
        g.nodos.back().nombre = c.nombre;
    }
    g.contador_ids = (int)ciudades.size();

    for (const auto& r : rutas) {
        float lon_origen = ciudades[r.origen_id].longitud;
        float lon_destino = ciudades[r.destino_id].longitud;
        float distancia_base = r.distancia_km;

        float peso_ida = (lon_destino > lon_origen) ? (distancia_base * 0.90f) : (distancia_base * 1.10f);
        float peso_vuelta = (lon_origen > lon_destino) ? (distancia_base * 0.90f) : (distancia_base * 1.10f);

        g.agregarArista(r.origen_id, r.destino_id, peso_ida, true);
        g.agregarArista(r.destino_id, r.origen_id, peso_vuelta, true);
    }

    return g;
}

} // namespace DatosMundo
