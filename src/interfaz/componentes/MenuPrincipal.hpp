#pragma once

class Interfaz;
class Grafo;
struct GLFWwindow;

namespace MenuPrincipal {
    void contenido(Interfaz& self, Grafo& red, GLFWwindow* ventana);
    void dibujar(Interfaz& self, Grafo& red, GLFWwindow* ventana);
}
