#pragma once

#include <string>
#include "../nucleo/Grafo.hpp"

namespace Persistencia {

inline std::string exportarGEXF(const Grafo& g) {
    std::string xml;
    xml.reserve(4096);

    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml += "<gexf xmlns=\"http://gexf.net/1.2\" version=\"1.2\">\n";
    xml += "  <meta>\n";
    xml += "    <creator>GraphCore</creator>\n";
    xml += "  </meta>\n";
    xml += "  <graph mode=\"static\" defaultedgetype=\"undirected\">\n";

    xml += "    <nodes>\n";
    for (const auto& n : g.nodos) {
        xml += "      <node id=\"" + std::to_string(n.id) +
               "\" label=\"" + n.nombre + "\"/>\n";
    }
    xml += "    </nodes>\n";

    xml += "    <edges>\n";
    for (size_t i = 0; i < g.aristas.size(); i++) {
        const auto& a = g.aristas[i];
        std::string tipo = a.es_dirigida ? "directed" : "undirected";
        xml += "      <edge id=\"" + std::to_string((int)i) +
               "\" source=\"" + std::to_string(a.origen_id) +
               "\" target=\"" + std::to_string(a.destino_id) +
               "\" weight=\"" + std::to_string(a.peso) +
               "\" type=\"" + tipo + "\"/>\n";
    }
    xml += "    </edges>\n";

    xml += "  </graph>\n";
    xml += "</gexf>\n";

    return xml;
}

} // namespace Persistencia
