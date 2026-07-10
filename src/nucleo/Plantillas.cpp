#include "Plantillas.hpp"
#include <cmath>

static const float PI = 3.14159265f;

// posiciona n nodos en circulo centrado en cx,cy
static void ponerEnCirculo(Grafo& g, int n, float cx, float cy, float radio) {
    for (int i = 0; i < n; ++i) {
        float ang = (2.0f * PI / n) * i - PI / 2.0f;
        g.agregarNodo(ImVec2(cx + cosf(ang) * radio, cy + sinf(ang) * radio));
        g.nodos.back().nombre = "V" + std::to_string(i);
    }
}

namespace Plantillas {

Grafo completo(int n) {
    Grafo g;
    if (n < 2) n = 2;
    if (n > 20) n = 20;
    ponerEnCirculo(g, n, 400.0f, 300.0f, 140.0f + n * 8.0f);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            g.agregarArista(i, j, 1.0f);
    return g;
}

Grafo ciclo(int n) {
    Grafo g;
    if (n < 3) n = 3;
    if (n > 30) n = 30;
    ponerEnCirculo(g, n, 400.0f, 300.0f, 140.0f + n * 6.0f);
    for (int i = 0; i < n; ++i)
        g.agregarArista(i, (i + 1) % n, 1.0f);
    return g;
}

Grafo camino(int n) {
    Grafo g;
    if (n < 2) n = 2;
    if (n > 30) n = 30;
    float startX = 200.0f;
    float spacing = std::min(60.0f, 600.0f / (float)(n - 1));
    for (int i = 0; i < n; ++i) {
        g.agregarNodo(ImVec2(startX + i * spacing, 300.0f));
        g.nodos.back().nombre = "V" + std::to_string(i);
    }
    for (int i = 0; i < n - 1; ++i)
        g.agregarArista(i, i + 1, 1.0f);
    return g;
}

Grafo estrella(int n) {
    Grafo g;
    if (n < 3) n = 3;
    if (n > 20) n = 20;
    // centro
    g.agregarNodo(ImVec2(400.0f, 300.0f));
    g.nodos.back().nombre = "C";
    // hojas
    for (int i = 0; i < n; ++i) {
        float ang = (2.0f * PI / n) * i - PI / 2.0f;
        float r = 160.0f + n * 5.0f;
        g.agregarNodo(ImVec2(400.0f + cosf(ang) * r, 300.0f + sinf(ang) * r));
        g.nodos.back().nombre = "V" + std::to_string(i);
        g.agregarArista(0, i + 1, 1.0f);
    }
    return g;
}

Grafo rueda(int n) {
    Grafo g;
    if (n < 4) n = 4;
    if (n > 20) n = 20;
    int borde = n - 1;
    // centro
    g.agregarNodo(ImVec2(400.0f, 300.0f));
    g.nodos.back().nombre = "C";
    // borde del ciclo
    for (int i = 0; i < borde; ++i) {
        float ang = (2.0f * PI / borde) * i - PI / 2.0f;
        float r = 160.0f + borde * 5.0f;
        g.agregarNodo(ImVec2(400.0f + cosf(ang) * r, 300.0f + sinf(ang) * r));
        g.nodos.back().nombre = "V" + std::to_string(i);
    }
    // aristas del ciclo
    for (int i = 1; i <= borde; ++i) {
        int next = (i % borde) + 1;
        g.agregarArista(i, next, 1.0f);
    }
    // radios al centro
    for (int i = 1; i <= borde; ++i)
        g.agregarArista(0, i, 1.0f);
    return g;
}

Grafo bipartito(int m, int n) {
    Grafo g;
    if (m < 1) m = 1; if (m > 10) m = 10;
    if (n < 1) n = 1; if (n > 10) n = 10;
    float spacingM = std::min(80.0f, 500.0f / (float)std::max(1, m - 1));
    float spacingN = std::min(80.0f, 500.0f / (float)std::max(1, n - 1));
    float startMx = 400.0f - (m - 1) * spacingM * 0.5f;
    float startNx = 400.0f - (n - 1) * spacingN * 0.5f;
    // conjunto A (arriba)
    for (int i = 0; i < m; ++i) {
        g.agregarNodo(ImVec2(startMx + i * spacingM, 200.0f));
        g.nodos.back().nombre = "A" + std::to_string(i);
    }
    // conjunto B (abajo)
    for (int i = 0; i < n; ++i) {
        g.agregarNodo(ImVec2(startNx + i * spacingN, 400.0f));
        g.nodos.back().nombre = "B" + std::to_string(i);
    }
    // todas las aristas cruzadas
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            g.agregarArista(i, m + j, 1.0f);
    return g;
}

Grafo petersen() {
    Grafo g;
    // anillo exterior (5 nodos)
    for (int i = 0; i < 5; ++i) {
        float ang = (2.0f * PI / 5) * i - PI / 2.0f;
        g.agregarNodo(ImVec2(400.0f + cosf(ang) * 200.0f, 300.0f + sinf(ang) * 200.0f));
        g.nodos.back().nombre = "E" + std::to_string(i);
    }
    // anillo interior (5 nodos)
    for (int i = 0; i < 5; ++i) {
        float ang = (2.0f * PI / 5) * i - PI / 2.0f;
        g.agregarNodo(ImVec2(400.0f + cosf(ang) * 90.0f, 300.0f + sinf(ang) * 90.0f));
        g.nodos.back().nombre = "I" + std::to_string(i);
    }
    // ciclo exterior
    for (int i = 0; i < 5; ++i)
        g.agregarArista(i, (i + 1) % 5, 1.0f);
    // pentagrama interior (saltos de 2)
    for (int i = 0; i < 5; ++i)
        g.agregarArista(5 + i, 5 + ((i + 2) % 5), 1.0f);
    // radios
    for (int i = 0; i < 5; ++i)
        g.agregarArista(i, 5 + i, 1.0f);
    return g;
}

} // namespace Plantillas
