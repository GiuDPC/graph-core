#include <gtest/gtest.h>
#include "nucleo/SimuladorRed.h"

// Proporcionar la definicion del global de sonido (el constructor default es seguro,
// reproducir() es no-op si no se llamo a abrir())
Sonidos g_sonidos;

// Simulador con aristas unidireccionales
TEST(SimuladorTest, FalloUnidireccional) {
    Grafo g;
    g.agregarNodo(ImVec2(0,0));    // 0
    g.agregarNodo(ImVec2(100,0));  // 1
    g.agregarArista(0, 1, 1.0f, true);  // 0→1 dirigida

    SimuladorRed sim;
    sim.inicializar(g);

    // Verificar que arista 0→1 existe
    auto key01 = std::make_pair(0, 1);
    auto key10 = std::make_pair(1, 0);
    EXPECT_TRUE(sim.estado.aristas.count(key01));
    // key10 NO debe existir (solo hay A→B, no B→A)
    EXPECT_FALSE(sim.estado.aristas.count(key10));

    // Fallar solo 0→1
    sim.simularFalloArista(0, 1, g);
    EXPECT_FALSE(sim.estado.aristas[key01].activa);

    // Restaurar
    sim.restaurarArista(0, 1, g);
    EXPECT_TRUE(sim.estado.aristas[key01].activa);
}

TEST(SimuladorTest, BidirectionalIndependent) {
    Grafo g;
    g.agregarNodo(ImVec2(0,0));    // 0
    g.agregarNodo(ImVec2(100,0));  // 1
    g.agregarArista(0, 1, 2.0f, true);  // 0→1 dirigida (peso 2)
    g.agregarArista(1, 0, 5.0f, true);  // 1→0 dirigida (peso 5)

    SimuladorRed sim;
    sim.inicializar(g);

    auto key01 = std::make_pair(0, 1);
    auto key10 = std::make_pair(1, 0);

    // Ambas direcciones deben existir como estados independientes
    EXPECT_TRUE(sim.estado.aristas.count(key01));
    EXPECT_TRUE(sim.estado.aristas.count(key10));
    EXPECT_TRUE(sim.estado.aristas[key01].activa);
    EXPECT_TRUE(sim.estado.aristas[key10].activa);

    // Fallar solo 0→1
    sim.simularFalloArista(0, 1, g);
    EXPECT_FALSE(sim.estado.aristas[key01].activa);
    EXPECT_TRUE(sim.estado.aristas[key10].activa);  // 1→0 sigue activa

    // Restaurar 0→1 no afecta 1→0
    sim.restaurarArista(0, 1, g);
    EXPECT_TRUE(sim.estado.aristas[key01].activa);
    EXPECT_TRUE(sim.estado.aristas[key10].activa);
}
