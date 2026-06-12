#include "interfaz/Interfaz.h"
#include "audio/Sonidos.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "IconsFontAwesome6.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include "nucleo/Grafo.h"

// Punteros globales (accesibles desde Interfaz)
ImFont* g_fontMono = nullptr;
Sonidos g_sonidos; // sistema de audio global

#include "interfaz/util/TextureLoader.h"

static void callbackErrorGlfw(int error, const char* descripcion) {
    fprintf(stderr, "[GLFW ERROR %d] %s\n", error, descripcion);
}

// Tema dark engineering v2.0
void configurarTemaIngenieria() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 4.0f;
    s.FrameRounding     = 3.0f;
    s.GrabRounding      = 3.0f;
    s.TabRounding       = 3.0f;
    s.ScrollbarRounding = 3.0f;
    s.PopupRounding     = 4.0f;
    s.ChildRounding     = 3.0f;

    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 1.0f;
    s.PopupBorderSize   = 1.0f;

    s.WindowPadding     = ImVec2(10, 10);
    s.FramePadding      = ImVec2(8, 5);
    s.ItemSpacing       = ImVec2(8, 6);
    s.ItemInnerSpacing  = ImVec2(6, 4);
    s.ScrollbarSize     = 12.0f;
    s.GrabMinSize       = 8.0f;

    ImVec4* c = s.Colors;

    // Fondos
    c[ImGuiCol_WindowBg]        = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    c[ImGuiCol_ChildBg]         = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
    c[ImGuiCol_PopupBg]         = ImVec4(0.11f, 0.11f, 0.15f, 0.97f);
    c[ImGuiCol_MenuBarBg]       = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

    // Bordes
    c[ImGuiCol_Border]          = ImVec4(0.22f, 0.24f, 0.28f, 0.65f);
    c[ImGuiCol_BorderShadow]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Titulos
    c[ImGuiCol_TitleBg]         = ImVec4(0.08f, 0.08f, 0.11f, 1.00f);
    c[ImGuiCol_TitleBgActive]   = ImVec4(0.10f, 0.13f, 0.18f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]= ImVec4(0.06f, 0.06f, 0.08f, 0.80f);

    // Inputs
    c[ImGuiCol_FrameBg]         = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
    c[ImGuiCol_FrameBgHovered]  = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBgActive]   = ImVec4(0.16f, 0.20f, 0.28f, 1.00f);

    // Botones — turquesa profundo
    c[ImGuiCol_Button]          = ImVec4(0.00f, 0.42f, 0.36f, 0.80f);
    c[ImGuiCol_ButtonHovered]   = ImVec4(0.00f, 0.56f, 0.47f, 0.90f);
    c[ImGuiCol_ButtonActive]    = ImVec4(0.00f, 0.70f, 0.55f, 1.00f);

    // Headers (collapsing, tree, selectable)
    c[ImGuiCol_Header]          = ImVec4(0.00f, 0.38f, 0.32f, 0.45f);
    c[ImGuiCol_HeaderHovered]   = ImVec4(0.00f, 0.50f, 0.42f, 0.65f);
    c[ImGuiCol_HeaderActive]    = ImVec4(0.00f, 0.60f, 0.48f, 0.85f);

    // Tabs
    c[ImGuiCol_Tab]             = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    c[ImGuiCol_TabHovered]      = ImVec4(0.00f, 0.50f, 0.42f, 0.80f);
    c[ImGuiCol_TabSelected]     = ImVec4(0.00f, 0.42f, 0.36f, 1.00f);
    c[ImGuiCol_TabDimmed]       = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    c[ImGuiCol_TabDimmedSelected] = ImVec4(0.00f, 0.30f, 0.25f, 1.00f);

    // Texto
    c[ImGuiCol_Text]            = ImVec4(0.86f, 0.88f, 0.91f, 1.00f);
    c[ImGuiCol_TextDisabled]    = ImVec4(0.42f, 0.44f, 0.48f, 1.00f);

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]     = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]   = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.32f, 0.36f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.38f, 0.40f, 0.44f, 1.00f);

    // Separadores
    c[ImGuiCol_Separator]       = ImVec4(0.22f, 0.24f, 0.28f, 0.50f);
    c[ImGuiCol_SeparatorHovered]= ImVec4(0.00f, 0.60f, 0.48f, 0.70f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.75f, 0.58f, 1.00f);

    // Checks, sliders
    c[ImGuiCol_CheckMark]       = ImVec4(0.00f, 0.83f, 0.67f, 1.00f);
    c[ImGuiCol_SliderGrab]      = ImVec4(0.00f, 0.60f, 0.48f, 1.00f);
    c[ImGuiCol_SliderGrabActive]= ImVec4(0.00f, 0.83f, 0.67f, 1.00f);

    // Resize grip
    c[ImGuiCol_ResizeGrip]        = ImVec4(0.00f, 0.42f, 0.36f, 0.30f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.56f, 0.47f, 0.60f);
    c[ImGuiCol_ResizeGripActive]  = ImVec4(0.00f, 0.70f, 0.55f, 1.00f);

    // Docking
    c[ImGuiCol_DockingPreview]  = ImVec4(0.00f, 0.70f, 0.55f, 0.40f);
    c[ImGuiCol_DockingEmptyBg]  = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);

    // Modal dim
    c[ImGuiCol_ModalWindowDimBg]= ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

