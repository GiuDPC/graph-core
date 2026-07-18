#include "LienzoAeroGrafos.hpp"
#include "Interfaz.hpp"
#include "imgui_internal.h"
#include "IconsFontAwesome6.h"
#include "nucleo/datos/DatosMundo.hpp"
#include "interfaz/estado/EstadoAeroGrafos.hpp"
#include "interfaz/util/TextureLoader.hpp"
#include "interfaz/util/Easing.hpp"
#include "audio/Sonidos.hpp"
#include <cmath>
#include <algorithm>
#include <string>
#include <cstdio>

extern Sonidos g_sonidos;

// Static helper functions

static ImVec2 virtualAPantalla(ImVec2 v, ImVec2 centro, float zoom, ImVec2 tam) {
    return ImVec2(
        (v.x - centro.x) * zoom + tam.x * 0.5f,
        (v.y - centro.y) * zoom + tam.y * 0.5f
    );
}

static ImVec2 pantallaAVirtual(ImVec2 s, ImVec2 centro, float zoom, ImVec2 tam) {
    return ImVec2(
        (s.x - tam.x * 0.5f) / zoom + centro.x,
        (s.y - tam.y * 0.5f) / zoom + centro.y
    );
}

static void cargarTexturaMundo(EstadoAeroGrafos& estado) {
    if (estado.textura_cargada) return;
    TextureInfo tex = cargarTextura("recursos/texturas/mundo_equirectangular.png");
    if (tex.id == 0)
        tex = cargarTextura("recursos/texturas/mundo_equirectangular.jpg");
    if (tex.id != 0) {
        estado.id_textura_mundo = tex.id;
        estado.ancho_textura = tex.width;
        estado.alto_textura = tex.height;
        estado.textura_cargada = true;
    }
}

static void dibujarOcean(ImDrawList* dl, ImVec2 pos, ImVec2 sz, bool noche) {
    if (noche) {
        dl->AddRectFilledMultiColor(pos,
            ImVec2(pos.x + sz.x, pos.y + sz.y),
            IM_COL32(1, 3, 12, 255),
            IM_COL32(1, 3, 12, 255),
            IM_COL32(2, 6, 20, 255),
            IM_COL32(2, 6, 20, 255));
    } else {
        dl->AddRectFilledMultiColor(pos,
            ImVec2(pos.x + sz.x, pos.y + sz.y),
            IM_COL32(3, 12, 30, 255),
            IM_COL32(3, 12, 30, 255),
            IM_COL32(6, 22, 50, 255),
            IM_COL32(6, 22, 50, 255));
    }
}

static void dibujarTexturaMundo(ImDrawList* dl, EstadoAeroGrafos& estado,
                                 ImVec2 pos, ImVec2 sz) {
    if (!estado.textura_cargada) return;
    ImU32 col = estado.modo_noche ? IM_COL32(60, 70, 100, 255) : IM_COL32_WHITE;

    ImVec2 p_min = virtualAPantalla(ImVec2(0, 0), estado.centro_mapa, estado.zoom_mapa, sz);
    ImVec2 p_max = virtualAPantalla(ImVec2(DatosMundo::ANCHO_VIRTUAL, DatosMundo::ALTO_VIRTUAL), estado.centro_mapa, estado.zoom_mapa, sz);

    dl->AddImage((ImTextureID)(intptr_t)estado.id_textura_mundo,
                 ImVec2(pos.x + p_min.x, pos.y + p_min.y),
                 ImVec2(pos.x + p_max.x, pos.y + p_max.y),
                 ImVec2(0, 0), ImVec2(1, 1), col);
}

static void dibujarGrid(ImDrawList* dl, EstadoAeroGrafos& estado,
                         ImVec2 pos, ImVec2 sz) {
    if (!estado.mostrar_grid) return;

    float paso = (estado.zoom_mapa < 0.3f) ? 60.0f :
                 (estado.zoom_mapa < 0.8f) ? 30.0f : 15.0f;
    if (estado.zoom_mapa < 0.15f) return;

    ImU32 col, col_resaltado;
    if (estado.modo_noche) {
        col = IM_COL32(30, 50, 80, 30);
        col_resaltado = IM_COL32(50, 80, 120, 50);
    } else {
        col = IM_COL32(50, 70, 110, 50);
        col_resaltado = IM_COL32(70, 110, 170, 80);
    }
    float grosor = 1.0f;

    ImVec2 p_min = virtualAPantalla(ImVec2(0, 0), estado.centro_mapa, estado.zoom_mapa, sz);
    ImVec2 p_max = virtualAPantalla(ImVec2(DatosMundo::ANCHO_VIRTUAL, DatosMundo::ALTO_VIRTUAL), estado.centro_mapa, estado.zoom_mapa, sz);

    float min_y = std::max(pos.y, pos.y + p_min.y);
    float max_y = std::min(pos.y + sz.y, pos.y + p_max.y);
    float min_x = std::max(pos.x, pos.x + p_min.x);
    float max_x = std::min(pos.x + sz.x, pos.x + p_max.x);

    for (float lon = -180.0f; lon <= 180.0f; lon += paso) {
        ImVec2 v = DatosMundo::latLonAVirtual(0, lon);
        ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
        float sx = pos.x + s.x;
        if (sx >= min_x - 5 && sx <= max_x + 5) {
            bool resaltado = (fmodf(lon, 90.0f) < 0.1f);
            dl->AddLine(ImVec2(sx, min_y), ImVec2(sx, max_y),
                        resaltado ? col_resaltado : col, resaltado ? 1.5f : grosor);
        }
    }

    for (float lat = -90.0f; lat <= 90.0f; lat += paso) {
        ImVec2 v = DatosMundo::latLonAVirtual(lat, 0);
        ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
        float sy = pos.y + s.y;
        if (sy >= min_y - 5 && sy <= max_y + 5) {
            bool resaltado = (fmodf(fabsf(lat), 90.0f) < 0.1f);
            dl->AddLine(ImVec2(min_x, sy), ImVec2(max_x, sy),
                        resaltado ? col_resaltado : col, resaltado ? 1.5f : grosor);
        }
    }
}

static std::vector<ImVec2> generarRutaOrtodromica(float lat1, float lon1,
                                                    float lat2, float lon2,
                                                    int num_puntos = 30) {
    std::vector<ImVec2> puntos;
    puntos.reserve(num_puntos);

    float lat1_r = lat1 * (IM_PI / 180.0f);
    float lon1_r = lon1 * (IM_PI / 180.0f);
    float lat2_r = lat2 * (IM_PI / 180.0f);
    float lon2_r = lon2 * (IM_PI / 180.0f);

    float dlon = lon2_r - lon1_r;
    float cos_lat1 = cosf(lat1_r), sin_lat1 = sinf(lat1_r);
    float cos_lat2 = cosf(lat2_r), sin_lat2 = sinf(lat2_r);

    float d = acosf(sin_lat1 * sin_lat2 + cos_lat1 * cos_lat2 * cosf(dlon));
    if (d < 0.001f) {
        puntos.emplace_back(lon1, lat1);
        puntos.emplace_back(lon2, lat2);
        return puntos;
    }

    float sin_d = sinf(d);
    float inv_sin_d = 1.0f / sin_d;

    for (int i = 0; i < num_puntos; i++) {
        float t = (float)i / (float)(num_puntos - 1);
        float A = sinf((1.0f - t) * d) * inv_sin_d;
        float B = sinf(t * d) * inv_sin_d;

        float x = A * cos_lat1 * cosf(lon1_r) + B * cos_lat2 * cosf(lon2_r);
        float y = A * cos_lat1 * sinf(lon1_r) + B * cos_lat2 * sinf(lon2_r);
        float z = A * sin_lat1 + B * sin_lat2;

        float lat_r = atan2f(z, sqrtf(x * x + y * y));
        float lon_r = atan2f(y, x);

        puntos.emplace_back(lon_r * (180.0f / IM_PI), lat_r * (180.0f / IM_PI));
    }
    return puntos;
}

