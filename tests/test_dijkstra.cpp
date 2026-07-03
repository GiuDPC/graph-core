#include <gtest/gtest.h>
#include "nucleo/algoritmos/Dijkstra.hpp"

// ── Dijkstra: ruta simple ──────────────────────────────────────────────────

TEST(DijkstraTest, SimplePath) {
    // Grafo: 0 - 4 → 1 - 3 → 2
    //        \_____ 10 _____/
    // Ruta optima: 0 → 1 → 2 con costo 7
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2

    g.agregarArista(0, 1, 4.0f);
    g.agregarArista(1, 2, 3.0f);
    g.agregarArista(0, 2, 10.0f);

    auto res = Algoritmos::dijkstra(g, 0, 2);

    EXPECT_TRUE(res.hay_ruta);
    EXPECT_FLOAT_EQ(res.costo_total, 7.0f);
    ASSERT_EQ(res.ruta.size(), 3);
    EXPECT_EQ(res.ruta[0], 0);
    EXPECT_EQ(res.ruta[1], 1);
    EXPECT_EQ(res.ruta[2], 2);
}

// ── Dijkstra: sin camino ───────────────────────────────────────────────────

TEST(DijkstraTest, NoPath) {
    // Grafo desconectado: 0-1 y 2-3 (componentes separados)
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2
    g.agregarNodo(ImVec2(300, 0)); // id 3

    g.agregarArista(0, 1, 2.0f);
    g.agregarArista(2, 3, 3.0f);

    auto res = Algoritmos::dijkstra(g, 0, 3);

    EXPECT_FALSE(res.hay_ruta);
    EXPECT_TRUE(res.ruta.empty());
}

// ── Dijkstra: origen = destino ────────────────────────────────────────────

TEST(DijkstraTest, OrigenIgualDestino) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1

    auto res = Algoritmos::dijkstra(g, 0, 0);
    EXPECT_FALSE(res.hay_ruta);
}

// ── Dijkstra: grafo con pesos ──────────────────────────────────────────────

TEST(DijkstraTest, WeightedGraph) {
    // Grafo: 0 --5-- 1 --2-- 3
    //        |       |
    //        3       4
    //        |       |
    //        2 --1-- 4
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(0, 100)); // id 2
    g.agregarNodo(ImVec2(200, 0)); // id 3
    g.agregarNodo(ImVec2(100, 100)); // id 4

    g.agregarArista(0, 1, 5.0f);
    g.agregarArista(0, 2, 3.0f);
    g.agregarArista(1, 3, 2.0f);
    g.agregarArista(1, 4, 4.0f);
    g.agregarArista(2, 4, 1.0f);

    // Ruta 0 → 3: 0→1→3 = 5+2 = 7
    auto res = Algoritmos::dijkstra(g, 0, 3);
    EXPECT_TRUE(res.hay_ruta);
    EXPECT_FLOAT_EQ(res.costo_total, 7.0f);

    // Ruta 0 → 4: 0→2→4 = 3+1 = 4 (mas corta que 0→1→4 = 5+4 = 9)
    auto res2 = Algoritmos::dijkstra(g, 0, 4);
    EXPECT_TRUE(res2.hay_ruta);
    EXPECT_FLOAT_EQ(res2.costo_total, 4.0f);
    ASSERT_EQ(res2.ruta.size(), 3);
    EXPECT_EQ(res2.ruta[0], 0);
    EXPECT_EQ(res2.ruta[1], 2);
    EXPECT_EQ(res2.ruta[2], 4);
}

// ── Dijkstra: grafo vacio ──────────────────────────────────────────────────

TEST(DijkstraTest, EmptyGraph) {
    Grafo g;
    auto res = Algoritmos::dijkstra(g, 0, 1);
    EXPECT_FALSE(res.hay_ruta);
}

// ── Dijkstra: un solo nodo ─────────────────────────────────────────────────

TEST(DijkstraTest, SingleNode) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    auto res = Algoritmos::dijkstra(g, 0, 0);
    EXPECT_FALSE(res.hay_ruta);
}

// ── Dijkstra: aristas dirigidas ────────────────────────────────────────────

TEST(DijkstraDirigidaTest, DirigidaPathExists) {
    // A→B (dirigida, peso 2), B→C (dirigida, peso 3)
    // Dijkstra de A a C → ruta A→B→C, costo 5
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0 = A
    g.agregarNodo(ImVec2(100, 0)); // id 1 = B
    g.agregarNodo(ImVec2(200, 0)); // id 2 = C

    g.agregarArista(0, 1, 2.0f, true);
    g.agregarArista(1, 2, 3.0f, true);

    auto res = Algoritmos::dijkstra(g, 0, 2);
    EXPECT_TRUE(res.hay_ruta);
    EXPECT_FLOAT_EQ(res.costo_total, 5.0f);
    ASSERT_EQ(res.ruta.size(), 3);
    EXPECT_EQ(res.ruta[0], 0);
    EXPECT_EQ(res.ruta[1], 1);
    EXPECT_EQ(res.ruta[2], 2);
}

TEST(DijkstraDirigidaTest, DirigidaReverseBlocked) {
    // A→B (dirigida, peso 2), no B→A
    // Dijkstra de B a A → NO hay ruta
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0 = A
    g.agregarNodo(ImVec2(100, 0)); // id 1 = B

    g.agregarArista(0, 1, 2.0f, true);

    auto res = Algoritmos::dijkstra(g, 1, 0);
    EXPECT_FALSE(res.hay_ruta);
}

TEST(DijkstraDirigidaTest, DirigidaLegacyUndirected) {
    // 0→1 (no-dirigida, peso 4), 1→2 (no-dirigida, peso 3)
    // Dijkstra de 2 a 0 → ruta 2→1→0 (legacy behavior preserved)
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(100, 0)); // id 1
    g.agregarNodo(ImVec2(200, 0)); // id 2

    g.agregarArista(0, 1, 4.0f);  // legacy sin flag → no-dirigida
    g.agregarArista(1, 2, 3.0f);  // legacy sin flag → no-dirigida

    auto res = Algoritmos::dijkstra(g, 2, 0);
    EXPECT_TRUE(res.hay_ruta);
    EXPECT_FLOAT_EQ(res.costo_total, 7.0f);
    ASSERT_EQ(res.ruta.size(), 3);
    EXPECT_EQ(res.ruta[0], 2);
    EXPECT_EQ(res.ruta[1], 1);
    EXPECT_EQ(res.ruta[2], 0);
}

TEST(DijkstraDirigidaTest, DirigidaMixedGraph) {
    // A→B (dirigida, peso 2), B→C (no-dirigida, peso 3), C→A (dirigida, peso 1)
    // Dijkstra de A a C → ruta A→B→C, costo 5
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0 = A
    g.agregarNodo(ImVec2(100, 0)); // id 1 = B
    g.agregarNodo(ImVec2(200, 0)); // id 2 = C

    g.agregarArista(0, 1, 2.0f, true);   // A→B dirigida
    g.agregarArista(1, 2, 3.0f, false);  // B→C no-dirigida
    g.agregarArista(2, 0, 1.0f, true);   // C→A dirigida

    auto res = Algoritmos::dijkstra(g, 0, 2);
    EXPECT_TRUE(res.hay_ruta);
    EXPECT_FLOAT_EQ(res.costo_total, 5.0f);
    ASSERT_EQ(res.ruta.size(), 3);
    EXPECT_EQ(res.ruta[0], 0);
    EXPECT_EQ(res.ruta[1], 1);
    EXPECT_EQ(res.ruta[2], 2);
}
