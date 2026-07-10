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

// malla bidimensional (grid m x n)
Grafo malla(int m, int n);

// arbol binario perfecto o casi perfecto
Grafo arbol_binario(int n);

// red de mundo pequeno (anillo con atajos aleatorios)
Grafo mundo_pequeno(int n);

} // namespace Plantillas
