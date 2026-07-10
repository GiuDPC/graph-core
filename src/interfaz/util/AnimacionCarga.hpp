#pragma once

#include "imgui.h"
#include "nucleo/Grafo.hpp"
#include <vector>

// ---------------------------------------------------------------------------
// AnimacionCarga — smooth "graph appears node by node" effect when loading
//
// Design:
//   - Nodes spawn sequentially with easeOutBack scale (pop-in).
//   - Edges fade in with alpha interpolation after both endpoints are visible.
//   - The whole sequence completes in ~1.2 s regardless of graph size.
//   - Zero memory overhead once complete (data is cleared).
// ---------------------------------------------------------------------------
struct AnimacionCarga {
    bool activa = false;

    // Elapsed time since the animation was triggered
    float tiempo = 0.0f;

    // Total duration of the full sequence (seconds)
    float duracion_total = 1.2f;

    // Per-node data needed during the animation
    struct DatoNodo {
        int   id;
        float tiempo_inicio;  // when the node starts appearing
        float scale;          // current visual scale [0,1]
        float alpha;          // current alpha [0,1]
    };
    std::vector<DatoNodo> nodos_dato;

    // Alpha for edge reveal (global, trailing slightly behind nodes)
    float alpha_aristas = 0.0f;

    // Prepare the animation for a freshly-loaded graph.
    // Call this immediately after Persistencia::cargar().
    void iniciar(const Grafo& red);

    // Advance by `dt` seconds, updating scales/alphas.
    void actualizar(float dt);

    // Returns the visual scale to use for node `id`.
    // Returns 1.0 when the animation is not active.
    float escalaParaNodo(int id) const;

    // Returns the alpha [0,255] for edges.
    int alphaAristas() const;
};
