#include <gtest/gtest.h>
#include "nucleo/SimuladorRed.hpp"

// g_sonidos global needed by SimuladorRed (audio is a no-op when not initialized)
#include "audio/Sonidos.hpp"
Sonidos g_sonidos;

// ── SimuladorRed: inicialización ───────────────────────────────────────────

TEST(SimuladorTest, InicializarVacio) {
    Grafo g;
    SimuladorRed sim;
    sim.inicializar(g);
    EXPECT_TRUE(sim.estado.activa);
    EXPECT_FLOAT_EQ(sim.estado.tiempo, 0.0f);
    EXPECT_EQ(sim.totalPaquetesEnviados(), 0);
    EXPECT_EQ(sim.totalPaquetesPerdidos(), 0);
    EXPECT_EQ(sim.totalPaquetesEntregados(), 0);
}

TEST(SimuladorTest, InicializarConNodos) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));

    SimuladorRed sim;
    sim.inicializar(g);
    EXPECT_TRUE(sim.estado.activa);
    EXPECT_EQ(sim.estado.nodos.size(), 2);
    EXPECT_TRUE(sim.estado.nodos[0].activo);
    EXPECT_TRUE(sim.estado.nodos[1].activo);
}

TEST(SimuladorTest, InicializarConAristas) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 5.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    EXPECT_EQ(sim.estado.aristas.size(), 1);
    EXPECT_TRUE((sim.estado.aristas[{0, 1}].activa));
    EXPECT_FLOAT_EQ((sim.estado.aristas[{0, 1}].bandwidth_mbps), 50.0f); // peso * 10
}

// ── SimuladorRed: routing ─────────────────────────────────────────────────

TEST(SimuladorTest, TablaRuteoDirecta) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1
    g.agregarArista(0, 1, 1.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    auto tabla = sim.tablaRuteo(g, 0);
    ASSERT_EQ(tabla.size(), 1);
    EXPECT_EQ(tabla[0].first, 1);
}

TEST(SimuladorTest, TablaRuteoSinVecinos) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1

    SimuladorRed sim;
    sim.inicializar(g);
    auto tabla = sim.tablaRuteo(g, 0);
    EXPECT_TRUE(tabla.empty()); // sin aristas → sin rutas
}

TEST(SimuladorTest, TablaRuteoNodoInexistente) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));

    SimuladorRed sim;
    sim.inicializar(g);
    auto tabla = sim.tablaRuteo(g, 999);
    EXPECT_TRUE(tabla.empty());
}

// ── SimuladorRed: flujos y paquetes ───────────────────────────────────────

TEST(SimuladorTest, EnviarFlujo) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1
    g.agregarArista(0, 1, 10.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    sim.enviarFlujo(0, 1, 2.0f, "HTTP", 10.0f, g);
    EXPECT_EQ(sim.estado.flujos.size(), 1);
    EXPECT_FLOAT_EQ(sim.estado.flujos[0].tiempo_restante, 10.0f);
}

TEST(SimuladorTest, EnviarFlujoPresetGeneraPaquetes) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1

    // Necesita al menos 2 nodos para que haya ruta
    g.agregarArista(0, 1, 10.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    sim.enviarFlujoPreset(g, 1);

    // Flujos creados inmediatamente
    EXPECT_GE(sim.estado.flujos.size(), 1);
    // Paquetes se generan en tick() via procesarColas()
    sim.tick(g, 0.2f);
    EXPECT_GT(sim.totalPaquetesEnviados(), 0);
}

// ── SimuladorRed: fallos ──────────────────────────────────────────────────

TEST(SimuladorTest, FalloNodo) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1

    SimuladorRed sim;
    sim.inicializar(g);

    sim.simularFalloNodo(0, g);
    EXPECT_FALSE(sim.estado.nodos[0].activo);
    EXPECT_TRUE(sim.estado.nodos[1].activo); // otros nodos no afectados
}

TEST(SimuladorTest, FalloArista) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 5.0f);

    SimuladorRed sim;
    sim.inicializar(g);

    sim.simularFalloArista(0, 1, g);
    EXPECT_FALSE((sim.estado.aristas[{0, 1}].activa));
}

TEST(SimuladorTest, RestaurarNodo) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));

    SimuladorRed sim;
    sim.inicializar(g);

    sim.simularFalloNodo(0, g);
    EXPECT_FALSE(sim.estado.nodos[0].activo);

    sim.restaurarNodo(0, g);
    EXPECT_TRUE(sim.estado.nodos[0].activo);
}

TEST(SimuladorTest, RestaurarArista) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 5.0f);

    SimuladorRed sim;
    sim.inicializar(g);

    sim.simularFalloArista(0, 1, g);
    EXPECT_FALSE((sim.estado.aristas[{0, 1}].activa));

    sim.restaurarArista(0, 1, g);
    EXPECT_TRUE((sim.estado.aristas[{0, 1}].activa));
}

// ── SimuladorRed: uso de aristas ──────────────────────────────────────────

TEST(SimuladorTest, UsoAristaSinCarga) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 10.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    EXPECT_FLOAT_EQ(sim.usoArista(0, 1), 0.0f); // sin flujo → 0 uso
}

TEST(SimuladorTest, UsoAristaInexistente) {
    Grafo g;
    SimuladorRed sim;
    sim.inicializar(g);
    EXPECT_FLOAT_EQ(sim.usoArista(0, 1), 0.0f);
}

// ── SimuladorRed: tick ────────────────────────────────────────────────────

TEST(SimuladorTest, TickSinFlujoNoCrashea) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 1.0f);

    SimuladorRed sim;
    sim.inicializar(g);

    EXPECT_NO_THROW(sim.tick(g, 1.0f));
    EXPECT_GT(sim.estado.tiempo, 0.0f);
}

TEST(SimuladorTest, TickConFlujo) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));   // id 0
    g.agregarNodo(ImVec2(10, 10)); // id 1
    g.agregarArista(0, 1, 10.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    sim.enviarFlujo(0, 1, 2.0f, "HTTP", 10.0f, g);

    EXPECT_EQ(sim.estado.flujos.size(), 1);

    sim.tick(g, 1.0f);
    // tick processa colas y genera paquetes
    EXPECT_GE(sim.estado.tiempo, 0.0f);
}

// ── SimuladorRed: eventos ─────────────────────────────────────────────────

TEST(SimuladorTest, EventosRegistrados) {
    Grafo g;
    g.agregarNodo(ImVec2(0, 0));
    g.agregarNodo(ImVec2(10, 10));
    g.agregarArista(0, 1, 1.0f);

    SimuladorRed sim;
    sim.inicializar(g);
    sim.simularFalloNodo(0, g);

    EXPECT_GT(sim.estado.log_eventos.size(), 0); // deberia haber un evento de fallo
}

// ── SimuladorRed: notificar ───────────────────────────────────────────────

TEST(SimuladorTest, NotificarAgrega) {
    Grafo g;
    SimuladorRed sim;
    sim.inicializar(g);
    sim.notificar("test", 0xFFFF3333, 2.0f);

    EXPECT_EQ(sim.estado.notificaciones.size(), 1);
    EXPECT_EQ(sim.estado.notificaciones[0].mensaje, "test");
}