static void dibujarPolylineSegura(ImDrawList* dl, const std::vector<ImVec2>& puntos_pantalla,
                                   ImU32 color, float grosor, float zoom) {
    ImVector<ImVec2> linea_actual;
    float umbral_salto = (DatosMundo::ANCHO_VIRTUAL * zoom) / 2.0f;
    for (size_t i = 0; i < puntos_pantalla.size(); ++i) {
        linea_actual.push_back(puntos_pantalla[i]);
        if (i < puntos_pantalla.size() - 1) {
            float dif_x = std::abs(puntos_pantalla[i].x - puntos_pantalla[i+1].x);
            if (dif_x > umbral_salto) {
                if (linea_actual.Size > 1) {
                    dl->AddPolyline(linea_actual.Data, linea_actual.Size, color, false, grosor);
                }
                linea_actual.clear();
            }
        }
    }
    if (linea_actual.Size > 1) {
        dl->AddPolyline(linea_actual.Data, linea_actual.Size, color, false, grosor);
    }
}

static void dibujarRutas(ImDrawList* dl, EstadoAeroGrafos& estado,
                          ImVec2 pos, ImVec2 sz) {
    const auto& ciudades = DatosMundo::obtenerCiudades();
    const auto& rutas = DatosMundo::obtenerRutas();

    ImU32 col_normal, col_sel, col_glow;
    if (estado.modo_noche) {
        col_normal = IM_COL32(40, 100, 160, 40);
        col_sel    = IM_COL32(80, 180, 255, 160);
        col_glow   = IM_COL32(80, 180, 255, 20);
    } else {
        col_normal = IM_COL32(60, 140, 210, 65);
        col_sel    = IM_COL32(100, 200, 255, 180);
        col_glow   = IM_COL32(100, 200, 255, 30);
    }

    if (estado.animacion.activa) {
        col_normal = (col_normal & 0x00FFFFFF) | (15 << 24);
    }
    float grosor_base = 1.5f * std::min(1.0f, estado.zoom_mapa);

    int sel = -1;
    if (!estado.animacion.activa && !estado.algoritmo_ejecutado) {
        sel = (estado.ciudad_destino >= 0) ? estado.ciudad_destino : estado.ciudad_origen;
    }

    for (size_t i = 0; i < rutas.size(); i++) {
        const auto& r = rutas[i];
        const Ciudad& c1 = ciudades[r.origen_id];
        const Ciudad& c2 = ciudades[r.destino_id];

        ImVec2 v1 = DatosMundo::latLonAVirtual(c1.latitud, c1.longitud);
        ImVec2 v2 = DatosMundo::latLonAVirtual(c2.latitud, c2.longitud);
        ImVec2 s1 = virtualAPantalla(v1, estado.centro_mapa, estado.zoom_mapa, sz);
        ImVec2 s2 = virtualAPantalla(v2, estado.centro_mapa, estado.zoom_mapa, sz);

        float m = 60.0f;
        bool off1 = (s1.x + pos.x < pos.x - m || s1.x + pos.x > pos.x + sz.x + m ||
                     s1.y + pos.y < pos.y - m || s1.y + pos.y > pos.y + sz.y + m);
        bool off2 = (s2.x + pos.x < pos.x - m || s2.x + pos.x > pos.x + sz.x + m ||
                     s2.y + pos.y < pos.y - m || s2.y + pos.y > pos.y + sz.y + m);
        if (off1 && off2) continue;

        bool resaltada = (sel >= 0 && (r.origen_id == sel || r.destino_id == sel));
        if (!resaltada && !estado.mostrar_todas_rutas) continue;

        auto pts_latlon = generarRutaOrtodromica(c1.latitud, c1.longitud,
                                                  c2.latitud, c2.longitud, 30);
        size_t n_visibles = 0;
        for (const auto& p : pts_latlon) {
            ImVec2 v = DatosMundo::latLonAVirtual(p.y, p.x);
            ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
            if (s.x >= -5000 && s.x <= 5000 && s.y >= -5000 && s.y <= 5000)
                n_visibles++;
        }
        if (n_visibles < 2) continue;

        int n_total = (int)pts_latlon.size();
        std::vector<ImVec2> pts_pantalla;
        pts_pantalla.reserve(n_total);
        for (const auto& p : pts_latlon) {
            ImVec2 v = DatosMundo::latLonAVirtual(p.y, p.x);
            ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
            pts_pantalla.emplace_back(pos.x + s.x, pos.y + s.y);
        }

        if (resaltada) {
            dibujarPolylineSegura(dl, pts_pantalla, col_glow, grosor_base * 5.0f, estado.zoom_mapa);
            dibujarPolylineSegura(dl, pts_pantalla, col_sel, grosor_base * 2.0f, estado.zoom_mapa);
        } else {
            dibujarPolylineSegura(dl, pts_pantalla, col_normal, grosor_base, estado.zoom_mapa);
        }
    }
}

