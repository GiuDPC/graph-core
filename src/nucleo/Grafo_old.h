#pragma once

#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_set>
#include <random>
#include <set>
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

// Tipos de hardware
enum class TipoHardware {
    Servidor, Router, Switch, Firewall, Terminal
};

inline float latenciaHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return 0.0f;
        case TipoHardware::Router:   return 5.0f;
        case TipoHardware::Switch:   return 1.0f;
        case TipoHardware::Firewall: return 15.0f;
        case TipoHardware::Terminal: return 0.0f;
        default: return 0.0f;
    }
}

inline const char* prefijoHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return "SRV";
        case TipoHardware::Router:   return "RTR";
        case TipoHardware::Switch:   return "SW";
        case TipoHardware::Firewall: return "FW";
        case TipoHardware::Terminal: return "PC";
        default: return "N";
    }
}

inline const char* nombreHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return "Servidor";
        case TipoHardware::Router:   return "Router";
        case TipoHardware::Switch:   return "Switch";
        case TipoHardware::Firewall: return "Firewall";
        case TipoHardware::Terminal: return "Terminal";
        default: return "Desconocido";
    }
}

// Animacion paso a paso
struct PasoAnimacion {
    enum Accion { VISITAR, EXPLORAR, CONFIRMAR, DESCARTAR };
    Accion accion;
    int nodo_id = -1;
    int arista_origen = -1;
    int arista_destino = -1;
    std::string descripcion;
};

// Estructuras de datos
struct Nodo {
    int id;
    std::string nombre;
    ImVec2 posicion;
    float radio;
    TipoHardware tipo;

    Nodo(int _id, ImVec2 _pos, TipoHardware _tipo = TipoHardware::Servidor) {
        id = _id;
        posicion = _pos;
        radio = 24.0f;
        tipo = _tipo;
        nombre = std::string(prefijoHardware(_tipo)) + std::to_string(_id);
    }
};

struct Arista {
    int origen_id;
    int destino_id;
    float peso;
    float peso_actual;
    bool es_dirigida;

    Arista(int _origen, int _destino, float _peso = 1.0f, bool _dirigida = false)
        : origen_id(_origen), destino_id(_destino), peso(_peso),
          peso_actual(_peso), es_dirigida(_dirigida) {}
};

// Union-find (disjoint set)
class UnionFind {
private:
    std::vector<int> padre;
    std::vector<int> rango;

public:
    UnionFind(int n) {
        padre.resize(n);
        rango.resize(n, 0);
        for (int i = 0; i < n; i++) padre[i] = i;
    }

    int encontrar(int i) {
        if (padre[i] == i) return i;
        return padre[i] = encontrar(padre[i]);
    }

    bool unir(int x, int y) {
        int raiz_x = encontrar(x);
        int raiz_y = encontrar(y);
        if (raiz_x == raiz_y) return false;
        if (rango[raiz_x] < rango[raiz_y]) padre[raiz_x] = raiz_y;
        else if (rango[raiz_x] > rango[raiz_y]) padre[raiz_y] = raiz_x;
        else { padre[raiz_y] = raiz_x; rango[raiz_x]++; }
        return true;
    }
};

// Clase grafo
class Grafo {
public:
    std::vector<Nodo> nodos;
    std::vector<Arista> aristas;
    int contador_ids = 0;

    // --- Helpers ---
    std::string nombreNodo(int id) {
        Nodo* n = obtenerNodo(id);
        return n ? n->nombre : "?";
    }

    // --- CRUD ---
    void agregarNodo(ImVec2 posicion, TipoHardware tipo = TipoHardware::Servidor) {
        nodos.push_back(Nodo(contador_ids, posicion, tipo));
        contador_ids++;
    }

    void agregarArista(int id1, int id2, float peso = 1.0f) {
        if (id1 == id2) return;
        for (const auto& a : aristas) {
            if ((a.origen_id == id1 && a.destino_id == id2) ||
                (a.origen_id == id2 && a.destino_id == id1)) return;
        }
        aristas.push_back(Arista(id1, id2, peso));
    }

    void eliminarNodo(int id) {
        aristas.erase(std::remove_if(aristas.begin(), aristas.end(),
            [id](const Arista& a) { return a.origen_id == id || a.destino_id == id; }),
            aristas.end());
        nodos.erase(std::remove_if(nodos.begin(), nodos.end(),
            [id](const Nodo& n) { return n.id == id; }), nodos.end());
        if (nodos.empty()) {
            contador_ids = 0;
        }
    }

