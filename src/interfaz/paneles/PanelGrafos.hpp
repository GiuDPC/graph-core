#pragma once

#include "imgui.h"

class Interfaz;
class Grafo;

namespace PanelGrafos {
    void sidebarInfo(Interfaz& self, Grafo& red);
    void panelContextual(Interfaz& self, Grafo& red);
}
