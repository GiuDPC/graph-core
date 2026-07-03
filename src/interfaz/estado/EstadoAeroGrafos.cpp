#include "EstadoAeroGrafos.hpp"

void EstadoAeroGrafos::actualizarMensajes(float dt) {
    for (auto it = mensajes.begin(); it != mensajes.end(); ) {
        it->tiempo_restante -= dt;
        if (it->tiempo_restante <= 0.0f) {
            it = mensajes.erase(it);
        } else {
            ++it;
        }
    }
}
