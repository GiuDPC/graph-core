#pragma once

#include <string>

class Interfaz;

// captura una region del framebuffer opengl y la guarda como png
namespace ExportadorPNG {

// captura un rectangulo del framebuffer gl
bool capturarRegion(const std::string& ruta, int x, int y, int w, int h);

// abre dialogo de guardado y captura solo el area del lienzo
void exportar(Interfaz& self);

} // namespace ExportadorPNG
