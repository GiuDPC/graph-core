#pragma once

#include "Grafo.hpp"

// generadores de grafos clasicos predefinidos
namespace Plantillas {

// grafo completo Kn
Grafo completo(int n);

// ciclo Cn
Grafo ciclo(int n);

// camino Pn
Grafo camino(int n);

// estrella Sn (1 centro + n hojas)
Grafo estrella(int n);

// rueda Wn (ciclo + centro)
Grafo rueda(int n);

// bipartito completo Km,n
Grafo bipartito(int m, int n);

// petersen (clasico, 10 nodos)
Grafo petersen();

} // namespace Plantillas
