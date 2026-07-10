#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../nucleo/Grafo.hpp"

using json = nlohmann::json;

// Serializacion json — separado del grafo
namespace Persistencia {

inline bool guardar(const Grafo& g, const std::string& ruta) {
    json j;
    j["version"] = "3.1";
    j["contador_ids"] = g.contador_ids;

    j["nodos"] = json::array();
    for (const auto& n : g.nodos) {
        j["nodos"].push_back({
            {"id", n.id}, {"nombre", n.nombre},
            {"x", n.posicion.x}, {"y", n.posicion.y},
            {"tipo", (int)n.tipo}
        });
    }

    j["aristas"] = json::array();
    for (const auto& a : g.aristas) {
        j["aristas"].push_back({
            {"origen", a.origen_id}, {"destino", a.destino_id},
            {"peso", a.peso}, {"dirigida", a.es_dirigida}
        });
    }

    j["anotaciones"] = json::array();
    for (const auto& an : g.anotaciones.items) {
        j["anotaciones"].push_back({
            {"id", an.id}, {"texto", an.texto},
            {"x", an.posicion.x}, {"y", an.posicion.y},
            {"color", an.color}
        });
    }
    j["contador_anotaciones"] = g.anotaciones.contador_ids;

    std::ofstream archivo(ruta);
    if (!archivo.is_open()) return false;
    archivo << j.dump(4);
    return true;
}

inline bool cargar(Grafo& g, const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) return false;

    json j;
    try { archivo >> j; }
    catch (...) { return false; }

    g.limpiar();
    g.contador_ids = j.value("contador_ids", 0);

    for (const auto& nj : j["nodos"]) {
        TipoHardware tipo = (TipoHardware)nj.value("tipo", 0);
        Nodo nuevo(nj["id"], ImVec2(nj["x"], nj["y"]), tipo);
        nuevo.nombre = nj.value("nombre", nuevo.nombre);
        g.nodos.push_back(nuevo);
    }
    for (const auto& aj : j["aristas"]) {
        bool dirigida = aj.value("dirigida", false);
        g.aristas.push_back(Arista(aj["origen"], aj["destino"], aj["peso"], dirigida));
    }

    g.anotaciones.contador_ids = j.value("contador_anotaciones", 0);
    if (j.contains("anotaciones")) {
        for (const auto& an_j : j["anotaciones"]) {
            Anotacion an;
            an.id = an_j.value("id", 0);
            an.texto = an_j.value("texto", "");
            an.posicion = ImVec2(an_j.value("x", 0.0f), an_j.value("y", 0.0f));
            an.color = an_j.value("color", IM_COL32(255, 220, 80, 255));
            g.anotaciones.items.push_back(an);
        }
    }

    return true;
}

} // namespace Persistencia
