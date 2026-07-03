#include "Topologias.hpp"
#include "tipos/TipoHardware.hpp"

void Topologias::empresarialBasica(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;

    g.agregarNodo(ImVec2(cx, cy - 220), TipoHardware::Firewall);
    g.agregarNodo(ImVec2(cx, cy - 120), TipoHardware::Router);
    g.agregarNodo(ImVec2(cx - 120, cy), TipoHardware::Switch);
    g.agregarNodo(ImVec2(cx + 120, cy), TipoHardware::Switch);
    g.agregarNodo(ImVec2(cx - 200, cy + 120), TipoHardware::Servidor);
    g.agregarNodo(ImVec2(cx, cy + 150), TipoHardware::Servidor);
    g.agregarNodo(ImVec2(cx + 80, cy + 120), TipoHardware::Terminal);
    g.agregarNodo(ImVec2(cx + 200, cy + 120), TipoHardware::Terminal);

    g.agregarArista(0, 1, 1.0f);
    g.agregarArista(1, 2, 1.0f);
    g.agregarArista(1, 3, 1.0f);
    g.agregarArista(2, 4, 1.0f);
    g.agregarArista(2, 5, 1.0f);
    g.agregarArista(3, 6, 1.0f);
    g.agregarArista(3, 7, 1.0f);
    g.agregarArista(2, 3, 2.0f);
}

void Topologias::meshTolerante(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;
    float r = 150.0f;

    for (int i = 0; i < 6; i++) {
        float angle = i * (3.14159f * 2.0f / 6.0f) - 3.14159f / 2.0f;
        ImVec2 pos(cx + r * cosf(angle), cy + r * sinf(angle));
        g.agregarNodo(pos, TipoHardware::Router);
    }

    g.agregarNodo(ImVec2(cx, cy), TipoHardware::Router);

    for (int i = 0; i < 6; i++)
        g.agregarArista(i, (i + 1) % 6, 1.0f);

    for (int i = 0; i < 6; i++)
        g.agregarArista(6, i, 1.0f);
}

void Topologias::estrellaSimple(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;

    g.agregarNodo(ImVec2(cx, cy), TipoHardware::Switch);

    float r = 160.0f;
    for (int i = 0; i < 5; i++) {
        float angle = i * (3.14159f * 2.0f / 5.0f) - 3.14159f / 2.0f;
        ImVec2 pos(cx + r * cosf(angle), cy + r * sinf(angle));
        g.agregarNodo(pos, TipoHardware::Terminal);
        g.agregarArista(0, i + 1, 1.0f);
    }
}

void Topologias::internetSimple(Grafo& g, ImVec2 centro) {
    g.limpiar();
    float cx = centro.x, cy = centro.y;

    g.agregarNodo(ImVec2(cx - 250, cy - 100), TipoHardware::Router);
    g.agregarNodo(ImVec2(cx,       cy - 150), TipoHardware::Router);
    g.agregarNodo(ImVec2(cx + 250, cy - 100), TipoHardware::Router);
    g.agregarNodo(ImVec2(cx - 200, cy),       TipoHardware::Router);
    g.agregarNodo(ImVec2(cx + 200, cy),       TipoHardware::Router);
    g.agregarNodo(ImVec2(cx - 200, cy + 120), TipoHardware::Firewall);
    g.agregarNodo(ImVec2(cx + 200, cy + 120), TipoHardware::Firewall);
    g.agregarNodo(ImVec2(cx - 280, cy + 220), TipoHardware::Servidor);
    g.agregarNodo(ImVec2(cx - 120, cy + 220), TipoHardware::Servidor);
    g.agregarNodo(ImVec2(cx + 120, cy + 220), TipoHardware::Terminal);
    g.agregarNodo(ImVec2(cx + 280, cy + 220), TipoHardware::Terminal);

    g.agregarArista(0, 1, 10.0f);
    g.agregarArista(1, 2, 10.0f);
    g.agregarArista(0, 2, 8.0f);
    g.agregarArista(0, 3, 5.0f);
    g.agregarArista(2, 4, 5.0f);
    g.agregarArista(3, 5, 2.0f);
    g.agregarArista(4, 6, 2.0f);
    g.agregarArista(5, 7, 1.0f);
    g.agregarArista(5, 8, 1.0f);
    g.agregarArista(6, 9, 1.0f);
    g.agregarArista(6, 10, 1.0f);
}
