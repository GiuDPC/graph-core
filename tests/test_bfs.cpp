#include <gtest/gtest.h>
#include "nucleo/algoritmos/BFS.hpp"

// ── BFS: aristas dirigidas ────────────────────────────────────────────────

TEST(BfsDirigidaTest, BfsDirigidaBasic) {
    // A→B (dirigida), B→C (dirigida)
    // BFS desde A debe visitar A, B, C
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0 = A
    g.agregarNodo(ImVec2(100, 0)); // id 1 = B
    g.agregarNodo(ImVec2(200, 0)); // id 2 = C

    g.agregarArista(0, 1, 1.0f, true);
    g.agregarArista(1, 2, 1.0f, true);

    auto res = Algoritmos::BFS::bfs(g, 0);

    ASSERT_EQ(res.orden_visita.size(), 3);
    EXPECT_EQ(res.orden_visita[0], 0);
    EXPECT_EQ(res.orden_visita[1], 1);
    EXPECT_EQ(res.orden_visita[2], 2);
}

TEST(BfsDirigidaTest, BfsDirigidaDoesNotGoBackwards) {
    // A→B (dirigida), pero NO A→C ni B→C
    // BFS desde B debe visitar solo B (no puede ir a A porque es dirigida y no existe B→A)
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0 = A
    g.agregarNodo(ImVec2(100, 0)); // id 1 = B

    g.agregarArista(0, 1, 1.0f, true);  // solo A→B

    auto res = Algoritmos::BFS::bfs(g, 1);  // desde B

    ASSERT_EQ(res.orden_visita.size(), 1);
    EXPECT_EQ(res.orden_visita[0], 1);
}
