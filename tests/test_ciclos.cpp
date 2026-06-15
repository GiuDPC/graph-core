#include <gtest/gtest.h>
#include "nucleo/algoritmos/Ciclos.h"

// ── Ciclos: deteccion de ciclos ───────────────────────────────────────────

TEST(CiclosTest, CicloNoDirigido) {
    // Grafo triangular 0-1-2-0 con aristas no-dirigidas
    // detectarCiclos() legacy (UnionFind) → tiene_ciclo = true
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2

    g.agregarArista(0, 1, 1.0f);  // legacy → no-dirigida
    g.agregarArista(1, 2, 1.0f);  // legacy → no-dirigida
    g.agregarArista(2, 0, 1.0f);  // legacy → no-dirigida

    auto res = Algoritmos::detectarCiclos(g);
    EXPECT_TRUE(res.tiene_ciclo);
}

TEST(CiclosTest, CicloDirigido) {
    // 0→1 (dirigida), 1→2 (dirigida), 2→0 (dirigida)
    // detectarCiclosDirigidos() → tiene_ciclo = true
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2

    g.agregarArista(0, 1, 1.0f, true);
    g.agregarArista(1, 2, 1.0f, true);
    g.agregarArista(2, 0, 1.0f, true);

    auto res = Algoritmos::detectarCiclosDirigidos(g);
    EXPECT_TRUE(res.tiene_ciclo);
}

TEST(CiclosTest, DAG) {
    // 0→1 (dirigida), 1→2 (dirigida), 0→2 (dirigida) — DAG
    // detectarCiclosDirigidos() → tiene_ciclo = false
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2

    g.agregarArista(0, 1, 1.0f, true);
    g.agregarArista(1, 2, 1.0f, true);
    g.agregarArista(0, 2, 1.0f, true);

    auto res = Algoritmos::detectarCiclosDirigidos(g);
    EXPECT_FALSE(res.tiene_ciclo);
}
