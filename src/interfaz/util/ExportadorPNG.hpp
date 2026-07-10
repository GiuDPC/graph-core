#pragma once

#include <string>

class Grafo;
class Interfaz;

// Reads back the current OpenGL framebuffer and writes it as PNG.
// Uses stb_image_write (header-only, no extra dependency).
// Resolution: the actual framebuffer size at the moment of the call.
namespace ExportadorPNG {

// Capture the current GL framebuffer and save to `ruta`.
// Returns true on success.
bool capturarFramebuffer(const std::string& ruta, int ancho, int alto);

// High-level helper: opens a save-file dialog and captures.
// `ventana` is used to query the real framebuffer size.
void exportar(Interfaz& self);

} // namespace ExportadorPNG
