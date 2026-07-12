#pragma once

struct EstadoUI;

struct VentanaAyuda {
    static void dibujar(EstadoUI& ui);
    static void dibujarTutorialRapido(EstadoUI& ui);

private:
    static void titulo(const char* t);
    static void subtitulo(const char* t);
    static void bullet_text_wrapped(const char* fmt);
    static void tip(const char* t);
    static void aviso(const char* t);
    static void dibujarContenido(int sec);

    static void secIntroduccion();
    static void secControles();
    static void secAeroGrafos();
    static void secGrafos();
    static void secFisicas();
    static void secDijkstra();
    static void secKruskal();
    static void secBFSDFS();
    static void secColoreo();
    static void secEuler();
    static void secIsomorfismo();
    static void secMatrices();
    static void secAnalizarRed();
    static void secPlantillas();
    static void secResaltadoVecinos();
};
