#include "Plantillas.hpp"
#include <cmath>
#include <functional>
#include <random>

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
    if (n > 50) n = 50;
    
    float startX = 150.0f;
    float startY = 150.0f;
    float spacingX = 80.0f;
    float spacingY = 80.0f;
    int items_per_row = 8;
    
    for (int i = 0; i < n; ++i) {
        int row = i / items_per_row;
        int col = i % items_per_row;
        // alternar direccion para hacer curva en S
        if (row % 2 == 1) {
            col = (items_per_row - 1) - col;
        }
        g.agregarNodo(ImVec2(startX + col * spacingX, startY + row * spacingY));
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

Grafo malla(int m, int n) {
    Grafo g;
    if (m < 2) m = 2; if (m > 15) m = 15;
    if (n < 2) n = 2; if (n > 15) n = 15;
    float startX = 400.0f - (n - 1) * 35.0f;
    float startY = 300.0f - (m - 1) * 35.0f;
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            g.agregarNodo(ImVec2(startX + j * 70.0f, startY + i * 70.0f));
            g.nodos.back().nombre = "N" + std::to_string(i*n + j);
        }
    }
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            int current = i * n + j;
            if (j < n - 1) g.agregarArista(current, current + 1, 1.0f); // derecha
            if (i < m - 1) g.agregarArista(current, current + n, 1.0f); // abajo
        }
    }
    return g;
}

Grafo arbol_binario(int n) {
    Grafo g;
    if (n < 3) n = 3;
    if (n > 63) n = 63; // max 6 niveles
    
    // calculo recursivo visual para posicionar bonito
    std::function<void(int, int, float, float, float)> generar_nodo = 
    [&](int id, int nivel, float x, float y, float x_offset) {
        if (id >= n) return;
        g.agregarNodo(ImVec2(x, y));
        g.nodos.back().nombre = "V" + std::to_string(id);
        
        int izq = 2 * id + 1;
        int der = 2 * id + 2;
        float y_next = y + 80.0f;
        float x_next_offset = x_offset * 0.5f;
        
        if (izq < n) {
            generar_nodo(izq, nivel + 1, x - x_offset, y_next, x_next_offset);
            g.agregarArista(id, izq, 1.0f);
        }
        if (der < n) {
            generar_nodo(der, nivel + 1, x + x_offset, y_next, x_next_offset);
            g.agregarArista(id, der, 1.0f);
        }
    };
    
    // raiz id=0
    generar_nodo(0, 0, 400.0f, 100.0f, 200.0f);

    std::vector<int> real_idx(n, -1);
    for (size_t i = 0; i < g.nodos.size(); ++i) {
        int logico = std::stoi(g.nodos[i].nombre.substr(1));
        real_idx[logico] = i;
    }
    g.aristas.clear();
    for (int i = 0; i < n; ++i) {
        int izq = 2 * i + 1;
        int der = 2 * i + 2;
        if (izq < n) g.agregarArista(real_idx[i], real_idx[izq], 1.0f);
        if (der < n) g.agregarArista(real_idx[i], real_idx[der], 1.0f);
    }
    
    return g;
}

Grafo mundo_pequeno(int n) {
    // modelo watts-strogatz simplificado
    Grafo g;
    if (n < 6) n = 6;
    if (n > 40) n = 40;
    ponerEnCirculo(g, n, 400.0f, 300.0f, 140.0f + n * 6.0f);
    
    // anillo base k=4 (cada nodo se conecta a 2 vecinos de cada lado)
    for (int i = 0; i < n; ++i) {
        g.agregarArista(i, (i + 1) % n, 1.0f);
        g.agregarArista(i, (i + 2) % n, 1.0f);
    }
    
    // reconexion (atajos aleatorios)
    auto& gen = Grafo::obtenerGeneradorAleatorio();
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::uniform_int_distribution<int> rand_n(0, n - 1);
    
    float beta = 0.2f; // probabilidad de reconexion
    for (size_t a_idx = 0; a_idx < g.aristas.size(); ++a_idx) {
        if (dist(gen) < beta) {
            int u = g.aristas[a_idx].origen_id;
            int v_nuevo = rand_n(gen);
            if (v_nuevo != u) {
                g.aristas[a_idx].destino_id = v_nuevo;
            }
        }
    }
    
    return g;
}

} // namespace Plantillas