    Nodo* obtenerNodo(int id) {
        for (auto& n : nodos) { if (n.id == id) return &n; }
        return nullptr;
    }

    void limpiar() {
        nodos.clear();
        aristas.clear();
        contador_ids = 0;
    }

    // --- DIJKSTRA ---
    std::vector<int> rutaMasCortaDijkstra(int id_origen, int id_destino, bool aplicar_latencia = false) {
        std::vector<int> camino;
        if (id_origen == id_destino) return camino;

        const float INF = std::numeric_limits<float>::infinity();
        std::vector<float> dist(contador_ids, INF);
        std::vector<int> prev(contador_ids, -1);
        dist[id_origen] = 0.0f;

        using Par = std::pair<float, int>;
        std::priority_queue<Par, std::vector<Par>, std::greater<Par>> cola;
        cola.push({0.0f, id_origen});

        while (!cola.empty()) {
            auto [d, u] = cola.top(); cola.pop();
            if (u == id_destino) break;
            if (d > dist[u]) continue;

            for (const auto& a : aristas) {
                int v = -1;
                if (a.origen_id == u) v = a.destino_id;
                else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;

                if (v != -1) {
                    float costo = a.peso_actual;
                    if (aplicar_latencia) {
                        Nodo* nv = obtenerNodo(v);
                        if (nv) costo += latenciaHardware(nv->tipo);
                    }
                    float nd = d + costo;
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        prev[v] = u;
                        cola.push({nd, v});
                    }
                }
            }
        }