// Main
int main(int, char**) {
    glfwSetErrorCallback(callbackErrorGlfw);
    if (!glfwInit()) {
        fprintf(stderr, "Error: No se pudo inicializar GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* ventana = glfwCreateWindow(
        1500, 900, "graphCore", nullptr, nullptr
    );
    if (!ventana) {
        fprintf(stderr, "Error: No se pudo crear la ventana GLFW\n");
        glfwTerminate();
        return 1;
    }

    setWindowIcon(ventana, "graph-core-logo.png");

    glfwMakeContextCurrent(ventana);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = nullptr; // Desactivar el guardado del layout en ini para usar siempre la estructura predefinida

    // --- Cargar fuentes ---
    ImFont* fontMain = io.Fonts->AddFontFromFileTTF("recursos/fuentes/Roboto-Regular.ttf", 16.0f);
    if (fontMain) {
        // Fusionar FontAwesome en la fuente principal
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = 16.0f;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        io.Fonts->AddFontFromFileTTF(
            "recursos/fuentes/Font Awesome 6 Free-Solid-900.otf",
            14.0f, &icons_config, icon_ranges
        );
    } else {
        io.Fonts->AddFontDefault();
    }

    // Fuente monospace para matrices y logs
    g_fontMono = io.Fonts->AddFontFromFileTTF("recursos/fuentes/JetBrainsMono-Regular.ttf", 15.0f);

    // Inicializar sistema de audio
    if (g_sonidos.abrir()) {
        printf("[OK] Audio iniciado\n");
    } else {
        printf("[!] Audio no disponible (sin dispositivo de sonido)\n");
    }

    configurarTemaIngenieria();

    ImGui_ImplGlfw_InitForOpenGL(ventana, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 colorFondo = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    Grafo red;
    static Interfaz ui;
    ui.estado_ui.fontMono = g_fontMono;

    TextureInfo logo = cargarTextura("/home/giuseppe/Escritorio/graphCore/graph-core-logo.png");
    setWindowIcon(ventana, "/home/giuseppe/Escritorio/graphCore/graph-core-logo.png");
    ui.estado_ui.id_logo = logo.id;
    ui.estado_ui.width_logo = logo.width;
    ui.estado_ui.height_logo = logo.height;

    while (!glfwWindowShouldClose(ventana)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui.dibujar(red, ventana);

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(ventana, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(colorFondo.x, colorFondo.y, colorFondo.z, colorFondo.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(ventana);
    }

    g_sonidos.cerrar();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ventana);
    glfwTerminate();

    return 0;
}
