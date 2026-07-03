#pragma once

class Grafo;
class HistorialGrafos;
struct EstadoUI;
struct EstadoGrafos;

namespace AtajosTeclado {
    void procesar(Grafo& red, HistorialGrafos& historial, EstadoUI& ui, EstadoGrafos& estado_grafos);
}