static int dibujarCiudades(ImDrawList* dl, EstadoAeroGrafos& estado,
                            ImVec2 pos, ImVec2 sz) {
    const auto& ciudades = DatosMundo::obtenerCiudades();
    ImVec2 raton = ImGui::GetIO().MousePos;
    float t = estado.tiempo_reloj;

    int hover = -1;

    for (const auto& c : ciudades) {
        ImVec2 v = DatosMundo::latLonAVirtual(c.latitud, c.longitud);
        ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
        float sx = pos.x + s.x, sy = pos.y + s.y;

        if (sx < pos.x - 60 || sx > pos.x + sz.x + 60 ||
            sy < pos.y - 60 || sy > pos.y + sz.y + 60) continue;

        float radio = 4.0f + (float)c.poblacion_millones * 0.5f;
        radio = std::max(2.5f, std::min(radio * std::min(1.0f, estado.zoom_mapa), 18.0f));

        float d = sqrtf((raton.x - sx) * (raton.x - sx) + (raton.y - sy) * (raton.y - sy));
        bool sobre = (d < radio + 12.0f);
        bool selec = (c.id == estado.ciudad_origen || c.id == estado.ciudad_destino);
        if (sobre) hover = c.id;

        float pulso = 0.0f;
        if (selec) pulso = sinf(t * 2.5f) * 0.15f + 0.15f;

        ImU32 fill, glow_c, circle;
        if (estado.modo_noche) {
            if (selec) {
                fill   = IM_COL32(255, 200, 50, 255);
                glow_c = IM_COL32(255, 200, 50, (int)(80 + pulso * 200));
                circle = IM_COL32(255, 220, 100, 60);
            } else if (sobre) {
                fill   = IM_COL32(255, 230, 80, 255);
                glow_c = IM_COL32(255, 230, 80, 80);
                circle = IM_COL32(255, 240, 120, 50);
            } else {
                float brillo = 0.6f + sinf(t * 1.7f + (float)c.id * 3.1f) * 0.2f;
                fill   = IM_COL32((int)(220*brillo), (int)(200*brillo), (int)(80*brillo), 220);
                glow_c = IM_COL32(255, 220, 80, (int)(30 * brillo));
                circle = IM_COL32(255, 255, 200, (int)(30 * brillo));
            }
        } else {
            if (selec) {
                fill   = IM_COL32(0, 230, 180, 255);
                glow_c = IM_COL32(0, 230, 180, (int)(100 + pulso * 200));
                circle = IM_COL32(0, 255, 200, 70);
            } else if (sobre) {
                fill   = IM_COL32(255, 230, 80, 255);
                glow_c = IM_COL32(255, 230, 80, 80);
                circle = IM_COL32(255, 240, 120, 50);
            } else {
                fill   = IM_COL32(190, 210, 240, 230);
                glow_c = IM_COL32(80, 140, 220, 45);
                circle = IM_COL32(255, 255, 255, 40);
            }
        }

        if (estado.restricciones_geopoliticas && std::string(c.pais) == "Rusia") {
            fill   = IM_COL32(255, 50, 50, 200);
            glow_c = IM_COL32(255, 50, 50, 60);
            circle = IM_COL32(255, 100, 100, 80);
            selec = false;
        }
        float radio_glow = radio * (2.8f + (selec ? pulso : 0.0f));

        dl->AddCircleFilled(ImVec2(sx, sy), radio_glow, glow_c, 24);
        dl->AddCircleFilled(ImVec2(sx, sy), radio, fill, 24);
        dl->AddCircle(ImVec2(sx, sy), radio, circle, 24, 1.5f);
        ImU32 centro_col = estado.modo_noche && !selec && !sobre
            ? IM_COL32(255, 255, 220, 220)
            : IM_COL32(255, 255, 255, 200);
        dl->AddCircleFilled(ImVec2(sx, sy), std::max(1.8f, radio * 0.3f),
                            centro_col, 8);

        bool mostrar_label = sobre || selec ||
                             (estado.mostrar_nombres && estado.zoom_mapa > 0.8f);
        if (mostrar_label) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s (%s)", c.nombre, c.codigo_iata);
            ImVec2 tam = ImGui::CalcTextSize(buf);
            float tx = sx - tam.x * 0.5f;
            float ty = sy - radio - tam.y - 6.0f;
            if (tx < pos.x + 4) tx = pos.x + 4;
            if (tx + tam.x + 4 > pos.x + sz.x) tx = pos.x + sz.x - tam.x - 4;

            ImU32 fondo;
            ImU32 tcol;
            if (estado.modo_noche) {
                fondo = selec ? IM_COL32(40, 30, 0, 210) :
                        sobre ? IM_COL32(20, 20, 30, 210) :
                                IM_COL32(0, 0, 0, 180);
                tcol  = selec ? IM_COL32(255, 220, 80, 255) :
                        sobre ? IM_COL32(255, 255, 200, 255) :
                                IM_COL32(220, 200, 180, 230);
            } else {
                fondo = selec ? IM_COL32(0, 40, 30, 200) :
                        sobre ? IM_COL32(40, 30, 0, 200) :
                                IM_COL32(0, 0, 0, 170);
                tcol  = selec ? IM_COL32(0, 255, 200, 255) :
                        sobre ? IM_COL32(255, 255, 200, 255) :
                                IM_COL32(210, 220, 240, 220);
            }
            dl->AddRectFilled(ImVec2(tx - 4, ty - 2),
                              ImVec2(tx + tam.x + 4, ty + tam.y + 2),
                              fondo, 3.0f);
            dl->AddText(ImVec2(tx, ty), tcol, buf);
        }
    }
    return hover;
}

