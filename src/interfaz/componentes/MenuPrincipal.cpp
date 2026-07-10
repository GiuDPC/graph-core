#include "interfaz/componentes/MenuPrincipal.hpp"
#include "Interfaz.hpp"
#include "persistencia/SerializadorJSON.hpp"
#include "persistencia/SerializadorGEXF.hpp"
#include "portable-file-dialogs.h"
#include "audio/Sonidos.hpp"
#include "interfaz/util/ExportadorPNG.hpp"

namespace MenuPrincipal {

void contenido(Interfaz& self, Grafo& red, GLFWwindow* ventana) {
    if (ImGui::MenuItem(ICON_FA_FILE_CIRCLE_PLUS " Nuevo Proyecto")) {
        red.limpiar();
        Animacion::reset(self.estado_grafos.anim_estado);
        self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
        self.registrarLog("Proyecto nuevo creado");
    }
    ImGui::Separator();
    if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Cargar...")) {
        auto resultado = pfd::open_file("Cargar topologia", ".", {"Archivos JSON", "*.json"}).result();
        if (resultado.empty()) {
            self.registrarLog("[!] Error: no se pudo abrir el dialogo de archivos.");
            self.registrarLog("[!] Verifica que 'zenity' este instalado: sudo apt install zenity");
            ImGui::OpenPopup("FallbackCargar");
        } else {
            Persistencia::cargar(red, resultado[0]);
            // Kick off the smooth load animation
            self.estado_ui.anim_carga.iniciar(red);
            Animacion::reset(self.estado_grafos.anim_estado);
            self.estado_grafos.ruta_optima.clear(); self.estado_grafos.aristas_mst.clear(); self.estado_grafos.mostrar_mst = false;
            self.registrarLog("[OK] Proyecto cargado: " + resultado[0]);
        }
    }
    if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Guardar como...")) {
        auto resultado = pfd::save_file("Guardar topologia", "topologia.json", {"Archivos JSON", "*.json"}).result();
        if (resultado.empty()) {
            self.registrarLog("[!] Error: no se pudo abrir el dialogo de guardado.");
            self.registrarLog("[!] Verifica que 'zenity' este instalado: sudo apt install zenity");
            ImGui::OpenPopup("FallbackGuardar");
        } else {
            std::string ruta = resultado;
            if (ruta.find(".json") == std::string::npos) ruta += ".json";
            Persistencia::guardar(red, ruta);
            self.registrarLog("[OK] Proyecto guardado: " + ruta);
        }
    }
    if (ImGui::MenuItem(ICON_FA_FILE_EXPORT " Exportar GEXF...")) {
        auto resultado = pfd::save_file("Exportar GEXF", "grafo.gexf", {"Archivos GEXF", "*.gexf"}).result();
        if (resultado.empty()) {
            self.registrarLog("[!] Error: no se pudo abrir el dialogo de exportacion.");
        } else {
            std::string ruta = resultado;
            if (ruta.find(".gexf") == std::string::npos) ruta += ".gexf";
            std::string xml = Persistencia::exportarGEXF(red);
            std::ofstream f(ruta);
            if (f.is_open()) {
                f << xml;
                self.registrarLog("[OK] Grafo exportado como GEXF: " + ruta);
            } else {
                self.registrarLog("[!] Error al escribir: " + ruta);
            }
        }
    }
    ImGui::Separator();
    if (ImGui::MenuItem(ICON_FA_IMAGE " Exportar PNG...")) {
        ExportadorPNG::exportar(self);
    }
    ImGui::Separator();
    if (ImGui::MenuItem(ICON_FA_DOOR_OPEN " Salir")) {
        glfwSetWindowShouldClose(ventana, true);
    }
    ImGui::Separator();
    if (ImGui::BeginMenu(ICON_FA_SLIDERS " Opciones")) {
        if (g_sonidos.funciona()) {
            ImGui::Text(ICON_FA_VOLUME_HIGH " Audio");
            static bool ultima_prueba = false;
            int vol_int = g_sonidos.getVolumenInt();
            if (ImGui::SliderInt("Vol. Global", &vol_int, 0, 100, "%d%%")) {
                g_sonidos.setVolumenInt(vol_int);
            }
            if (ImGui::SmallButton(ICON_FA_VOLUME_HIGH " Probar sonido")) {
                ultima_prueba = g_sonidos.probar();
            }
            if (ultima_prueba) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_CHECK);
            }
            const char* be = g_sonidos.backendString();
            ImGui::TextDisabled("Backend: %s | Tonos: %d | Vol: %d%%",
                be[0] ? be : "N/A", Sonidos::COUNT, g_sonidos.getVolumenInt());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), ICON_FA_VOLUME_XMARK " Audio no disponible");
            const char* be = g_sonidos.backendString();
            ImGui::TextDisabled("Engine: %s", be[0] ? be : "N/A");
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(ICON_FA_CIRCLE_INFO " Acerca de")) {
        self.estado_ui.mostrar_acerca_de = true;
    }
}

void dibujar(Interfaz& self, Grafo& red, GLFWwindow* ventana) {
    if (ImGui::BeginMainMenuBar()) {
        contenido(self, red, ventana);
        ImGui::EndMainMenuBar();
    }
}

} // namespace MenuPrincipal
