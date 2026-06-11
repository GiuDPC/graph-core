#pragma once

#include "interfaz/Interfaz.h"
#include "persistencia/SerializadorJSON.h"
#include "portable-file-dialogs.h"
#include "audio/Sonidos.h"

// Barra de menú principal: Archivo, Opciones, Acerca de
namespace MenuPrincipal {

inline void dibujar(Interfaz& self, Grafo& red, GLFWwindow* ventana) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(ICON_FA_FILE " Archivo")) {
            if (ImGui::MenuItem(ICON_FA_FILE_CIRCLE_PLUS " Nuevo Proyecto")) {
                red.limpiar();
                Animacion::reset(self.anim_estado);
                self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
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
                    Animacion::reset(self.anim_estado);
                    self.ruta_optima.clear(); self.aristas_mst.clear(); self.mostrar_mst = false;
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
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_DOOR_OPEN " Salir")) {
                glfwSetWindowShouldClose(ventana, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_SLIDERS " Opciones")) {
            if (g_sonidos.funciona()) {
                ImGui::Text(ICON_FA_VOLUME_HIGH " Audio");
                float vol = g_sonidos.getVolumen();
                if (ImGui::SliderFloat("Volumen", &vol, 0.0f, 1.0f, "%.0f%%")) {
                    g_sonidos.setVolumen(vol);
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                    ICON_FA_VOLUME_XMARK " Audio no disponible");
                ImGui::TextDisabled("No se detecto dispositivo de sonido");
            }
            ImGui::Separator();
            ImGui::TextDisabled("OptiClusters v4.0 — NetSim Pro");
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(ICON_FA_CIRCLE_INFO " Acerca de")) {
            ImGui::OpenPopup("Acerca de OptiClusters");
        }
        ImGui::EndMainMenuBar();
    }
}

} // namespace MenuPrincipal