static void dibujarResultadoAlgoritmo(ImDrawList* dl, EstadoAeroGrafos& estado,
                                       ImVec2 pos, ImVec2 sz) {
    if (!estado.algoritmo_ejecutado) return;
    const auto& ciudades = DatosMundo::obtenerCiudades();

    static bool prev_ejecutado = false;
    static float anim_progreso = 0.0f;
    static EstadoAeroGrafos::Algoritmo prev_algo = EstadoAeroGrafos::Algoritmo::RutaMasCorta;
    static std::vector<int> prev_ruta;
    static std::vector<std::pair<int, int>> prev_mst;

    bool reset_anim = false;
    if (!prev_ejecutado && estado.algoritmo_ejecutado) reset_anim = true;
    if (prev_algo != estado.algoritmo_activo) reset_anim = true;
    if (prev_ruta != estado.ruta_resultado) reset_anim = true;
    if (prev_mst != estado.aristas_mst) reset_anim = true;

    if (reset_anim) {
        anim_progreso = 0.0f;
    }

    prev_ejecutado = estado.algoritmo_ejecutado;
    prev_algo = estado.algoritmo_activo;
    prev_ruta = estado.ruta_resultado;
    prev_mst = estado.aristas_mst;

    if (estado.algoritmo_ejecutado) {
        anim_progreso += ImGui::GetIO().DeltaTime * 1.5f; // Velocidad fluida
        if (anim_progreso > 1.0f) anim_progreso = 1.0f;
    }

    auto ciudadAPantalla = [&](int id) -> ImVec2 {
        if (id < 0 || id >= (int)ciudades.size()) return ImVec2(0,0);
        const auto& c = ciudades[id];
        ImVec2 v = DatosMundo::latLonAVirtual(c.latitud, c.longitud);
        ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
        return ImVec2(pos.x + s.x, pos.y + s.y);
    };
    auto dibujarAristaResultado = [&](int id1, int id2, ImU32 color, float grosor, float fraccion = 1.0f) {
        if (id1 < 0 || id2 < 0 || id1 >= (int)ciudades.size() || id2 >= (int)ciudades.size()) return;
        if (fraccion <= 0.01f) return;
        
        const auto& c1 = ciudades[id1];
        const auto& c2 = ciudades[id2];
        auto pts_latlon = generarRutaOrtodromica(c1.latitud, c1.longitud,
                                                  c2.latitud, c2.longitud, 30);
        int n_total = (int)pts_latlon.size();
        int n_dibujar = std::max(2, (int)(n_total * fraccion));
        if (fraccion >= 1.0f) n_dibujar = n_total;

        std::vector<ImVec2> pts_scr;
        pts_scr.reserve(n_dibujar);
        for (int i = 0; i < n_dibujar; i++) {
            const auto& p = pts_latlon[i];
            ImVec2 v = DatosMundo::latLonAVirtual(p.y, p.x);
            ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
            pts_scr.emplace_back(pos.x + s.x, pos.y + s.y);
        }
        dibujarPolylineSegura(dl, pts_scr, color, grosor, estado.zoom_mapa);
    };

    switch (estado.algoritmo_activo) {
        case EstadoAeroGrafos::Algoritmo::RutaMasCorta:
            if (estado.ruta_resultado.size() < 2) break;
            {
                float dist_acumulada = 0.0f;
                float total_edges = (float)(estado.ruta_resultado.size() - 1);
                float progreso_actual = anim_progreso * total_edges;
                
                for (size_t i = 0; i + 1 < estado.ruta_resultado.size(); i++) {
                    float fraccion = 0.0f;
                    if (progreso_actual >= i + 1.0f) fraccion = 1.0f;
                    else if (progreso_actual > i) fraccion = progreso_actual - i;
                    
                    if (fraccion <= 0.0f) break;

                    int origen = estado.ruta_resultado[i];
                    int destino = estado.ruta_resultado[i+1];
                    dibujarAristaResultado(origen, destino, IM_COL32(0, 255, 100, 200), 4.0f, fraccion);
                    dibujarAristaResultado(origen, destino, IM_COL32(0, 255, 100, 50), 10.0f, fraccion);
                    
                    // Dibujar etiqueta km para el nodo actual
                    if (fraccion >= 1.0f && dist_acumulada > 0.0f) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%.0f km", dist_acumulada);
                        ImVec2 p = ciudadAPantalla(origen);
                        ImVec2 tsz = ImGui::CalcTextSize(buf);
                        ImVec2 b_pos(p.x - tsz.x * 0.5f, p.y - 30.0f);
                        dl->AddRectFilled(ImVec2(b_pos.x - 6, b_pos.y - 4), ImVec2(b_pos.x + tsz.x + 6, b_pos.y + tsz.y + 4), IM_COL32(20, 30, 40, 240), 4.0f);
                        dl->AddRect(ImVec2(b_pos.x - 6, b_pos.y - 4), ImVec2(b_pos.x + tsz.x + 6, b_pos.y + tsz.y + 4), IM_COL32(255, 255, 255, 180), 4.0f, 0, 1.5f);
                        dl->AddText(b_pos, IM_COL32(255, 255, 255, 255), buf);
                    }

                    float d = DatosMundo::calcularDistancia(ciudades[origen].latitud, ciudades[origen].longitud, ciudades[destino].latitud, ciudades[destino].longitud);
                    float peso_ruta = (ciudades[destino].longitud > ciudades[origen].longitud) ? (d * 0.90f) : (d * 1.10f);
                    dist_acumulada += peso_ruta;
                }
                
                if (anim_progreso >= 1.0f) {
                    // Dibujar etiqueta km para el destino final
                    int final_dest = estado.ruta_resultado.back();
                    char buf_fin[32];
                    snprintf(buf_fin, sizeof(buf_fin), "%.0f km", dist_acumulada);
                    ImVec2 p_fin = ciudadAPantalla(final_dest);
                    ImVec2 tsz_fin = ImGui::CalcTextSize(buf_fin);
                    ImVec2 b_pos_fin(p_fin.x - tsz_fin.x * 0.5f, p_fin.y - 30.0f);
                    dl->AddRectFilled(ImVec2(b_pos_fin.x - 6, b_pos_fin.y - 4), ImVec2(b_pos_fin.x + tsz_fin.x + 6, b_pos_fin.y + tsz_fin.y + 4), IM_COL32(20, 30, 40, 240), 4.0f);
                    dl->AddRect(ImVec2(b_pos_fin.x - 6, b_pos_fin.y - 4), ImVec2(b_pos_fin.x + tsz_fin.x + 6, b_pos_fin.y + tsz_fin.y + 4), IM_COL32(255, 255, 255, 180), 4.0f, 0, 1.5f);
                    dl->AddText(b_pos_fin, IM_COL32(255, 255, 255, 255), buf_fin);

                    auto p1 = ciudadAPantalla(estado.ruta_resultado.front());
                    auto p2 = ciudadAPantalla(estado.ruta_resultado.back());
                    dl->AddCircleFilled(p1, 10.0f, IM_COL32(0,255,100,120), 16);
                    dl->AddCircleFilled(p2, 10.0f, IM_COL32(255,80,80,120), 16);
                    dl->AddCircle(p1, 10.0f, IM_COL32(0,255,100,255), 16, 2.0f);
                    dl->AddCircle(p2, 10.0f, IM_COL32(255,80,80,255), 16, 2.0f);
                }
            }
            break;
        case EstadoAeroGrafos::Algoritmo::ConectarTodo:
            {
                float total_edges = (float)estado.aristas_mst.size();
                float progreso_actual = anim_progreso * total_edges;
                for (size_t i = 0; i < estado.aristas_mst.size(); i++) {
                    float fraccion = 0.0f;
                    if (progreso_actual >= i + 1.0f) fraccion = 1.0f;
                    else if (progreso_actual > i) fraccion = progreso_actual - i;
                    
                    if (fraccion <= 0.0f) break;

                    const auto& a = estado.aristas_mst[i];
                    dibujarAristaResultado(a.first, a.second,
                        IM_COL32(0, 220, 255, 180), 3.0f, fraccion);
                    dibujarAristaResultado(a.first, a.second,
                        IM_COL32(0, 220, 255, 40), 8.0f, fraccion);
                }
            }
            break;
        case EstadoAeroGrafos::Algoritmo::ExplorarNiveles:
        case EstadoAeroGrafos::Algoritmo::ExplorarTodo:
            if (estado.orden_visita.empty()) break;
            for (size_t i = 0; i < estado.orden_visita.size(); i++) {
                int id = estado.orden_visita[i];
                ImVec2 p = ciudadAPantalla(id);
                if (p.x < pos.x - 20 || p.x > pos.x + sz.x + 20 ||
                    p.y < pos.y - 20 || p.y > pos.y + sz.y + 20) continue;
                float t = (float)i / (float)estado.orden_visita.size();
                ImU32 col = ImLerp(IM_COL32(100,255,100,200), IM_COL32(255,200,50,200), t);
                dl->AddCircleFilled(p, 9.0f, col, 12);
                char num[8];
                snprintf(num, sizeof(num), "%zu", i+1);
                ImVec2 tam = ImGui::CalcTextSize(num);
                dl->AddText(ImVec2(p.x - tam.x*0.5f, p.y - tam.y*0.5f),
                    IM_COL32(0,0,0,220), num);
            }
            break;
        case EstadoAeroGrafos::Algoritmo::ColorearRegiones: {
            if (estado.colores_asignados.empty()) break;
            ImU32 paleta[] = {
                IM_COL32(255, 60, 60, 220),   IM_COL32(60, 220, 60, 220),
                IM_COL32(60, 120, 255, 220),  IM_COL32(255, 220, 40, 220),
                IM_COL32(200, 80, 255, 220),  IM_COL32(40, 230, 230, 220),
                IM_COL32(255, 140, 40, 220),  IM_COL32(255, 80, 160, 220),
                IM_COL32(140, 200, 80, 220),  IM_COL32(100, 100, 100, 220),
            };
            int num_paleta = sizeof(paleta)/sizeof(paleta[0]);
            for (size_t i = 0; i < ciudades.size(); i++) {
                int col_idx = (i < (size_t)estado.colores_asignados.size())
                    ? estado.colores_asignados[i] : -1;
                if (col_idx < 0) continue;
                ImVec2 p = ciudadAPantalla((int)i);
                if (p.x < pos.x - 20 || p.x > pos.x + sz.x + 20) continue;
                ImU32 col = paleta[col_idx % num_paleta];
                dl->AddCircleFilled(p, 8.0f, col, 12);
                dl->AddCircle(p, 8.0f, IM_COL32(255,255,255,120), 12, 2.0f);
            }
            break;
        }
        case EstadoAeroGrafos::Algoritmo::RutaMantenimiento:
            if (estado.ruta_resultado.size() < 2) break;
            {
                float total_edges = (float)(estado.ruta_resultado.size() - 1);
                float progreso_actual = anim_progreso * total_edges;
                for (size_t i = 0; i + 1 < estado.ruta_resultado.size(); i++) {
                    float fraccion = 0.0f;
                    if (progreso_actual >= i + 1.0f) fraccion = 1.0f;
                    else if (progreso_actual > i) fraccion = progreso_actual - i;
                    
                    if (fraccion <= 0.0f) break;
                    
                    dibujarAristaResultado(estado.ruta_resultado[i],
                        estado.ruta_resultado[i+1],
                        IM_COL32(255, 200, 50, 180), 2.0f, fraccion);
                }
            }
            break;
        case EstadoAeroGrafos::Algoritmo::VueltaAlMundo:
            if (estado.ruta_resultado.size() < 2) break;
            {
                float total_edges = (float)(estado.ruta_resultado.size() - 1);
                float progreso_actual = anim_progreso * total_edges;
                for (size_t i = 0; i + 1 < estado.ruta_resultado.size(); i++) {
                    float fraccion = 0.0f;
                    if (progreso_actual >= i + 1.0f) fraccion = 1.0f;
                    else if (progreso_actual > i) fraccion = progreso_actual - i;
                    
                    if (fraccion <= 0.0f) break;

                    dibujarAristaResultado(estado.ruta_resultado[i],
                        estado.ruta_resultado[i+1],
                        IM_COL32(255, 200, 20, 200), 3.5f, fraccion);
                    dibujarAristaResultado(estado.ruta_resultado[i],
                        estado.ruta_resultado[i+1],
                        IM_COL32(255, 200, 20, 40), 9.0f, fraccion);
                }
            }
            break;
        case EstadoAeroGrafos::Algoritmo::AnalizarRed:
            break;
    }
}

