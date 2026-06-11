#include <gtest/gtest.h>
#include "nucleo/UnionFind.h"

// ── UnionFind: operaciones basicas ─────────────────────────────────────────

TEST(UnionFindTest, InitialState) {
    UnionFind uf(5);

    // Cada elemento es su propio representante
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(uf.encontrar(i), i);
    }

    // Elementos distintos no estan conectados
    EXPECT_FALSE(uf.mismoComponente(0, 1));
    EXPECT_FALSE(uf.mismoComponente(2, 4));
}

TEST(UnionFindTest, UnionAndConnectivity) {
    UnionFind uf(5);

    // Unir 0-1, 1-2, 3-4
    EXPECT_TRUE(uf.unir(0, 1));
    EXPECT_TRUE(uf.unir(1, 2));
    EXPECT_TRUE(uf.unir(3, 4));

    // 0, 1, 2 deben estar en el mismo componente
    EXPECT_TRUE(uf.mismoComponente(0, 1));
    EXPECT_TRUE(uf.mismoComponente(0, 2));
    EXPECT_TRUE(uf.mismoComponente(1, 2));

    // 3 y 4 deben estar en el mismo componente
    EXPECT_TRUE(uf.mismoComponente(3, 4));

    // Los grupos 0-1-2 y 3-4 deben estar separados
    EXPECT_FALSE(uf.mismoComponente(0, 3));
    EXPECT_FALSE(uf.mismoComponente(2, 4));
}

TEST(UnionFindTest, UnionReturnsFalseWhenAlreadyConnected) {
    UnionFind uf(3);

    uf.unir(0, 1);
    // Segunda union del mismo par debe retornar false
    EXPECT_FALSE(uf.unir(0, 1));
    // Union transitiva (0 ya conectado con 1)
    EXPECT_FALSE(uf.unir(1, 0));
}

TEST(UnionFindTest, PathCompression) {
    UnionFind uf(10);

    // Crear una cadena: 0-1-2-3-4
    uf.unir(0, 1);
    uf.unir(1, 2);
    uf.unir(2, 3);
    uf.unir(3, 4);

    // Todos deben ser del mismo componente
    EXPECT_TRUE(uf.mismoComponente(0, 4));
    EXPECT_TRUE(uf.mismoComponente(1, 3));

    // Elemento 5 no conectado
    EXPECT_FALSE(uf.mismoComponente(0, 5));
}

TEST(UnionFindTest, MultipleComponents) {
    UnionFind uf(8);

    // Componente A: 0, 1, 2
    uf.unir(0, 1);
    uf.unir(1, 2);

    // Componente B: 3, 4
    uf.unir(3, 4);

    // Componente C: 5, 6, 7
    uf.unir(5, 6);
    uf.unir(6, 7);

    // Verificar componentes
    EXPECT_TRUE(uf.mismoComponente(0, 2));
    EXPECT_TRUE(uf.mismoComponente(3, 4));
    EXPECT_TRUE(uf.mismoComponente(5, 7));

    // Diferentes componentes
    EXPECT_FALSE(uf.mismoComponente(0, 3));
    EXPECT_FALSE(uf.mismoComponente(2, 5));
    EXPECT_FALSE(uf.mismoComponente(4, 6));
}
