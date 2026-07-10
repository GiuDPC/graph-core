#pragma once

#include "imgui.h"
#include <string>
#include <vector>

// nota de texto flotante en el lienzo
struct Anotacion {
    ImVec2      posicion;       // coordenadas mundo (sin zoom/offset)
    std::string texto;
    ImU32       color = IM_COL32(255, 220, 80, 255);
    int         id    = 0;
};

// contenedor de anotaciones para el grafo
struct EstadoAnotaciones {
    std::vector<Anotacion> items;
    int  contador_ids   = 0;
    int  editando_id    = -1;   // id de la nota en edicion
    bool creando        = false;
    char buffer[256]    = {};

    int agregar(ImVec2 pos) {
        Anotacion a;
        a.id = contador_ids++;
        a.posicion = pos;
        a.texto = "";
        items.push_back(a);
        return a.id;
    }

    Anotacion* obtener(int id) {
        for (auto& a : items)
            if (a.id == id) return &a;
        return nullptr;
    }

    void eliminar(int id) {
        items.erase(
            std::remove_if(items.begin(), items.end(),
                [id](const Anotacion& a) { return a.id == id; }),
            items.end());
        if (editando_id == id) editando_id = -1;
    }
};