static void dibujarAnimacion(ImDrawList* dl, EstadoAeroGrafos& estado,
                              ImVec2 pos, ImVec2 sz) {
    const auto& anim = estado.animacion;
    const auto& ciudades = DatosMundo::obtenerCiudades();
    float t = estado.tiempo_reloj;

    auto ciudadAPantalla = [&](int id) -> ImVec2 {
        if (id < 0 || id >= (int)ciudades.size()) return ImVec2(0,0);
        const auto& c = ciudades[id];
        ImVec2 v = DatosMundo::latLonAVirtual(c.latitud, c.longitud);
        ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
        return ImVec2(pos.x + s.x, pos.y + s.y);
    };

    auto dibujarAristaAnim = [&](int id1, int id2, ImU32 color, float grosor, float alpha, bool es_gradiente = false) {
        if (id1 < 0 || id2 < 0 || id1 >= (int)ciudades.size() || id2 >= (int)ciudades.size()) return;
        const auto& c1 = ciudades[id1];
        const auto& c2 = ciudades[id2];
        auto pts_latlon = generarRutaOrtodromica(c1.latitud, c1.longitud, c2.latitud, c2.longitud, 24);
        std::vector<ImVec2> pts_scr;
        pts_scr.reserve(pts_latlon.size());
        for (const auto& p : pts_latlon) {
            ImVec2 v = DatosMundo::latLonAVirtual(p.y, p.x);
            ImVec2 s = virtualAPantalla(v, estado.centro_mapa, estado.zoom_mapa, sz);
            pts_scr.emplace_back(pos.x + s.x, pos.y + s.y);
        }

        ImU32 col_alpha = (color & 0x00FFFFFF) | ((int)(alpha * 255) << 24);
        if (es_gradiente && pts_scr.size() > 2) {
            for (size_t i = 0; i < pts_scr.size() - 1; i++) {
                float f = (float)i / (pts_scr.size() - 1);
                ImU32 c = (color & 0x00FFFFFF) | ((int)(alpha * 255 * (0.2f + 0.8f * f)) << 24);
                dl->AddLine(pts_scr[i], pts_scr[i+1], c, grosor);
            }
        } else {
            dibujarPolylineSegura(dl, pts_scr, col_alpha, grosor, estado.zoom_mapa);
        }
    };

    if (!estado.animacion.completa) {
        dl->AddRectFilled(pos, ImVec2(pos.x + sz.x, pos.y + sz.y), IM_COL32(0, 0, 0, 160));
    }

    std::pair<int, int> ultima_confirmada = {-1, -1};
    if (anim.paso_actual >= 0 && anim.paso_actual < (int)anim.pasos.size()) {
        for (int i = anim.paso_actual; i >= 0; i--) {
            if (anim.pasos[i].accion == PasoAnimacion::CONFIRMAR && anim.pasos[i].arista_origen >= 0 && anim.pasos[i].arista_destino >= 0) {
                ultima_confirmada = {anim.pasos[i].arista_origen, anim.pasos[i].arista_destino};
                break;
            }
        }
    }

    for (const auto& par : anim.confirmadas) {
        bool es_ultima = (par == ultima_confirmada || (par.first == ultima_confirmada.second && par.second == ultima_confirmada.first));
        if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento) {
            if (es_ultima) {
                dibujarAristaAnim(par.first, par.second, IM_COL32(255, 200, 50, 255), 3.0f, 1.0f);
                dibujarAristaAnim(par.first, par.second, IM_COL32(255, 150, 0, 200), 8.0f, 0.6f);
            } else {
                dibujarAristaAnim(par.first, par.second, IM_COL32(255, 200, 50, 180), 2.0f, 0.7f);
            }
        } else {
            if (es_ultima) {
                float pulso = sinf(t * 12.0f) * 0.2f + 0.8f;
                dibujarAristaAnim(par.first, par.second, IM_COL32(0, 255, 100, 255), 8.0f * pulso, 1.0f);
                dibujarAristaAnim(par.first, par.second, IM_COL32(0, 255, 100, 200), 16.0f * pulso, 0.6f);
            } else {
                dibujarAristaAnim(par.first, par.second, IM_COL32(0, 255, 100, 220), 5.0f, 0.9f);
                dibujarAristaAnim(par.first, par.second, IM_COL32(0, 255, 100, 150), 12.0f, 0.3f);
            }
        }
    }

    if (anim.paso_actual >= 0 && anim.paso_actual < (int)anim.pasos.size()) {
        int max_history = 15;
        int limit = std::max(0, anim.paso_actual - max_history);
        for (int i = anim.paso_actual; i >= limit; i--) {
            const auto& p = anim.pasos[i];
            if (p.accion == PasoAnimacion::DESCARTAR && p.arista_origen >= 0 && p.arista_destino >= 0) {
                float age = (float)(anim.paso_actual - i);
                float fade = 1.0f - (age / (float)max_history);
                if (fade > 0.0f) {
                    ImU32 col = IM_COL32(255, 50, 50, 255);
                    if (p.descripcion.find("Retrocediendo") != std::string::npos || p.descripcion.find("Back-edge") != std::string::npos) {
                        col = IM_COL32(255, 140, 0, 255);
                    }
                    dibujarAristaAnim(p.arista_origen, p.arista_destino, col, 3.0f, fade * 0.8f);
                }
            }
        }
    }

    if (anim.paso_actual >= 0 && anim.paso_actual < (int)anim.pasos.size()) {
        const auto& paso = anim.pasos[anim.paso_actual];
        if (paso.accion == PasoAnimacion::EXPLORAR && paso.arista_origen >= 0 && paso.arista_destino >= 0) {
            float pulsar = sinf(t * 15.0f) * 0.3f + 0.7f;
            dibujarAristaAnim(paso.arista_origen, paso.arista_destino, IM_COL32(50, 200, 255, 255), 6.0f, pulsar, true);
        }
    }

    std::map<int, float> distancias;
    std::map<int, int> niveles;
    int nivel_actual_max = -1;

    for (int i = 0; i <= anim.paso_actual && i < (int)anim.pasos.size(); i++) {
        const auto& p = anim.pasos[i];
        if (p.nodo_id >= 0) {
            if (p.distancia_acumulada >= 0.0f) distancias[p.nodo_id] = p.distancia_acumulada;
            if (p.nivel_profundidad >= 0) {
                niveles[p.nodo_id] = p.nivel_profundidad;
                if (p.nivel_profundidad > nivel_actual_max) nivel_actual_max = p.nivel_profundidad;
            }
        }
    }

    for (int id : anim.visitados) {
        ImVec2 p = ciudadAPantalla(id);
        if (p.x < pos.x - 30 || p.x > pos.x + sz.x + 30) continue;

        if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento ||
            estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::VueltaAlMundo) {
            continue;
        }

        bool procesando = anim.procesando.count(id) > 0;

        ImU32 nodo_color = procesando ? IM_COL32(255, 220, 50, 255) : IM_COL32(0, 255, 120, 220);

        if (procesando) {
            float pulso = sinf(t * 8.0f) * 0.4f + 0.8f;
            dl->AddCircleFilled(p, 10.0f * pulso, nodo_color, 16);
            dl->AddCircle(p, 18.0f * pulso, nodo_color, 16, 3.0f);
        } else {
            dl->AddCircleFilled(p, 6.0f, nodo_color, 16);
            dl->AddCircle(p, 10.0f, nodo_color, 16, 2.0f);
        }

        if (procesando && estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::ExplorarNiveles) {
            float pulso = fmodf(t * 1.5f, 1.0f);
            dl->AddCircle(p, 6.0f + 40.0f * pulso, IM_COL32(50, 200, 255, (int)(255 * (1.0f - pulso))), 24, 3.0f);
        }

        if (distancias.count(id)) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f km", distancias[id]);
            ImVec2 tsz = ImGui::CalcTextSize(buf);
            ImVec2 b_pos(p.x - tsz.x * 0.5f, p.y - 30.0f);
            dl->AddRectFilled(ImVec2(b_pos.x - 6, b_pos.y - 4), ImVec2(b_pos.x + tsz.x + 6, b_pos.y + tsz.y + 4), IM_COL32(20, 30, 40, 240), 4.0f);
            dl->AddRect(ImVec2(b_pos.x - 6, b_pos.y - 4), ImVec2(b_pos.x + tsz.x + 6, b_pos.y + tsz.y + 4), IM_COL32(255, 255, 255, 180), 4.0f, 0, 1.5f);
            dl->AddText(b_pos, IM_COL32(255, 255, 255, 255), buf);
        }
    }

    ImU32 paleta[] = {
        IM_COL32(255, 60, 60, 220),   IM_COL32(60, 220, 60, 220),
        IM_COL32(60, 120, 255, 220),  IM_COL32(255, 220, 40, 220),
        IM_COL32(200, 80, 255, 220),  IM_COL32(40, 230, 230, 220),
        IM_COL32(255, 140, 40, 220),  IM_COL32(255, 80, 160, 220),
        IM_COL32(140, 200, 80, 220),  IM_COL32(100, 100, 100, 220),
    };
    int num_paleta = sizeof(paleta)/sizeof(paleta[0]);

    if (anim.paso_actual >= 0) {
        for (int i = 0; i <= anim.paso_actual && i < (int)anim.pasos.size(); i++) {
            const auto& paso = anim.pasos[i];
            if (paso.accion == PasoAnimacion::COLOREAR && paso.nodo_id >= 0 && paso.color_asignado >= 0) {
                ImVec2 p = ciudadAPantalla(paso.nodo_id);
                if (p.x >= pos.x - 20 && p.x <= pos.x + sz.x + 20) {
                    ImU32 col = paleta[paso.color_asignado % num_paleta];
                    float r = 10.0f;
                    if (i == anim.paso_actual) {
                        float pulso = sinf(t * 8.0f) * 0.4f + 1.0f;
                        r *= pulso;
                        dl->AddCircleFilled(p, r * 1.5f, (col & 0x00FFFFFF) | (100 << 24), 16);
                    }
                    dl->AddCircleFilled(p, r, col, 12);
                    dl->AddCircle(p, r, IM_COL32(255,255,255,150), 12, 2.0f);
                }
            }
        }
    }

    if (anim.paso_actual >= 0 && anim.paso_actual < (int)anim.pasos.size()) {
        const auto& paso = anim.pasos[anim.paso_actual];
        if (!paso.descripcion.empty()) {
            ImVec2 tam = ImGui::CalcTextSize(paso.descripcion.c_str());
            float tx = pos.x + (sz.x - tam.x) * 0.5f;
            float ty = pos.y + 40.0f;

            dl->AddRectFilled(ImVec2(tx - 25, ty - 15),
                              ImVec2(tx + tam.x + 25, ty + tam.y + 15),
                              IM_COL32(10, 15, 25, 240), 10.0f);
            dl->AddRect(ImVec2(tx - 25, ty - 15),
                        ImVec2(tx + tam.x + 25, ty + tam.y + 15),
                        IM_COL32(0, 255, 150, 220), 10.0f, 0, 3.0f);

            dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), paso.descripcion.c_str());
        }
    }

    if (anim.completa &&
       (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento ||
        estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::VueltaAlMundo)) {

        std::vector<int> grados(63, 0);
        for (const auto& r : DatosMundo::obtenerRutas()) {
            grados[r.origen_id]++;
            grados[r.destino_id]++;
        }

        for (int id = 0; id < 63; id++) {
            ImVec2 p = ciudadAPantalla(id);
            if (p.x < pos.x - 30 || p.x > pos.x + sz.x + 30) continue;

            float pulso = sinf(t * 10.0f) * 0.5f + 0.5f;

            if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::VueltaAlMundo && grados[id] <= 1) {
                dl->AddCircleFilled(p, 15.0f + 10.0f * pulso, IM_COL32(255, 0, 0, 150), 16);
                dl->AddCircle(p, 25.0f + 15.0f * pulso, IM_COL32(255, 50, 50, 255), 16, 4.0f);
            }
            else if (estado.algoritmo_activo == EstadoAeroGrafos::Algoritmo::RutaMantenimiento && (grados[id] % 2 != 0)) {
                dl->AddCircleFilled(p, 12.0f + 8.0f * pulso, IM_COL32(255, 100, 0, 150), 16);
                dl->AddCircle(p, 20.0f + 10.0f * pulso, IM_COL32(255, 150, 0, 255), 16, 3.0f);
            }
        }
    }
}

