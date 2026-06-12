#pragma once

#include <string>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct TextureInfo {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

inline void setWindowIcon(GLFWwindow* window, const char* filename) {
    int width, height, channels;
    unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
    if (pixels) {
        GLFWimage images[1];
        images[0].width = width;
        images[0].height = height;
        images[0].pixels = pixels;
        // Wayland no soporta iconos de ventana, ignorar error silenciosamente
        auto prev_cb = glfwSetErrorCallback(nullptr);
        glfwSetWindowIcon(window, 1, images);
        glfwSetErrorCallback(prev_cb);
        stbi_image_free(pixels);
    }
}

inline TextureInfo cargarTextura(const std::string& ruta) {
    TextureInfo ti;
    int channels;
    unsigned char* data = stbi_load(ruta.c_str(), &ti.width, &ti.height, &channels, 4);
    if (!data) return ti;

    glGenTextures(1, &ti.id);
    glBindTexture(GL_TEXTURE_2D, ti.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ti.width, ti.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    return ti;
}
