#include "interfaz/util/AnimacionCarga.hpp"
#include "interfaz/util/Easing.hpp"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// AnimacionCarga implementation
// ---------------------------------------------------------------------------

void AnimacionCarga::iniciar(const Grafo& red) {
    activa          = true;
    tiempo          = 0.0f;
    alpha_aristas   = 0.0f;
    nodos_dato.clear();

    if (red.nodos.empty()) {
        activa = false;
        return;
    }

    // Stagger nodes evenly over 70% of the total duration.
    // The remaining 30% lets the last node finish its pop-in and edges fade.
    size_t n        = red.nodos.size();
    float ventana   = duracion_total * 0.70f;
    float step      = (n <= 1) ? 0.0f : (ventana / static_cast<float>(n - 1));

    for (size_t i = 0; i < n; ++i) {
        DatoNodo d;
        d.id           = red.nodos[i].id;
        d.tiempo_inicio = static_cast<float>(i) * step;
        d.scale         = 0.0f;
        d.alpha         = 0.0f;
        nodos_dato.push_back(d);
    }
}

void AnimacionCarga::actualizar(float dt) {
    if (!activa) return;

    tiempo += dt;

    // Duration of the individual pop-in per node (fixed 0.35 s feels snappy)
    const float pop_dur = 0.35f;

    bool todos_completos = true;

    for (auto& d : nodos_dato) {
        float t_local = tiempo - d.tiempo_inicio;

        if (t_local < 0.0f) {
            d.scale = 0.0f;
            d.alpha = 0.0f;
            todos_completos = false;
        } else if (t_local >= pop_dur) {
            d.scale = 1.0f;
            d.alpha = 1.0f;
        } else {
            float t_norm = t_local / pop_dur;
            // easeOutBack gives that satisfying "overshoot & settle" pop
            d.scale = Easing::easeOutBack(t_norm);
            d.alpha = std::min(1.0f, t_norm * 2.0f);   // alpha appears faster
            todos_completos = false;
        }
    }

    // Edges start fading in after 40% of the total sequence
    float edge_start = duracion_total * 0.40f;
    float edge_end   = duracion_total * 1.00f;
    if (tiempo >= edge_start) {
        float t_e = (tiempo - edge_start) / (edge_end - edge_start);
        alpha_aristas = std::min(1.0f, t_e);
    }

    // Finish once all nodes are done and edges are fully visible
    if (todos_completos && alpha_aristas >= 1.0f) {
        activa = false;
        nodos_dato.clear();  // free memory
    }
}

float AnimacionCarga::escalaParaNodo(int id) const {
    if (!activa) return 1.0f;
    for (const auto& d : nodos_dato) {
        if (d.id == id) return d.scale;
    }
    return 1.0f;
}

int AnimacionCarga::alphaAristas() const {
    if (!activa) return 255;
    return static_cast<int>(alpha_aristas * 255.0f);
}
