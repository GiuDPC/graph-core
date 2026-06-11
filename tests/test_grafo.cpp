#include <gtest/gtest.h>
#include "nucleo/Grafo.h"

// ── Grafo: creacion y nodos ───────────────────────────────────────────────

TEST(GrafoTest, EmptyGraph) {
    Grafo g;
    EXPECT_TRUE(g.estaVacio());
    EXPECT_EQ(g.nodos.size(), 0);
    EXPECT_EQ(g.aristas.size(), 0);
}

TEST(GrafoTest, AddNodes) {
    Grafo g;
    g.agregarNodo(ImVec2(100, 100));
    g.agregarNodo(ImVec2(200, 200));
    g.agregarNodo(ImVec2(300, 300));

    EXPECT_FALSE(g.estaVacio());
    EXPECT_EQ(g.nodos.size(), 3);
    EXPECT_EQ(g.nodos[0].id, 0);
    EXPECT_EQ(g.nodos[1].id, 1);
    EXPECT_EQ(g.nodos[2].id, 2);
}

TEST(GrafoTest, ObtenerNodo) {
    Grafo g;
    g.agregarNodo(ImVec2(50, 50));

    Nodo* n = g.obtenerNodo(0);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->id, 0);

    EXPECT_EQ(g.obtenerNodo(999), nullptr);
}

TEST(GrafoTest, EliminarNodo) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    EXPECT_EQ(g.nodos.size(), 2);

    g.eliminarNodo(0);
    EXPECT_EQ(g.nodos.size(), 1);
    EXPECT_EQ(g.obtenerNodo(0), nullptr);
    EXPECT_NE(g.obtenerNodo(1), nullptr);
}

// ── Grafo: aristas ─────────────────────────────────────────────────────────

TEST(GrafoTest, AddEdges) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarNodo(ImVec2(20, 20));

    g.agregarArista(0, 1, 5.0f);
    g.agregarArista(1, 2, 3.0f);

    EXPECT_EQ(g.aristas.size(), 2);

    const Arista* a01 = g.obtenerArista(0, 1);
    ASSERT_NE(a01, nullptr);
    EXPECT_FLOAT_EQ(a01->peso_actual, 5.0f);
}

TEST(GrafoTest, NoDuplicateEdges) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));

    g.agregarArista(0, 1, 2.0f);
    g.agregarArista(1, 0, 3.0f);  // duplicado (no dirigida)

    EXPECT_EQ(g.aristas.size(), 1);
}

TEST(GrafoTest, NoSelfEdge) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));

    g.agregarArista(0, 0, 1.0f);  // self-loop
    EXPECT_EQ(g.aristas.size(), 0);
}

TEST(GrafoTest, EdgeDeletionWithNode) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 3.0f);
    ASSERT_EQ(g.aristas.size(), 1);

    g.eliminarNodo(0);
    EXPECT_EQ(g.aristas.size(), 0);
}

// ── Grafo: vecinos ─────────────────────────────────────────────────────────

TEST(GrafoTest, Vecinos) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));    // id 0
    g.agregarNodo(ImVec2(10, 10));  // id 1
    g.agregarNodo(ImVec2(20, 20));  // id 2

    g.agregarArista(0, 1, 1.0f);
    g.agregarArista(0, 2, 1.0f);

    auto vec = g.vecinos(0);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_NE(std::find(vec.begin(), vec.end(), 1), vec.end());
    EXPECT_NE(std::find(vec.begin(), vec.end(), 2), vec.end());
}

TEST(GrafoTest, Grado) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));    // id 0
    g.agregarNodo(ImVec2(10, 10));  // id 1
    g.agregarNodo(ImVec2(20, 20));  // id 2

    g.agregarArista(0, 1, 1.0f);
    g.agregarArista(0, 2, 2.0f);

    EXPECT_EQ(g.gradoNodo(0), 2);
    EXPECT_EQ(g.gradoNodo(1), 1);
    EXPECT_EQ(g.gradoNodo(2), 1);
}

// ── Grafo: limpiar ─────────────────────────────────────────────────────────

TEST(GrafoTest, Limpiar) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 1.0f);
    ASSERT_EQ(g.nodos.size(), 2);

    g.limpiar();
    EXPECT_TRUE(g.estaVacio());
    EXPECT_EQ(g.nodos.size(), 0);
    EXPECT_EQ(g.aristas.size(), 0);
}
