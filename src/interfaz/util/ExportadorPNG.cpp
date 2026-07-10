// implementacion de stb_image_write (header-only, single TU)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "interfaz/util/stb_image_write.h"

#include "interfaz/util/ExportadorPNG.hpp"
#include "Interfaz.hpp"
#include "portable-file-dialogs.h"
#include "audio/Sonidos.hpp"
#include "IconsFontAwesome6.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>

extern Sonidos g_sonidos;

namespace ExportadorPNG {

// voltear pixeles verticalmente (gl lee de abajo a arriba)
static void flipVertical(std::vector<unsigned char>& px, int w, int h, int ch) {
    int row = w * ch;
    for (int y = 0; y < h / 2; ++y) {
        unsigned char* top = px.data() + y * row;
        unsigned char* bot = px.data() + (h - 1 - y) * row;
        for (int x = 0; x < row; ++x)
            std::swap(top[x], bot[x]);
    }
}

bool capturarRegion(const std::string& ruta, int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return false;

    int ch = 3;
    std::vector<unsigned char> px(static_cast<size_t>(w) * h * ch);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());

    flipVertical(px, w, h, ch);

    return stbi_write_png(ruta.c_str(), w, h, ch, px.data(), w * ch) != 0;
}

void exportar(Interfaz& self) {
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
    if (ruta.size() < 4 || ruta.compare(ruta.size() - 4, 4, ".png") != 0)
        ruta += ".png";

    // obtener tamano real del framebuffer para escalar coordenadas (hidpi)
    GLFWwindow* win = glfwGetCurrentContext();
    int fb_w = 0, fb_h = 0, win_w = 0, win_h = 0;
    if (win) {
        glfwGetFramebufferSize(win, &fb_w, &fb_h);
        glfwGetWindowSize(win, &win_w, &win_h);
    }
    if (fb_w <= 0 || win_w <= 0) {
        self.registrarLog("[!] No se pudo obtener el tamano del framebuffer.");
        return;
    }

    // factor de escala hidpi
    float sx = (float)fb_w / (float)win_w;
    float sy = (float)fb_h / (float)win_h;

    // coordenadas del lienzo en pixeles del framebuffer
    int rx = (int)(self.estado_ui.lienzo_origin.x * sx);
    int ry = fb_h - (int)((self.estado_ui.lienzo_origin.y + self.estado_ui.lienzo_size.y) * sy);
    int rw = (int)(self.estado_ui.lienzo_size.x * sx);
    int rh = (int)(self.estado_ui.lienzo_size.y * sy);

    // clamp
    if (rx < 0) { rw += rx; rx = 0; }
    if (ry < 0) { rh += ry; ry = 0; }
    if (rx + rw > fb_w) rw = fb_w - rx;
    if (ry + rh > fb_h) rh = fb_h - ry;

    if (rw <= 0 || rh <= 0) {
        self.registrarLog("[!] Region del lienzo invalida.");
        return;
    }

    if (capturarRegion(ruta, rx, ry, rw, rh)) {
        self.registrarLog("[OK] Imagen exportada: " + ruta +
                          " (" + std::to_string(rw) + "x" + std::to_string(rh) + " px)");
        g_sonidos.reproducir(Sonidos::TRIUNFO_DIJKSTRA);
    } else {
        self.registrarLog("[!] Error al escribir PNG: " + ruta);
        g_sonidos.reproducir(Sonidos::DESCARTAR);
    }
}

} // namespace ExportadorPNG
