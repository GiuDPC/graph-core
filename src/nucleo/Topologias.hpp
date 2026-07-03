#pragma once
#include "Grafo.hpp"

// Topologias de red predefinidas
namespace Topologias {

void empresarialBasica(Grafo& g, ImVec2 centro);
void meshTolerante(Grafo& g, ImVec2 centro);
void estrellaSimple(Grafo& g, ImVec2 centro);
void internetSimple(Grafo& g, ImVec2 centro);

} // namespace Topologias