        if (dist[id_destino] == INF) return camino;
        for (int at = id_destino; at != -1; at = prev[at]) camino.push_back(at);
        std::reverse(camino.begin(), camino.end());
        return camino;
    }

    // --- KRUSKAL ---
    std::vector<Arista> kruskalMST() {
        std::vector<Arista> resultado;
        if (nodos.empty()) return resultado;
        std::vector<Arista> ordenadas = aristas;
        std::sort(ordenadas.begin(), ordenadas.end(),
            [](const Arista& a, const Arista& b) { return a.peso < b.peso; });
        UnionFind uf(contador_ids);
        for (const auto& a : ordenadas) {
            if (uf.unir(a.origen_id, a.destino_id)) {
                resultado.push_back(a);
                if (resultado.size() == nodos.size() - 1) break;
            }
        }
        return resultado;
    }

    // --- BFS ---
    std::vector<int> recorridoBFS(int inicio_id) {
        std::vector<int> orden;
        if (nodos.empty() || !obtenerNodo(inicio_id)) return orden;
        std::vector<bool> visitado(contador_ids, false);
        std::queue<int> cola;
        visitado[inicio_id] = true;
        cola.push(inicio_id);
        while (!cola.empty()) {
            int u = cola.front(); cola.pop();
            orden.push_back(u);
            for (const auto& a : aristas) {
                int v = -1;
                if (a.origen_id == u) v = a.destino_id;
                else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
                if (v != -1 && !visitado[v]) { visitado[v] = true; cola.push(v); }
            }
        }
        return orden;
    }

    // --- DFS ---
    void dfsRecursivo(int u, std::vector<bool>& vis, std::vector<int>& orden) {
        vis[u] = true;
        orden.push_back(u);
        for (const auto& a : aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v != -1 && !vis[v]) dfsRecursivo(v, vis, orden);
        }
    }

    std::vector<int> recorridoDFS(int inicio_id) {
        std::vector<int> orden;
        if (nodos.empty() || !obtenerNodo(inicio_id)) return orden;
        std::vector<bool> vis(contador_ids, false);
        dfsRecursivo(inicio_id, vis, orden);
        return orden;
    }

    // --- CICLOS ---
    bool tieneCiclos() {
        UnionFind uf(contador_ids);
        for (const auto& a : aristas) {
            if (!uf.unir(a.origen_id, a.destino_id)) return true;
        }
        return false;
    }

    // --- COLOREO GREEDY ---
    std::vector<int> colorearGrafo() {
        std::vector<int> color(contador_ids, -1);
        if (nodos.empty()) return color;
        color[nodos[0].id] = 0;
        for (size_t i = 1; i < nodos.size(); i++) {
            int u = nodos[i].id;
            std::vector<bool> usado(contador_ids, false);
            for (const auto& a : aristas) {
                int v = -1;
                if (a.origen_id == u) v = a.destino_id;
                else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
                if (v != -1 && color[v] != -1) usado[color[v]] = true;
            }
            int c = 0;
            while (usado[c]) c++;
            color[u] = c;
        }
        return color;
    }

        // GENERADORES DE PASOS (ANIMACION)
        std::vector<PasoAnimacion> generarPasosBFS(int inicio_id) {
        std::vector<PasoAnimacion> pasos;
        if (nodos.empty() || !obtenerNodo(inicio_id)) return pasos;

        std::vector<bool> visitado(contador_ids, false);
        std::queue<int> cola;
        visitado[inicio_id] = true;
        cola.push(inicio_id);
        pasos.push_back({PasoAnimacion::VISITAR, inicio_id, -1, -1,
            "[BFS] Inicio en " + nombreNodo(inicio_id)});

        while (!cola.empty()) {
            int u = cola.front(); cola.pop();
            pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
                "[BFS] Procesando " + nombreNodo(u)});

            for (const auto& a : aristas) {
                int v = -1;
                if (a.origen_id == u) v = a.destino_id;
                else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
                if (v == -1) continue;

                pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
                    "[BFS] Explorando " + nombreNodo(u) + " -> " + nombreNodo(v)});

                if (!visitado[v]) {
                    visitado[v] = true;
                    cola.push(v);
                    pasos.push_back({PasoAnimacion::VISITAR, v, -1, -1,
                        "[BFS] Descubierto " + nombreNodo(v)});
                }
            }
        }
        return pasos;
    }

    std::vector<PasoAnimacion> generarPasosDFS(int inicio_id) {
        std::vector<PasoAnimacion> pasos;
        if (nodos.empty() || !obtenerNodo(inicio_id)) return pasos;
        std::vector<bool> visitado(contador_ids, false);
        dfsPasos(inicio_id, visitado, pasos);
        return pasos;
    }

    std::vector<PasoAnimacion> generarPasosDijkstra(int id_origen, int id_destino, bool lat = false) {
        std::vector<PasoAnimacion> pasos;
        if (id_origen == id_destino) return pasos;

        const float INF = std::numeric_limits<float>::infinity();
        std::vector<float> dist(contador_ids, INF);
        std::vector<int> prev(contador_ids, -1);
        dist[id_origen] = 0.0f;

        using Par = std::pair<float, int>;
        std::priority_queue<Par, std::vector<Par>, std::greater<Par>> cola;
        cola.push({0.0f, id_origen});

        pasos.push_back({PasoAnimacion::VISITAR, id_origen, -1, -1,
            "[Dijkstra] Inicio en " + nombreNodo(id_origen) + " (dist=0)"});

        while (!cola.empty()) {
            auto [d, u] = cola.top(); cola.pop();
            if (d > dist[u]) continue;

            pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
                "[Dijkstra] Confirmado " + nombreNodo(u) + " (dist=" +
                std::to_string((int)dist[u]) + ")"});

            if (u == id_destino) break;

            for (const auto& a : aristas) {
                int v = -1;
                if (a.origen_id == u) v = a.destino_id;
                else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
                if (v == -1) continue;

                float costo = a.peso_actual;
                if (lat) {
                    Nodo* nv = obtenerNodo(v);
                    if (nv) costo += latenciaHardware(nv->tipo);
                }

                pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
                    "[Dijkstra] Evaluando " + nombreNodo(u) + " -> " + nombreNodo(v) +
                    " (costo=" + std::to_string((int)costo) + ")"});

                float nd = d + costo;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    prev[v] = u;
                    cola.push({nd, v});
                    pasos.push_back({PasoAnimacion::VISITAR, v, -1, -1,
                        "[Dijkstra] Mejorado " + nombreNodo(v) + " (dist=" +
                        std::to_string((int)nd) + ")"});
                }
            }
        }

        // Highlight final path
        if (dist[id_destino] < INF) {
            std::vector<int> ruta;
            for (int at = id_destino; at != -1; at = prev[at]) ruta.push_back(at);
            std::reverse(ruta.begin(), ruta.end());
            for (size_t i = 0; i + 1 < ruta.size(); i++) {
                pasos.push_back({PasoAnimacion::CONFIRMAR, -1, ruta[i], ruta[i + 1],
                    "[Dijkstra] Ruta optima: " + nombreNodo(ruta[i]) + " -> " + nombreNodo(ruta[i + 1])});
            }
        }
        return pasos;
    }

    std::vector<PasoAnimacion> generarPasosKruskal() {
        std::vector<PasoAnimacion> pasos;
        if (nodos.empty()) return pasos;

        std::vector<Arista> ord = aristas;
        std::sort(ord.begin(), ord.end(),
            [](const Arista& a, const Arista& b) { return a.peso < b.peso; });

        UnionFind uf(contador_ids);
        int aceptadas = 0;

        for (const auto& a : ord) {
            pasos.push_back({PasoAnimacion::EXPLORAR, -1, a.origen_id, a.destino_id,
                "[Kruskal] Evaluando " + nombreNodo(a.origen_id) + " - " +
                nombreNodo(a.destino_id) + " (peso=" + std::to_string((int)a.peso) + ")"});

            if (uf.unir(a.origen_id, a.destino_id)) {
                pasos.push_back({PasoAnimacion::CONFIRMAR, -1, a.origen_id, a.destino_id,
                    "[Kruskal] Aceptada: conecta componentes distintos"});
                pasos.push_back({PasoAnimacion::CONFIRMAR, a.origen_id, -1, -1, ""});
                pasos.push_back({PasoAnimacion::CONFIRMAR, a.destino_id, -1, -1, ""});
                aceptadas++;
                if (aceptadas == (int)nodos.size() - 1) break;
            } else {
                pasos.push_back({PasoAnimacion::DESCARTAR, -1, a.origen_id, a.destino_id,
                    "[Kruskal] Rechazada: crearia ciclo"});
            }
        }
        return pasos;
    }

        // JITTER / SIMULACION
        void aplicarJitter(float porcentaje) {
        static std::mt19937 gen(42);
        for (auto& a : aristas) {
            std::uniform_real_distribution<float> dist(-porcentaje, porcentaje);
            a.peso_actual = a.peso * (1.0f + dist(gen));
            if (a.peso_actual < 0.1f) a.peso_actual = 0.1f;
        }
    }

    void resetearPesos() {
        for (auto& a : aristas) a.peso_actual = a.peso;
    }

        // PERSISTENCIA JSON
        void guardarJSON(const std::string& ruta) {
        json j;
        j["version"] = "2.0";
        j["contador_ids"] = contador_ids;

        j["nodos"] = json::array();
        for (const auto& n : nodos) {
            j["nodos"].push_back({
                {"id", n.id}, {"nombre", n.nombre},
                {"x", n.posicion.x}, {"y", n.posicion.y},
                {"tipo", (int)n.tipo}
            });
        }

        j["aristas"] = json::array();
        for (const auto& a : aristas) {
            j["aristas"].push_back({
                {"origen", a.origen_id}, {"destino", a.destino_id},
                {"peso", a.peso}
            });
        }

        std::ofstream archivo(ruta);
        if (archivo.is_open()) {
            archivo << j.dump(4);
            archivo.close();
        }
    }

    void cargarJSON(const std::string& ruta) {
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return;

        json j;
        archivo >> j;
        archivo.close();

        nodos.clear();
        aristas.clear();
        contador_ids = j.value("contador_ids", 0);

        for (const auto& nj : j["nodos"]) {
            TipoHardware tipo = (TipoHardware)nj.value("tipo", 0);
            Nodo nuevo(nj["id"], ImVec2(nj["x"], nj["y"]), tipo);
            nuevo.nombre = nj["nombre"];
            nodos.push_back(nuevo);
        }

        for (const auto& aj : j["aristas"]) {
            aristas.push_back(Arista(aj["origen"], aj["destino"], aj["peso"]));
        }
    }

private:
    void dfsPasos(int u, std::vector<bool>& vis, std::vector<PasoAnimacion>& pasos) {
        vis[u] = true;
        pasos.push_back({PasoAnimacion::VISITAR, u, -1, -1,
            "[DFS] Visitando " + nombreNodo(u)});

        for (const auto& a : aristas) {
            int v = -1;
            if (a.origen_id == u) v = a.destino_id;
            else if (!a.es_dirigida && a.destino_id == u) v = a.origen_id;
            if (v == -1) continue;

            pasos.push_back({PasoAnimacion::EXPLORAR, -1, u, v,
                "[DFS] Explorando " + nombreNodo(u) + " -> " + nombreNodo(v)});

            if (!vis[v]) {
                dfsPasos(v, vis, pasos);
            } else {
                pasos.push_back({PasoAnimacion::DESCARTAR, -1, u, v,
                    "[DFS] Ya visitado " + nombreNodo(v)});
            }
        }
        pasos.push_back({PasoAnimacion::CONFIRMAR, u, -1, -1,
            "[DFS] Completado " + nombreNodo(u)});
    }
};