static void dibujarMensajes(ImDrawList* dl, EstadoAeroGrafos& estado,
                             ImVec2 pos, ImVec2 sz) {
    int inicio = std::max(0, (int)estado.mensajes.size() - 5);
    float y = 45.0f;
    for (int i = inicio; i < (int)estado.mensajes.size(); i++) {
        const auto& msg = estado.mensajes[i];
        float a = std::min(1.0f, (float)msg.tiempo_restante);
        if (a < 0.05f) continue;
        ImU32 col = (msg.color & 0x00FFFFFF) | ((int)(a * 255) << 24);
        ImVec2 tam = ImGui::CalcTextSize(msg.texto.c_str());
        float tx = pos.x + 14.0f;
        float ty = pos.y + sz.y - 14.0f - y - tam.y;
        dl->AddRectFilled(ImVec2(tx - 10, ty - 5),
                          ImVec2(tx + tam.x + 10, ty + tam.y + 5),
                          IM_COL32(0, 0, 0, (int)(185 * a)), 6.0f);
        dl->AddText(ImVec2(tx, ty), col, msg.texto.c_str());
        y += tam.y + 6.0f;
    }
}

static void dibujarTooltipCiudad(int hover) {
    if (hover < 0) return;
    const auto& ciudades = DatosMundo::obtenerCiudades();
    if (hover >= (int)ciudades.size()) return;
    const Ciudad& c = ciudades[hover];

    const auto& rutas = DatosMundo::obtenerRutas();
    int n_conexiones = 0;
    float dist_prom = 0;
    int dest_cercano = 99999, dest_lejano = 0;
    for (const auto& r : rutas) {
        if (r.origen_id == hover || r.destino_id == hover) {
            n_conexiones++;
            dist_prom += r.distancia_km;
            if (r.distancia_km < dest_cercano) dest_cercano = (int)r.distancia_km;
            if (r.distancia_km > dest_lejano) dest_lejano = (int)r.distancia_km;
        }
    }
    if (n_conexiones > 0) dist_prom /= n_conexiones;

    ImGui::BeginTooltip();
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 0.65f, 1.0f), "%s", c.nombre);
    ImGui::SameLine();
    ImGui::TextDisabled("  (%s)", c.codigo_iata);

    ImGui::Separator();

    ImGui::TextUnformatted(ICON_FA_FLAG);
    ImGui::SameLine();
    ImGui::TextUnformatted(c.pais);

    ImGui::TextUnformatted(ICON_FA_USERS);
    ImGui::SameLine();
    ImGui::Text("%d M habitantes", c.poblacion_millones);

    ImGui::TextUnformatted(ICON_FA_LOCATION_DOT);
    ImGui::SameLine();
    char dir_lat = (c.latitud >= 0) ? 'N' : 'S';
    char dir_lon = (c.longitud >= 0) ? 'E' : 'O';
    ImGui::Text("%.2f° %c, %.2f° %c", fabsf(c.latitud), dir_lat, fabsf(c.longitud), dir_lon);

    ImGui::Separator();

    ImGui::TextUnformatted(ICON_FA_ROUTE);
    ImGui::SameLine();
    ImGui::Text("%d rutas  |  Prom: %.0f km  |  Min: %d km  |  Max: %d km",
        n_conexiones, dist_prom, dest_cercano, dest_lejano);

    ImGui::EndTooltip();
}

