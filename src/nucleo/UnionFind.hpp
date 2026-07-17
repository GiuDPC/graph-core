#pragma once
#include <vector>

// Union-find con compresión de caminos y unión por rango
class UnionFind {
public:
    explicit UnionFind(int n) {
        padre.resize(n);
        rango.resize(n, 0);
        for (int i = 0; i < n; i++) padre[i] = i;
    }

    int encontrar(int i) {
        if (padre[i] != i) padre[i] = encontrar(padre[i]); // compresion de caminos
        return padre[i];
    }

    // Retorna true si la union se realizo
    bool unir(int x, int y) {
        int rx = encontrar(x);
        int ry = encontrar(y);
        if (rx == ry) return false;
        if (rango[rx] < rango[ry])      padre[rx] = ry;
        else if (rango[rx] > rango[ry]) padre[ry] = rx;
        else { padre[ry] = rx; rango[rx]++; }
        return true;
    }

    bool mismoComponente(int x, int y) {
        return encontrar(x) == encontrar(y);
    }

private:
    std::vector<int> padre;
    std::vector<int> rango;
};
