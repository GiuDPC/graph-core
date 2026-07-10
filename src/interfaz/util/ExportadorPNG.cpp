// stb_image_write implementation (header-only, single TU)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "interfaz/util/stb_image_write.h"

#include "interfaz/util/ExportadorPNG.hpp"
#include "Interfaz.hpp"
#include "portable-file-dialogs.h"
#include "audio/Sonidos.hpp"
#include "IconsFontAwesome6.h"

// OpenGL headers (already available via GLFW / ImGui backend)
#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>
#include <cstring>

extern Sonidos g_sonidos;

namespace ExportadorPNG {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Flip the pixel buffer vertically in-place (GL reads bottom-to-top).
static void flipVertical(std::vector<unsigned char>& pixels, int w, int h, int channels) {
    int row_bytes = w * channels;
    for (int y = 0; y < h / 2; ++y) {
        unsigned char* top = pixels.data() + y * row_bytes;
        unsigned char* bot = pixels.data() + (h - 1 - y) * row_bytes;
        for (int x = 0; x < row_bytes; ++x)
            std::swap(top[x], bot[x]);
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool capturarFramebuffer(const std::string& ruta, int ancho, int alto) {
    if (ancho <= 0 || alto <= 0) return false;

    // Read the current GL default framebuffer (what's visible on screen)
    int channels = 3;  // RGB — no alpha needed, saves ~25% size
    std::vector<unsigned char> pixels(static_cast<size_t>(ancho) * alto * channels);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, ancho, alto, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL origin is bottom-left; PNG expects top-left
    flipVertical(pixels, ancho, alto, channels);

    // stb writes PNG — lossless, handles large images fine
    int ok = stbi_write_png(ruta.c_str(),
                            ancho, alto, channels,
                            pixels.data(),
                            ancho * channels);
    return ok != 0;
}

void exportar(Interfaz& self) {
    // --- pick destination file ---
    auto resultado = pfd::save_file(
        "Exportar imagen PNG",
        "grafo.png",
        {"Imagen PNG", "*.png"}
    ).result();

    if (resultado.empty()) {
        self.registrarLog("[!] Exportacion cancelada.");
        return;
    }

    std::string ruta = resultado;
    if (ruta.size() < 4 ||
        ruta.compare(ruta.size() - 4, 4, ".png") != 0) {
        ruta += ".png";
    }

    // Query the real framebuffer size (handles HiDPI correctly)
    GLFWwindow* win = glfwGetCurrentContext();
    int fb_w = 0, fb_h = 0;
    if (win) {
        glfwGetFramebufferSize(win, &fb_w, &fb_h);
    }

    if (fb_w <= 0 || fb_h <= 0) {
        self.registrarLog("[!] No se pudo obtener el tamano del framebuffer.");
        return;
    }

    if (capturarFramebuffer(ruta, fb_w, fb_h)) {
        self.registrarLog("[OK] Imagen exportada: " + ruta +
                          " (" + std::to_string(fb_w) + "x" + std::to_string(fb_h) + " px)");
        g_sonidos.reproducir(Sonidos::TRIUNFO_DIJKSTRA);
    } else {
        self.registrarLog("[!] Error al escribir PNG: " + ruta);
        g_sonidos.reproducir(Sonidos::DESCARTAR);
    }
}

} // namespace ExportadorPNG