static void clampaCentro(EstadoAeroGrafos& estado, ImVec2 tam) {
    float min_zoom_x = tam.x / DatosMundo::ANCHO_VIRTUAL;
    float min_zoom_y = tam.y / DatosMundo::ALTO_VIRTUAL;
    float min_zoom = std::max(min_zoom_x, min_zoom_y);

    estado.zoom_mapa = std::max(min_zoom, std::min(estado.zoom_mapa, 15.0f));

    float min_cx = tam.x / (2.0f * estado.zoom_mapa);
    float max_cx = DatosMundo::ANCHO_VIRTUAL - min_cx;
    float min_cy = tam.y / (2.0f * estado.zoom_mapa);
    float max_cy = DatosMundo::ALTO_VIRTUAL - min_cy;

    if (max_cx < min_cx) { estado.centro_mapa.x = DatosMundo::ANCHO_VIRTUAL / 2.0f; }
    else { estado.centro_mapa.x = std::max(min_cx, std::min(max_cx, estado.centro_mapa.x)); }

    if (max_cy < min_cy) { estado.centro_mapa.y = DatosMundo::ALTO_VIRTUAL / 2.0f; }
    else { estado.centro_mapa.y = std::max(min_cy, std::min(max_cy, estado.centro_mapa.y)); }
}

static void manejarInteraccion(EstadoAeroGrafos& estado, bool lienzo_hovered,
                                ImVec2 pos, ImVec2 tam, int hover_ciudad) {
    static bool paneando = false;
    static int  ciudad_pres = -1;
    static bool click_candidato = false;
    static bool se_movio = false;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && lienzo_hovered) {
        ciudad_pres = hover_ciudad;
        click_candidato = true;
        se_movio = false;
        paneando = false;
    }

    if (lienzo_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !se_movio) {
        float dx = ImGui::GetIO().MouseDelta.x;
        float dy = ImGui::GetIO().MouseDelta.y;
        if (fabsf(dx) > 3.0f || fabsf(dy) > 3.0f) {
            se_movio = true;
            click_candidato = false;
            paneando = (ciudad_pres < 0);
        }
    }

    if (lienzo_hovered && paneando && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        estado.interpolando_camara = false;
        estado.centro_mapa.x -= ImGui::GetIO().MouseDelta.x / estado.zoom_mapa;
        estado.centro_mapa.y -= ImGui::GetIO().MouseDelta.y / estado.zoom_mapa;
        clampaCentro(estado, tam);
        estado.target_centro = estado.centro_mapa;
    }

    if (lienzo_hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 2.0f)) {
        estado.interpolando_camara = false;
        estado.centro_mapa.x -= ImGui::GetIO().MouseDelta.x / estado.zoom_mapa;
        estado.centro_mapa.y -= ImGui::GetIO().MouseDelta.y / estado.zoom_mapa;
        clampaCentro(estado, tam);
        estado.target_centro = estado.centro_mapa;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (click_candidato && ciudad_pres >= 0 && !se_movio) {
            const auto& ciudades = DatosMundo::obtenerCiudades();
            if (estado.restricciones_geopoliticas && std::string(ciudades[ciudad_pres].pais) == "Rusia") {
                estado.agregarMensaje(ICON_FA_BAN " Espacio aereo ruso bloqueado (Moscu/Rusia no seleccionable)", IM_COL32(255, 80, 80, 255));
                g_sonidos.reproducir(Sonidos::DESCARTAR);
            } else {
            if (estado.ciudad_origen < 0 || estado.ciudad_destino >= 0) {
                estado.ciudad_origen = ciudad_pres;
                estado.ciudad_destino = -1;
                estado.agregarMensaje(ICON_FA_PLANE_DEPARTURE " Origen: " +
                    std::string(ciudades[ciudad_pres].nombre),
                    IM_COL32(100, 255, 200, 255));
            } else if (ciudad_pres != estado.ciudad_origen) {
                estado.ciudad_destino = ciudad_pres;
                estado.agregarMensaje(ICON_FA_PLANE_ARRIVAL " Destino: " +
                    std::string(ciudades[ciudad_pres].nombre),
                    IM_COL32(100, 200, 255, 255));
            }
            }
        }
        paneando = false;
        click_candidato = false;
        ciudad_pres = -1;
        se_movio = false;
    }

    if (lienzo_hovered) {
        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f) {
            estado.interpolando_camara = false;
            ImVec2 raton = ImGui::GetMousePos();
            float rx = raton.x - pos.x, ry = raton.y - pos.y;
            float wx = estado.centro_mapa.x + (rx - tam.x * 0.5f) / estado.zoom_mapa;
            float wy = estado.centro_mapa.y + (ry - tam.y * 0.5f) / estado.zoom_mapa;
            float f = powf(1.15f, scroll);
            estado.zoom_mapa = std::max(0.2f, std::min(estado.zoom_mapa * f, 15.0f));
            estado.target_zoom = estado.zoom_mapa;
            estado.centro_mapa.x = wx - (rx - tam.x * 0.5f) / estado.zoom_mapa;
            estado.centro_mapa.y = wy - (ry - tam.y * 0.5f) / estado.zoom_mapa;
            clampaCentro(estado, tam);
            estado.target_centro = estado.centro_mapa;
        }
        
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
            estado.interpolando_camara = false;
            estado.zoom_mapa = std::min(estado.zoom_mapa * 1.4f, 15.0f);
            estado.target_zoom = estado.zoom_mapa;
            clampaCentro(estado, tam);
            estado.target_centro = estado.centro_mapa;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
            estado.interpolando_camara = false;
            estado.zoom_mapa = std::max(0.2f, std::min(estado.zoom_mapa / 1.4f, 15.0f));
            estado.target_zoom = estado.zoom_mapa;
            clampaCentro(estado, tam);
            estado.target_centro = estado.centro_mapa;
        }
    }
}

static void dibujarControlesZoom(EstadoAeroGrafos& estado) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float x = pos.x + ImGui::GetContentRegionAvail().x - 50.0f;
    float y = pos.y + 8.0f;

    ImGui::SetCursorScreenPos(ImVec2(x, y));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.18f, 0.30f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.20f, 0.28f, 0.45f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.30f, 0.40f, 0.60f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button("+", ImVec2(36, 30))) {
        estado.interpolando_camara = false;
        estado.zoom_mapa = std::min(estado.zoom_mapa * 1.4f, 15.0f);
        estado.target_zoom = estado.zoom_mapa;
        clampaCentro(estado, ImGui::GetContentRegionAvail());
        estado.target_centro = estado.centro_mapa;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SetCursorScreenPos(ImVec2(x, y + 34));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.18f, 0.30f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.20f, 0.28f, 0.45f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.30f, 0.40f, 0.60f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button("-", ImVec2(36, 30))) {
        estado.interpolando_camara = false;
        estado.zoom_mapa = std::max(estado.zoom_mapa / 1.4f, 0.2f);
        estado.target_zoom = estado.zoom_mapa;
        clampaCentro(estado, ImGui::GetContentRegionAvail());
        estado.target_centro = estado.centro_mapa;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SetCursorScreenPos(ImVec2(x, y + 68));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.18f, 0.30f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.20f, 0.28f, 0.45f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.30f, 0.40f, 0.60f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button(ICON_FA_HOUSE, ImVec2(36, 30))) {
        estado.interpolando_camara = false;
        estado.centro_mapa = ImVec2(DatosMundo::ANCHO_VIRTUAL / 2.0f,
                                    DatosMundo::ALTO_VIRTUAL / 2.0f);
        estado.zoom_mapa = 1.0f;
        estado.target_centro = estado.centro_mapa;
        estado.target_zoom = estado.zoom_mapa;
        estado.ciudad_origen = -1;
        estado.ciudad_destino = -1;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Resetear vista");
}

