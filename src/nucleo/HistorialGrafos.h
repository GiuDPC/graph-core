#pragma once
#include "Grafo.h"
#include <deque>

struct HistorialGrafos {
    static constexpr size_t MAX_HISTORIAL = 50;
    
    std::deque<Grafo> undo_stack;
    std::deque<Grafo> redo_stack;

    void capturar(const Grafo& estado_actual) {
        if (undo_stack.size() >= MAX_HISTORIAL)
            undo_stack.pop_front();
        undo_stack.push_back(estado_actual);
        redo_stack.clear();
    }

    bool deshacer(Grafo& actual) {
        if (undo_stack.empty()) return false;
        redo_stack.push_back(actual);
        actual = undo_stack.back();
        undo_stack.pop_back();
        return true;
    }

    bool rehacer(Grafo& actual) {
        if (redo_stack.empty()) return false;
        undo_stack.push_back(actual);
        actual = redo_stack.back();
        redo_stack.pop_back();
        return true;
    }
    
    void limpiar() {
        undo_stack.clear();
        redo_stack.clear();
    }
};
