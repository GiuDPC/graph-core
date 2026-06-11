#pragma once
#include "Grafo.h"

// Topologias de red predefinidas
namespace Topologias {

// Red empresarial basica
inline void empresarialBasica(Grafo& g, ImVec2 centro) {
    g.limpiar();
    // Internet (entrada)
    float cx = centro.x, cy = centro.y;

    // Firewall (entrada)
    g.agregarNodo(ImVec2(cx, cy - 220), TipoHardware::Firewall);           // 0: FW0
    // Router principal
    g.agregarNodo(ImVec2(cx, cy - 120), TipoHardware::Router);             // 1: RTR0
    // Switch de distribucion
    g.agregarNodo(ImVec2(cx - 120, cy), TipoHardware::Switch);             // 2: SW0
    g.agregarNodo(ImVec2(cx + 120, cy), TipoHardware::Switch);             // 3: SW1
    // Servidores
    g.agregarNodo(ImVec2(cx - 200, cy + 120), TipoHardware::Servidor);     // 4: SRV0
    g.agregarNodo(ImVec2(cx, cy + 150), TipoHardware::Servidor);           // 5: SRV1
    // Terminales
    g.agregarNodo(ImVec2(cx + 80, cy + 120), TipoHardware::Terminal);      // 6: PC0
    g.agregarNodo(ImVec2(cx + 200, cy + 120), TipoHardware::Terminal);     // 7: PC1

    // Conexiones
    g.agregarArista(0, 1, 1.0f);   // FW → RTR (1 Gbps normalizado)
    g.agregarArista(1, 2, 1.0f);   // RTR → SW0
    g.agregarArista(1, 3, 1.0f);   // RTR → SW1
    g.agregarArista(2, 4, 1.0f);   // SW0 → SRV0
    g.agregarArista(2, 5, 1.0f);   // SW0 → SRV1
    g.agregarArista(3, 6, 1.0f);   // SW1 → PC0
    g.agregarArista(3, 7, 1.0f);   // SW1 → PC1
    g.agregarArista(2, 3, 2.0f);   // SW0 → SW1 (enlace troncal de 2 Gbps)
}

// Red mesh (alta tolerancia a fallos)
inline void meshTolerante(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;
    float r = 150.0f;

    // 6 routers en hexagono
    for (int i = 0; i < 6; i++) {
        float angle = i * (3.14159f * 2.0f / 6.0f) - 3.14159f / 2.0f;
        ImVec2 pos(cx + r * cosf(angle), cy + r * sinf(angle));
        g.agregarNodo(pos, TipoHardware::Router);
    }

    // Router central
    g.agregarNodo(ImVec2(cx, cy), TipoHardware::Router);  // 6: centro

    // Conexiones anillo exterior
    for (int i = 0; i < 6; i++)
        g.agregarArista(i, (i + 1) % 6, 1.0f);

    // Conexiones radiales (centro a cada vertice)
    for (int i = 0; i < 6; i++)
        g.agregarArista(6, i, 1.0f);
}

// Red estrella (vulnerable al nodo central)
inline void estrellaSimple(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;

    // Switch central
    g.agregarNodo(ImVec2(cx, cy), TipoHardware::Switch);  // 0: SW0 (nodo critico)

    // 5 terminales alrededor
    float r = 160.0f;
    for (int i = 0; i < 5; i++) {
        float angle = i * (3.14159f * 2.0f / 5.0f) - 3.14159f / 2.0f;
        ImVec2 pos(cx + r * cosf(angle), cy + r * sinf(angle));
        g.agregarNodo(pos, TipoHardware::Terminal);
        g.agregarArista(0, i + 1, 1.0f);
    }
}

// Segmento de Internet simplificado
inline void internetSimple(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;

    // ISPs (routers backbone)
    g.agregarNodo(ImVec2(cx - 250, cy - 100), TipoHardware::Router);  // 0: ISP-A
    g.agregarNodo(ImVec2(cx,       cy - 150), TipoHardware::Router);  // 1: ISP-B
    g.agregarNodo(ImVec2(cx + 250, cy - 100), TipoHardware::Router);  // 2: ISP-C
    // Routers de borde
    g.agregarNodo(ImVec2(cx - 200, cy),       TipoHardware::Router);  // 3: Edge-A
    g.agregarNodo(ImVec2(cx + 200, cy),       TipoHardware::Router);  // 4: Edge-B
    // Firewalls empresariales
    g.agregarNodo(ImVec2(cx - 200, cy + 120), TipoHardware::Firewall); // 5: FW-A
    g.agregarNodo(ImVec2(cx + 200, cy + 120), TipoHardware::Firewall); // 6: FW-B
    // Servidores finales
    g.agregarNodo(ImVec2(cx - 280, cy + 220), TipoHardware::Servidor); // 7: Web-A
    g.agregarNodo(ImVec2(cx - 120, cy + 220), TipoHardware::Servidor); // 8: DB-A
    g.agregarNodo(ImVec2(cx + 120, cy + 220), TipoHardware::Terminal); // 9: PC-B
    g.agregarNodo(ImVec2(cx + 280, cy + 220), TipoHardware::Terminal); // 10: PC-C

    // Backbone entre ISPs
    g.agregarArista(0, 1, 10.0f);
    g.agregarArista(1, 2, 10.0f);
    g.agregarArista(0, 2, 8.0f);
    // ISP a edge
    g.agregarArista(0, 3, 5.0f);
    g.agregarArista(2, 4, 5.0f);
    // Edge a firewall
    g.agregarArista(3, 5, 2.0f);
    g.agregarArista(4, 6, 2.0f);
    // Firewall a servidores/terminales
    g.agregarArista(5, 7, 1.0f);
    g.agregarArista(5, 8, 1.0f);
    g.agregarArista(6, 9, 1.0f);
    g.agregarArista(6, 10, 1.0f);
}

} // namespace Topologias