static void dibujarInfoZoom(ImDrawList* dl, EstadoAeroGrafos& estado,
                             ImVec2 pos, ImVec2 sz) {
    char buf[96];
    const char* modo = estado.modo_noche ? "Noche" : "Dia";
    snprintf(buf, sizeof(buf), ICON_FA_MAP " %.1fx  |  %s %s", estado.zoom_mapa,
        estado.modo_noche ? ICON_FA_MOON : ICON_FA_SUN, modo);
    ImVec2 tam = ImGui::CalcTextSize(buf);
    float x = pos.x + 12.0f, y = pos.y + sz.y - tam.y - 12.0f;
    ImU32 bg = estado.modo_noche ? IM_COL32(10, 10, 20, 180) : IM_COL32(0, 0, 0, 140);
    ImU32 fg = estado.modo_noche ? IM_COL32(200, 180, 140, 220) : IM_COL32(180, 200, 220, 220);
    dl->AddRectFilled(ImVec2(x - 6, y - 4), ImVec2(x + tam.x + 6, y + tam.y + 4),
                      bg, 4.0f);
    dl->AddText(ImVec2(x, y), fg, buf);
}

//Funcion principal de dibujado

namespace LienzoAeroGrafos {

void dibujar(Grafo& red, Interfaz& self) {
    auto& estado = self.estado_aerografos;
    float dt = ImGui::GetIO().DeltaTime;
    estado.tiempo_reloj += dt;

    // Camara Cinematica (Lerp) movida mas abajo para tener canvas_size

    // Avanzar animacion automaticamente y reproducir sonidos
    if (estado.animacion.activa) {
        int paso_ant = estado.animacion.paso_actual;
        bool completa_ant = estado.animacion.completa;
        Animacion::avanzarAuto(estado.animacion, dt);

        if (estado.animacion.paso_actual != paso_ant && estado.animacion.paso_actual >= 0) {
            auto accion = estado.animacion.pasos[estado.animacion.paso_actual].accion;
            if (accion == PasoAnimacion::VISITAR) g_sonidos.reproducir(Sonidos::VISITAR_NODO);
            else if (accion == PasoAnimacion::CONFIRMAR) g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
            else if (accion == PasoAnimacion::DESCARTAR) g_sonidos.reproducir(Sonidos::DESCARTAR);
            else if (accion == PasoAnimacion::EXPLORAR) g_sonidos.reproducir(Sonidos::VISITAR_NODO);
            else if (accion == PasoAnimacion::COLOREAR) g_sonidos.reproducir(Sonidos::CONFIRMAR_RUTA);
        }
        if (!completa_ant && estado.animacion.completa) {
            g_sonidos.reproducir(Sonidos::TRIUNFO_DIJKSTRA);
        }
    }

    cargarTexturaMundo(estado);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Mapa AeroGrafos", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();

    if (canvas_size.x <= 0 || canvas_size.y <= 0) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    // Camara Cinematica (Lerp) - movida aqui para tener acceso a canvas_size
    if (estado.interpolando_camara) {
        float min_zoom_x = canvas_size.x / DatosMundo::ANCHO_VIRTUAL;
        float min_zoom_y = canvas_size.y / DatosMundo::ALTO_VIRTUAL;
        float min_zoom = std::max(min_zoom_x, min_zoom_y);
        estado.target_zoom = std::max(estado.target_zoom, min_zoom);
        
        float min_cx = canvas_size.x / (2.0f * estado.target_zoom);
        float max_cx = DatosMundo::ANCHO_VIRTUAL - min_cx;
        float min_cy = canvas_size.y / (2.0f * estado.target_zoom);
        float max_cy = DatosMundo::ALTO_VIRTUAL - min_cy;
        
        if (max_cx < min_cx) { estado.target_centro.x = DatosMundo::ANCHO_VIRTUAL / 2.0f; }
        else { estado.target_centro.x = std::max(min_cx, std::min(max_cx, estado.target_centro.x)); }

        if (max_cy < min_cy) { estado.target_centro.y = DatosMundo::ALTO_VIRTUAL / 2.0f; }
        else { estado.target_centro.y = std::max(min_cy, std::min(max_cy, estado.target_centro.y)); }

        float t = std::min(dt * 5.0f, 1.0f);
        estado.centro_mapa.x = estado.centro_mapa.x + (estado.target_centro.x - estado.centro_mapa.x) * t;
        estado.centro_mapa.y = estado.centro_mapa.y + (estado.target_centro.y - estado.centro_mapa.y) * t;
        estado.zoom_mapa = estado.zoom_mapa + (estado.target_zoom - estado.zoom_mapa) * t;
        
        clampaCentro(estado, canvas_size);

        if (fabsf(estado.centro_mapa.x - estado.target_centro.x) < 0.5f &&
            fabsf(estado.centro_mapa.y - estado.target_centro.y) < 0.5f &&
            fabsf(estado.zoom_mapa - estado.target_zoom) < 0.01f) {
            estado.centro_mapa = estado.target_centro;
            estado.zoom_mapa = estado.target_zoom;
            estado.interpolando_camara = false;
        }
    }

    dl->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

    // 1. Oceano
    dibujarOcean(dl, canvas_pos, canvas_size, estado.modo_noche);

    // 2. Textura del mundo
    dibujarTexturaMundo(dl, estado, canvas_pos, canvas_size);

    // 3. Grid
    dibujarGrid(dl, estado, canvas_pos, canvas_size);

    // 4. Rutas
    dibujarRutas(dl, estado, canvas_pos, canvas_size);

    // 5. Ciudades + hover
    int hover_ciudad = dibujarCiudades(dl, estado, canvas_pos, canvas_size);

    // 6. Resultados de algoritmos (solo si no hay animacion activa)
    if (!estado.animacion.activa) {
        dibujarResultadoAlgoritmo(dl, estado, canvas_pos, canvas_size);
    }

    // 7. Animacion paso a paso
    if (estado.animacion.activa)
        dibujarAnimacion(dl, estado, canvas_pos, canvas_size);

    // 8. Mensajes
    dibujarMensajes(dl, estado, canvas_pos, canvas_size);

    // 9. Indicador de zoom
    dibujarInfoZoom(dl, estado, canvas_pos, canvas_size);

    dl->PopClipRect();

    // Controles de zoom (como widgets ImGui, encima del canvas)
    dibujarControlesZoom(estado);

    // Area clickeable
    ImGui::SetCursorScreenPos(canvas_pos);
    ImGui::InvisibleButton("lienzo_aerografos", canvas_size,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);

    bool hovered = ImGui::IsItemHovered();

    // Interaccion
    manejarInteraccion(estado, hovered, canvas_pos, canvas_size, hover_ciudad);

    // Tooltip
    dibujarTooltipCiudad(hover_ciudad);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace LienzoAeroGrafos
