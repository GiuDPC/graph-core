#include "LienzoRed.hpp"
#include "Interfaz.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "interfaz/util/Easing.hpp"
#include "interfaz/util/Animacion.hpp"
#include "interfaz/util/AnimacionUI.hpp"

#include "IconsFontAwesome6.h"
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cfloat>
#include <climits>

// Static helper functions

static void dibujarGrid(ImDrawList* dl, const EstadoUI& ui,
                         ImVec2 origin, ImVec2 tamano)
{
    float zoom_estimado = 1.0f; // grid base, no tenemos acceso directo a nodos
    float grid = std::max(10.0f, 25.0f * zoom_estimado);
    float off_x = fmodf(ui.offset_lienzo.x, grid);
    float off_y = fmodf(ui.offset_lienzo.y, grid);
    if (off_x < 0) off_x += grid;
    if (off_y < 0) off_y += grid;

    int alpha_grid = std::min(30, std::max(2, (int)(12 * zoom_estimado)));
    ImU32 color_grid = IM_COL32(255, 255, 255, alpha_grid);

    for (float x = off_x; x < tamano.x; x += grid)
        dl->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + tamano.y), color_grid);
    for (float y = off_y; y < tamano.y; y += grid)
        dl->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + tamano.x, origin.y + y), color_grid);
}

// Icono hardware
static const char* iconoHardware(TipoHardware tipo) {
    switch (tipo) {
        case TipoHardware::Servidor: return ICON_FA_SERVER;
        case TipoHardware::Router:   return ICON_FA_NETWORK_WIRED;
        case TipoHardware::Switch:   return ICON_FA_RIGHT_LEFT;
        case TipoHardware::Firewall: return ICON_FA_SHIELD_HALVED;
        case TipoHardware::Terminal: return ICON_FA_DESKTOP;
        default: return ICON_FA_CIRCLE;
    }
}

// Color de saturacion: verde→amarillo→rojo
static ImU32 colorSaturacion(float uso) {
    if (uso < 0.5f) {
        float t = uso * 2.0f;
        return IM_COL32((int)(t * 255), 200, (int)((1.0f - t) * 100), 220);
    } else {
        float t = (uso - 0.5f) * 2.0f;
        return IM_COL32(255, (int)((1.0f - t) * 200), (int)((1.0f - t) * 50), 220);
    }
}

// Color health
static ImU32 colorHealth(float v) {
    if (v < 0.5f) {
        float t = v * 2.0f;
        return IM_COL32((int)(t * 255), 200, (int)((1.0f - t) * 80), 200);
    } else {
        float t = (v - 0.5f) * 2.0f;
        return IM_COL32(255, (int)((1.0f - t) * 200), (int)((1.0f - t) * 50), 200);
    }
}

// Helpers para aristas dirigidas y curvas

static void calcularPuntaFlecha(ImVec2 origen, ImVec2 destino, float radio_nodo,
                                 ImVec2& p1, ImVec2& p2, ImVec2& p3) {
    float dx = destino.x - origen.x;
    float dy = destino.y - origen.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.001f) { p1 = p2 = p3 = destino; return; }
    float nx = dx / dist;
    float ny = dy / dist;
    float bx = destino.x - nx * radio_nodo;
    float by = destino.y - ny * radio_nodo;
    float tam_flecha = 10.0f;
    float perp_x = -ny;
    float perp_y = nx;
    p1 = ImVec2(bx, by);
    p2 = ImVec2(bx - nx * tam_flecha + perp_x * tam_flecha * 0.4f,
                by - ny * tam_flecha + perp_y * tam_flecha * 0.4f);
    p3 = ImVec2(bx - nx * tam_flecha - perp_x * tam_flecha * 0.4f,
                by - ny * tam_flecha - perp_y * tam_flecha * 0.4f);
}

static ImVec2 puntoBezierCuadratico(ImVec2 p0, ImVec2 p1, ImVec2 p2, float t) {
    float u = 1.0f - t;
    return ImVec2(
        u*u*p0.x + 2*u*t*p1.x + t*t*p2.x,
        u*u*p0.y + 2*u*t*p1.y + t*t*p2.y
    );
}

static ImVec2 calcularPuntoControlCurva(ImVec2 o, ImVec2 d, float desplazo = 30.0f) {
    float dx = d.x - o.x;
    float dy = d.y - o.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 0.001f) return ImVec2(o.x + 30, o.y - 30);
    float perp_x = -dy / dist;
    float perp_y =  dx / dist;
    ImVec2 mid((o.x + d.x) * 0.5f, (o.y + d.y) * 0.5f);
    return ImVec2(mid.x + perp_x * desplazo, mid.y + perp_y * desplazo);
}

static ImVec2 puntoEnArista(ImVec2 o, ImVec2 d, ImVec2 pc, bool es_curva, float t) {
    if (es_curva) {
        return puntoBezierCuadratico(o, pc, d, t);
    }
    return ImVec2(o.x + (d.x - o.x) * t, o.y + (d.y - o.y) * t);
}

static void lineaArista(ImDrawList* dl, ImVec2 o, ImVec2 d, float r_o, float r_d,
                         ImVec2 pc, bool es_curva, ImU32 col, float grosor) {
    float dx = d.x - o.x;
    float dy = d.y - o.y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist <= (r_o + r_d + 1.0f)) return;

    if (!es_curva) {
        dl->AddLine(o, d, col, grosor);
    } else {
        dl->PathLineTo(o);
        dl->PathBezierQuadraticCurveTo(pc, d, 20);
        dl->PathStroke(col, 0, grosor);
    }
}

// Dibujar aristas de un grafo
static void dibujarAristasEnGrafo(ImDrawList* dl, const Grafo& g_dib,
    EstadoUI& ui, EstadoGrafos& grafos,
    const std::unordered_set<uint64_t>& edge_lookup,
    bool usar_ranking, float rank_size_min, float rank_size_max,
    float rank_color_min, float rank_color_max,
    bool es_g2, bool editando_g2,
    ImVec2 origin, ImVec2 tamano, float tiempo, ImVec2 mouse,
    bool& tooltip_mostrado,
    const std::unordered_set<int>* vecinos_hover)
{
    (void)rank_size_min; (void)rank_size_max;
    (void)rank_color_min; (void)rank_color_max;
    (void)usar_ranking;

    auto getValorRank = [&](int nid, EstadoUI::EstadoRanking::Atributo attr) -> float {
        switch (attr) {
            case EstadoUI::EstadoRanking::GRADO: return (float)g_dib.gradoNodo(nid);
            default: return 0;
        }
    };

    auto get_radio_dibujo = [&](const Nodo* nd) -> float {
        if (!nd) return 0.0f;
        float r = nd->radio;
        if (usar_ranking) {
            auto& rk = ui.ranking;
            if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                float range = std::max(0.001f, rank_size_max - rank_size_min);
                float val = getValorRank(nd->id, rk.atributo_size);
                float t = (val - rank_size_min) / range;
                if (rk.invertir_size) t = 1.0f - t;
                t = std::max(0.0f, std::min(1.0f, t));
                r = rk.min_size + t * (rk.max_size - rk.min_size);
            }
        }
        if ((!es_g2 && !editando_g2 && nd->id == ui.nodo_hover) ||
            (es_g2 && editando_g2 && nd->id == ui.nodo_hover)) {
            r += 2.0f + sinf(tiempo * 8.0f) * 1.5f;
        }
        return r;
    };

    for (size_t idx = 0; idx < g_dib.aristas.size(); idx++) {
        const auto& a = g_dib.aristas[idx];
        const Nodo* o = g_dib.obtenerNodo(a.origen_id);
        const Nodo* d = g_dib.obtenerNodo(a.destino_id);

        // Frustum Culling
        if (std::max(o->posicion.x, d->posicion.x) < origin.x - 100.0f ||
            std::min(o->posicion.x, d->posicion.x) > origin.x + tamano.x + 100.0f ||
            std::max(o->posicion.y, d->posicion.y) < origin.y - 100.0f ||
            std::min(o->posicion.y, d->posicion.y) > origin.y + tamano.y + 100.0f) {
            continue;
        }
        if (!o || !d) continue;

        // Self-loop: lazo Bezier simetrico arriba del nodo
        if (o == d) {
            float r = o->radio;
            float h = r * 2.8f;
            ImVec2 p0(o->posicion.x - r * 0.7f, o->posicion.y - r * 0.7f);
            ImVec2 pc1(o->posicion.x - r * 1.8f, o->posicion.y - h);
            ImVec2 pc2(o->posicion.x + r * 1.8f, o->posicion.y - h);
            ImVec2 p3(o->posicion.x + r * 0.7f, o->posicion.y - r * 0.7f);

            ImU32 col_self = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 120);
            float grosor_self = 2.0f + std::min(a.peso_actual / 10.0f, 4.0f);

            dl->AddBezierCubic(p0, pc1, pc2, p3, col_self, grosor_self);

            if (a.es_dirigida) {
                float t1 = 0.95f, t2 = 0.99f;
                float u1 = 1-t1, u2 = 1-t2;
                ImVec2 prev(
                    u1*u1*u1*p0.x + 3*u1*u1*t1*pc1.x + 3*u1*t1*t1*pc2.x + t1*t1*t1*p3.x,
                    u1*u1*u1*p0.y + 3*u1*u1*t1*pc1.y + 3*u1*t1*t1*pc2.y + t1*t1*t1*p3.y);
                ImVec2 tip(
                    u2*u2*u2*p0.x + 3*u2*u2*t2*pc1.x + 3*u2*t2*t2*pc2.x + t2*t2*t2*p3.x,
                    u2*u2*u2*p0.y + 3*u2*u2*t2*pc1.y + 3*u2*t2*t2*pc2.y + t2*t2*t2*p3.y);
                ImVec2 fp1, fp2, fp3;
                calcularPuntaFlecha(prev, tip, 2.0f, fp1, fp2, fp3);
                dl->AddTriangleFilled(fp1, fp2, fp3, col_self);
                dl->AddTriangle(fp1, fp2, fp3, IM_COL32(255, 255, 255, 80), 1.0f);
            }

            float alpha_lod = std::clamp((ui.zoom_lienzo - 0.25f) / 0.25f, 0.0f, 1.0f);
            if (alpha_lod > 0.0f) {
                ImVec2 peso_pos(o->posicion.x, o->posicion.y - r - h * 0.45f);
                char peso_txt[16];
                snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
                ImVec2 ts = ImGui::CalcTextSize(peso_txt);
                dl->AddRectFilled(
                    ImVec2(peso_pos.x - ts.x*0.5f - 3, peso_pos.y - ts.y*0.5f - 1),
                    ImVec2(peso_pos.x + ts.x*0.5f + 3, peso_pos.y + ts.y*0.5f + 1),
                    IM_COL32(20, 20, 25, (int)(200 * alpha_lod)), 3.0f);
                dl->AddText(ImVec2(peso_pos.x - ts.x*0.5f, peso_pos.y - ts.y*0.5f),
                    IM_COL32(200, 210, 220, (int)(220 * alpha_lod)), peso_txt);
            }
            continue;
        }

        ImU32 col = es_g2 ? IM_COL32(200, 200, 50, 150) : IM_COL32(120, 130, 140, 70);
        float grosor = 1.0f + std::min(a.peso_actual / 20.0f, 2.0f);

        auto par = std::make_pair(a.origen_id, a.destino_id);
        auto parR = std::make_pair(a.destino_id, a.origen_id);

        // Detectar curva (par bidireccional)
        bool es_curva = false;
        ImVec2 punto_control(0, 0);
        if (!es_g2 && o != d) {
            uint64_t reverse_key = ((uint64_t)a.destino_id << 32) | (uint32_t)a.origen_id;
            if (edge_lookup.count(reverse_key)) {
                es_curva = true;
                float desplazo = a.es_dirigida ? 45.0f : 30.0f;
                punto_control = calcularPuntoControlCurva(o->posicion, d->posicion, desplazo);
            }
        }

        // animacion
        if (!es_g2) {
            bool anim_activa = (grafos.anim_estado.activa || grafos.anim_estado.paso_actual >= 0);
            if (anim_activa) {
                bool check_par_c = grafos.anim_estado.confirmadas.count(par) != 0;
                bool check_parR_c = !a.es_dirigida && grafos.anim_estado.confirmadas.count(parR) != 0;
                if (check_par_c || check_parR_c) {
                    col = IM_COL32(255, 179, 0, 255); grosor = 5.0f;
                    lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(255, 230, 100, 80), grosor + 4.0f);
                } else {
                    bool check_par_e = grafos.anim_estado.exploradas.count(par) != 0;
                    bool check_parR_e = !a.es_dirigida && grafos.anim_estado.exploradas.count(parR) != 0;
                    if (check_par_e || check_parR_e) {
                        col = IM_COL32(0, 188, 212, 220); grosor = 3.5f;
                    } else {
                        bool check_par_d = grafos.anim_estado.descartadas.count(par) != 0;
                        bool check_parR_d = !a.es_dirigida && grafos.anim_estado.descartadas.count(parR) != 0;
                        if (check_par_d || check_parR_d) {
                            float pulse_a = (sinf(tiempo * 5.0f) + 1.0f) * 0.5f;
                            col = IM_COL32(255, 68, 68, (int)(60 + pulse_a * 60)); grosor = 2.5f;
                        }
                    }
                }
            }
            bool mostrar_res = (!grafos.anim_estado.activa && grafos.anim_estado.paso_actual >= (int)grafos.anim_estado.pasos.size() - 1);
            if (!anim_activa || mostrar_res) {
                if (!grafos.mostrar_mst && grafos.ruta_optima.size() >= 2) {
                    for (size_t i = 0; i + 1 < grafos.ruta_optima.size(); i++) {
                        if ((a.origen_id == grafos.ruta_optima[i] && a.destino_id == grafos.ruta_optima[i + 1]) ||
                            (!a.es_dirigida && a.origen_id == grafos.ruta_optima[i + 1] && a.destino_id == grafos.ruta_optima[i])) {
                            col = IM_COL32(255, 179, 0, 255); grosor = 6.0f;
                            lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, IM_COL32(255, 255, 255, 150), grosor + 4.0f); break;
                        }
                    }
                }
                if (grafos.mostrar_mst) {
                    for (const auto& m : grafos.aristas_mst) {
                        if ((a.origen_id == m.origen_id && a.destino_id == m.destino_id) ||
                            (!a.es_dirigida && a.origen_id == m.destino_id && a.destino_id == m.origen_id))
                            { col = IM_COL32(200, 50, 200, 255); grosor = 5.0f; break; }
                    }
                }
            }
        }

        {
            if (!es_g2 && editando_g2) {
                col = (col & 0x00FFFFFF) | (40 << 24);
            }

            // Adjacency highlighting
            if (vecinos_hover) {
                bool adyacente = vecinos_hover->count(a.origen_id) && vecinos_hover->count(a.destino_id);
                if (adyacente) {
                    col = (col & 0x00FFFFFF) | (255 << 24);
                    grosor = std::max(grosor, 3.5f);
                } else {
                    col = (col & 0x00FFFFFF) | (25 << 24);
                    grosor = std::min(grosor, 1.0f);
                }
            }

            lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, col, grosor);

            if (a.es_dirigida) {
                ImVec2 fp1, fp2, fp3;
                if (es_curva) {
                    float t_tang = 0.99f;
                    ImVec2 tan_pt = puntoBezierCuadratico(o->posicion, punto_control, d->posicion, t_tang);
                    calcularPuntaFlecha(tan_pt, d->posicion, get_radio_dibujo(d), fp1, fp2, fp3);
                } else {
                    calcularPuntaFlecha(o->posicion, d->posicion, get_radio_dibujo(d), fp1, fp2, fp3);
                }
                dl->AddTriangleFilled(fp1, fp2, fp3, col);
                dl->AddTriangle(fp1, fp2, fp3, IM_COL32(255, 255, 255, 80), 1.0f);
            }
        }

        float alpha_lod = std::clamp((ui.zoom_lienzo - 0.25f) / 0.25f, 0.0f, 1.0f);
        if (alpha_lod > 0.0f) {
            ImVec2 mid = puntoEnArista(o->posicion, d->posicion, punto_control, es_curva, 0.5f);
            char peso_txt[16];
            snprintf(peso_txt, sizeof(peso_txt), "%.1f", a.peso_actual);
            ImVec2 ts = ImGui::CalcTextSize(peso_txt);
            dl->AddRectFilled(
                ImVec2(mid.x - ts.x * 0.5f - 3, mid.y - ts.y * 0.5f - 1),
                ImVec2(mid.x + ts.x * 0.5f + 3, mid.y + ts.y * 0.5f + 1),
                IM_COL32(20, 20, 25, (int)(200 * alpha_lod)), 3.0f);
            ImU32 col_txt = IM_COL32(200, 210, 220, 255);
            col_txt = (col_txt & 0x00FFFFFF) | ((int)(220 * alpha_lod) << 24);
            dl->AddText(ImVec2(mid.x - ts.x * 0.5f, mid.y - ts.y * 0.5f), col_txt, peso_txt);
        }

        // cruces
        if (!es_g2 && grafos.mostrar_coloreo &&
            grafos.planar_analizado &&
            grafos.mostrar_cruces &&
            grafos.resultado_planaridad.cruces_estimadas > 0) {
            bool cruza = false;
            for (const auto& par : grafos.resultado_planaridad.aristas_cruce) {
                if (par.first == (int)idx || par.second == (int)idx) { cruza = true; break; }
            }
            if (cruza) {
                float pulse = sinf(tiempo * 3.0f + (float)idx) * 0.3f + 0.7f;
                ImU32 col_cruce = IM_COL32(255, 120, 0, (int)(pulse * 120));
                lineaArista(dl, o->posicion, d->posicion, get_radio_dibujo(o), get_radio_dibujo(d), punto_control, es_curva, col_cruce, 6.0f);
            }
        }

    }
}

// Dibujar nodos de un grafo
static void dibujarNodosEnGrafo(ImDrawList* dl, const Grafo& g_dib,
    EstadoUI& ui, EstadoGrafos& grafos,
    bool usar_ranking, float rank_size_min, float rank_size_max,
    float rank_color_min, float rank_color_max,
    bool es_g2, bool editando_g2,
    ImVec2 origin, ImVec2 tamano, float tiempo, ImVec2 mouse,
    bool& tooltip_mostrado,
    const std::unordered_set<int>* vecinos_hover)
{
    auto getValorRank = [&](int nid, EstadoUI::EstadoRanking::Atributo attr) -> float {
        switch (attr) {
            case EstadoUI::EstadoRanking::GRADO: return (float)g_dib.gradoNodo(nid);
            default: return 0;
        }
    };

    for (auto& n : g_dib.nodos) {
        if (!es_g2 && grafos.ocultar_vertices_fractal && ui.herramienta_activa == EstadoUI::CatFractales) {
            continue;
        }

        float r_cull = n.radio + 50.0f;
        if (n.posicion.x < origin.x - r_cull || n.posicion.x > origin.x + tamano.x + r_cull ||
            n.posicion.y < origin.y - r_cull || n.posicion.y > origin.y + tamano.y + r_cull) {
            continue;
        }

        ImU32 colorFondo, colorBorde;
        bool es_anim = (!es_g2 && (grafos.anim_estado.activa || grafos.anim_estado.paso_actual >= 0));

        if (es_g2) {
            colorFondo = IM_COL32(180, 180, 0, 255);
            colorBorde = IM_COL32(255, 255, 100, 255);
        } else if (es_anim && grafos.anim_estado.procesando.count(n.id)) {
            colorFondo = IM_COL32(255, 215, 0, 200);
            colorBorde = IM_COL32(255, 235, 100, 255);
        } else if (es_anim && grafos.anim_estado.visitados.count(n.id)) {
            colorFondo = IM_COL32(0, 230, 118, 200);
            colorBorde = IM_COL32(100, 255, 180, 255);
        } else if (grafos.mostrar_coloreo) {
            if (grafos.modo_fractal &&
                (int)grafos.colores_fractales.size() > n.id &&
                grafos.colores_fractales[n.id] != 0) {
                uint32_t fc = grafos.colores_fractales[n.id];
                colorFondo = IM_COL32(
                    (fc >> 16) & 0xFF, (fc >> 8) & 0xFF, fc & 0xFF, 255);
                colorBorde = IM_COL32(255, 255, 255, 180);
            } else if ((int)grafos.colores_nodos.size() > n.id &&
                       grafos.colores_nodos[n.id] >= 0) {
                ImU32 paleta[] = {
                    IM_COL32(230, 60, 60, 255), IM_COL32(60, 200, 80, 255),
                    IM_COL32(60, 100, 230, 255), IM_COL32(230, 200, 50, 255),
                    IM_COL32(200, 60, 200, 255), IM_COL32(60, 200, 200, 255)
                };
                colorFondo = paleta[grafos.colores_nodos[n.id] % 6];
                colorBorde = IM_COL32(255, 255, 255, 180);
            } else {
                colorFondo = IM_COL32(40, 40, 48, 255);
                colorBorde = IM_COL32(100, 100, 100, 200);
            }
        } else if (ui.modo_actual == EstadoUI::ModoApp::AeroGrafos) {
            colorFondo = IM_COL32(30, 35, 45, 255);
            switch (n.tipo) {
                case TipoHardware::Servidor: colorBorde = IM_COL32(41, 128, 185, 255); break;
                case TipoHardware::Router:   colorBorde = IM_COL32(211, 84, 0, 255); break;
                case TipoHardware::Switch:   colorBorde = IM_COL32(39, 174, 96, 255); break;
                case TipoHardware::Firewall: colorBorde = IM_COL32(192, 57, 43, 255); break;
                case TipoHardware::Terminal: colorBorde = IM_COL32(142, 68, 173, 255); break;
                default: colorBorde = IM_COL32(149, 165, 166, 255); break;
            }
        } else {
            colorFondo = IM_COL32(0, 160, 130, 255);
            colorBorde = IM_COL32(200, 220, 230, 200);
        }



        // seleccion / hover
        if ((es_g2 && editando_g2 && n.id == ui.nodo_seleccionado) ||
            (!es_g2 && !editando_g2 && n.id == ui.nodo_seleccionado)) {
            float t_sel = tiempo;
            float pulse1 = sinf(t_sel * 3.0f) * 3.0f + 5.0f;
            float pulse2 = sinf(t_sel * 3.0f + 1.0f) * 2.0f + 3.0f;
            float alpha1 = (sinf(t_sel * 3.0f) + 1.0f) * 0.5f;
            dl->AddCircle(n.posicion, n.radio + pulse1 + 4, IM_COL32(255, 200, 80, (int)(40 + alpha1 * 40)), 32, 2.0f);
            dl->AddCircleFilled(n.posicion, n.radio + pulse2 + 2, IM_COL32(255, 180, 50, 30), 32);
            colorBorde = IM_COL32(255, 210, 90, 255);
        }
        if ((es_g2 && editando_g2 && n.id == ui.nodo_hover && n.id != ui.nodo_seleccionado) ||
            (!es_g2 && !editando_g2 && n.id == ui.nodo_hover && n.id != ui.nodo_seleccionado)) {
            dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(0, 255, 200, 30), 32);
        }

        // arbol
        if (!es_g2 && grafos.arbol_analizado) {
            if (n.id == grafos.arbol_props.raiz_id)
                dl->AddCircleFilled(n.posicion, n.radio + 6, IM_COL32(255, 200, 0, 60), 32);
            auto& hojas_ref = grafos.arbol_props.hojas;
            if (std::find(hojas_ref.begin(), hojas_ref.end(), n.id) != hojas_ref.end())
                dl->AddCircleFilled(n.posicion, n.radio + 4, IM_COL32(50, 200, 50, 50), 32);
            if (grafos.arbol_props.nivel.count(n.id)) {
                char nivel_txt[8]; snprintf(nivel_txt, sizeof(nivel_txt), "L%d", grafos.arbol_props.nivel.at(n.id));
                ImVec2 lt = ImGui::CalcTextSize(nivel_txt);
                dl->AddText(ImVec2(n.posicion.x - lt.x * 0.5f, n.posicion.y - n.radio - 18), IM_COL32(180, 220, 180, 200), nivel_txt);
            }
        }

        // efecto pop al visitar
        if (es_anim && grafos.anim_estado.visitados.count(n.id) &&
            grafos.anim_estado.tiempo_visita_nodo.count(n.id)) {
            float t_since = tiempo - grafos.anim_estado.tiempo_visita_nodo[n.id];
            if (t_since < 0.4f) {
                float scale = 1.0f + Easing::easeOutBounce(t_since / 0.4f) * 0.3f;
                dl->AddCircleFilled(n.posicion, n.radio * scale, IM_COL32(0, 230, 120, 80), 32);
            }
        }

        // radio dinamico con ranking visual aplicado
        float radio_dibujo = n.radio;
        ImU32 colorRanking = colorFondo;

        if (usar_ranking) {
            auto& rk = ui.ranking;
            if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                float range = std::max(0.001f, rank_size_max - rank_size_min);
                float val = getValorRank(n.id, rk.atributo_size);
                float t = (val - rank_size_min) / range;
                if (rk.invertir_size) t = 1.0f - t;
                t = std::max(0.0f, std::min(1.0f, t));
                radio_dibujo = rk.min_size + t * (rk.max_size - rk.min_size);
            }
            if (rk.atributo_color != EstadoUI::EstadoRanking::NINGUNO) {
                float range = std::max(0.001f, rank_color_max - rank_color_min);
                float val = getValorRank(n.id, rk.atributo_color);
                float t = (val - rank_color_min) / range;
                if (rk.invertir_color) t = 1.0f - t;
                t = std::max(0.0f, std::min(1.0f, t));
                colorRanking = IM_COL32(
                    (int)(50 + t * 205),
                    (int)(100 * (1.0f - t)),
                    (int)(255 * (1.0f - t)),
                    255);
                colorFondo = colorRanking;
            }
        }

        if ((!es_g2 && !editando_g2 && n.id == ui.nodo_hover) ||
            (es_g2 && editando_g2 && n.id == ui.nodo_hover)) {
            radio_dibujo += 2.0f + sinf(tiempo * 8.0f) * 1.5f;
        }

        if (!es_g2 && editando_g2) {
            colorFondo = (colorFondo & 0x00FFFFFF) | (40 << 24);
            colorBorde = (colorBorde & 0x00FFFFFF) | (40 << 24);
        }

        // Adjacency highlighting
        if (vecinos_hover && !vecinos_hover->count(n.id)) {
            colorFondo = (colorFondo & 0x00FFFFFF) | (40 << 24);
            colorBorde = (colorBorde & 0x00FFFFFF) | (40 << 24);
        }

        dl->AddCircleFilled(n.posicion, radio_dibujo, colorFondo, 32);
        dl->AddCircle(n.posicion, radio_dibujo, colorBorde, 32, 2.5f);

        if (grafos.mostrar_coloreo && grafos.modo_fractal && !es_g2) {
            float fp = sinf(tiempo * 2.0f + (float)n.id * 1.7f) * 0.3f + 0.7f;
            dl->AddCircle(n.posicion, radio_dibujo + 6 + fp * 4, IM_COL32(150, 60, 200, (int)(fp * 120)), 32, 1.5f);
        }

        float alpha_lod = 1.0f;
        if (alpha_lod > 0.0f) {
            int alpha_base = (!es_g2 && editando_g2) ? 60 : 255;
            if (vecinos_hover && !vecinos_hover->count(n.id)) alpha_base = 40;
            int alpha_texto = (int)(alpha_base * alpha_lod);

            if (!es_g2 && ui.modo_actual == EstadoUI::ModoApp::AeroGrafos) {
                const char* icono = iconoHardware(n.tipo);
                ImVec2 is = ImGui::CalcTextSize(icono);
                ImU32 iconColor = (colorBorde & 0x00FFFFFF) | (alpha_texto << 24);
                dl->AddText(ImVec2(n.posicion.x - is.x * 0.5f, n.posicion.y - is.y * 0.5f), iconColor, icono);
                ImVec2 ns = ImGui::CalcTextSize(n.nombre.c_str());
                int sub_alpha = (int)(((!es_g2 && editando_g2) ? 40 : 200) * alpha_lod);
                dl->AddText(ImVec2(n.posicion.x - ns.x * 0.5f, n.posicion.y + radio_dibujo + 3), IM_COL32(190, 195, 200, sub_alpha), n.nombre.c_str());
            } else {
                ImVec2 ts = ImGui::CalcTextSize(n.nombre.c_str());
                dl->AddText(ImVec2(n.posicion.x - ts.x * 0.5f, n.posicion.y - ts.y * 0.5f), IM_COL32(255, 255, 255, alpha_texto), n.nombre.c_str());
            }
        }

        // tooltip nodo
        if (!es_g2 && n.id == ui.nodo_hover && !tooltip_mostrado) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", n.nombre.c_str());
            ImGui::Text("Grado: %d", g_dib.gradoNodo(n.id));
            ImGui::EndTooltip();
            tooltip_mostrado = true;
        }
    }
}

// Particula de animacion
static void dibujarParticulaAnimacion(ImDrawList* dl, EstadoGrafos& grafos,
    EstadoUI& ui, const Grafo& g_dib, bool es_g2)
{
    if (!(grafos.anim_estado.activa || grafos.anim_estado.paso_actual >= 0) ||
        !grafos.anim_estado.particula.activa || es_g2) return;

    float t_ease = Easing::easeInOutCubic(grafos.anim_estado.particula.progreso);
    ImVec2 p_inicio_raw = grafos.anim_estado.particula.pos_inicio;
    ImVec2 p_fin_raw = grafos.anim_estado.particula.pos_fin;
    ImVec2 p_inicio(
        p_inicio_raw.x * ui.zoom_lienzo + ui.offset_lienzo.x,
        p_inicio_raw.y * ui.zoom_lienzo + ui.offset_lienzo.y
    );
    ImVec2 p_fin(
        p_fin_raw.x * ui.zoom_lienzo + ui.offset_lienzo.x,
        p_fin_raw.y * ui.zoom_lienzo + ui.offset_lienzo.y
    );
    bool p_curvo = false;
    ImVec2 p_pc;
    int paso_cur = grafos.anim_estado.paso_actual;
    if (paso_cur >= 0 && paso_cur < (int)grafos.anim_estado.pasos.size()) {
        int a_o = grafos.anim_estado.pasos[paso_cur].arista_origen;
        int a_d = grafos.anim_estado.pasos[paso_cur].arista_destino;
        if (a_o >= 0 && a_d >= 0 && a_o != a_d) {
            for (const auto& other : g_dib.aristas) {
                if (other.origen_id == a_d && other.destino_id == a_o) {
                    p_curvo = true;
                    p_pc = calcularPuntoControlCurva(p_inicio, p_fin);
                    break;
                }
            }
        }
    }
    ImVec2 pos = puntoEnArista(p_inicio, p_fin, p_pc, p_curvo, t_ease);
    float halo_r = grafos.anim_estado.particula.radio * 2.5f * (1.0f - grafos.anim_estado.particula.progreso * 0.5f);
    dl->AddCircleFilled(pos, halo_r, (grafos.anim_estado.particula.color & 0x00FFFFFF) | (60 << 24), 24);
    dl->AddCircleFilled(pos, grafos.anim_estado.particula.radio, grafos.anim_estado.particula.color, 24);
    dl->AddCircleFilled(pos, grafos.anim_estado.particula.radio * 0.4f, IM_COL32(255, 255, 255, 200), 16);
}

// Notificaciones emergentes
// Overlay de resultados de algoritmos
static void dibujarOverlayAlgoritmos(ImDrawList* dl, EstadoUI& ui,
                                      EstadoGrafos& grafos, const Grafo& red,
                                      ImVec2 origin)
{
    if (ui.modo_actual != EstadoUI::ModoApp::Grafos) return;
    float ox = origin.x + 12, oy = origin.y + 12;

    if (!grafos.ruta_optima.empty()) {
        char buf[128];
        snprintf(buf, sizeof(buf), "ruta optima: %d saltos, costo %.1f",
            (int)grafos.ruta_optima.size() - 1,
            grafos.dijkstra_costo_total);
        dl->AddText(ImVec2(ox, oy), IM_COL32(0, 255, 180, 240), ICON_FA_ROUTE);
        dl->AddText(ImVec2(ox + 22, oy), IM_COL32(0, 255, 180, 240), buf);
        oy += 22;
    }

    if (ui.herramienta_activa == EstadoUI::CatBusqueda) {
        if (!grafos.bfs_resultado.orden_visita.empty()) {
            char buf[64];
            snprintf(buf, sizeof(buf), "BFS: %d nodos visitados",
                (int)grafos.bfs_resultado.orden_visita.size());
            dl->AddText(ImVec2(ox, oy), IM_COL32(100, 200, 255, 240), buf);
            oy += 22;
        }
        if (!grafos.dfs_resultado.orden_visita.empty()) {
            char buf[64];
            snprintf(buf, sizeof(buf), "DFS: %d pasos, %d back-edges",
                (int)grafos.dfs_resultado.orden_visita.size(),
                (int)grafos.dfs_resultado.back_edges.size());
            dl->AddText(ImVec2(ox, oy), IM_COL32(180, 130, 255, 240), buf);
            oy += 22;
        }
    }

    if (grafos.mostrar_coloreo) {
        char buf[128];
        if (grafos.modo_fractal) {
            snprintf(buf, sizeof(buf), "coloreo fractal (colores dinamicos) | colores=%d",
                grafos.resultado_coloreo.num_colores);
            dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_FAN);
            dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
            oy += 22;
            dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                "los nodos cambian de color con el tiempo segun su posicion fractal");
        } else {
            snprintf(buf, sizeof(buf), "coloreo clasico | colores usados: %d (greedy), %d (welsh-powell)",
                grafos.resultado_coloreo.num_colores,
                grafos.resultado_welsh_powell.num_colores);
            dl->AddText(ImVec2(ox, oy), IM_COL32(200, 100, 255, 240), ICON_FA_PAINTBRUSH);
            dl->AddText(ImVec2(ox + 22, oy), IM_COL32(200, 100, 255, 240), buf);
            oy += 22;
            dl->AddText(ImVec2(ox, oy), IM_COL32(150, 60, 200, 180),
                "dos nodos conectados nunca comparten el mismo color");
        }
        oy += 22;

        if (grafos.planar_analizado) {
            ImU32 col_planar = grafos.resultado_planaridad.es_planar
                ? IM_COL32(0, 255, 120, 240) : IM_COL32(255, 150, 0, 240);
            const char* txt = grafos.resultado_planaridad.es_planar
                ? "grafo planar (χ<=4 por teorema 4-colores)"
                : "no planar";
            dl->AddText(ImVec2(ox, oy), col_planar, txt);
            oy += 22;
        }
    }

    if (grafos.mostrar_mst && !grafos.aristas_mst.empty()) {
        float peso_total = 0;
        for (const auto& a : grafos.aristas_mst) peso_total += a.peso;
        char buf[64];
        snprintf(buf, sizeof(buf), "MST: %d aristas, peso total %.1f",
            (int)grafos.aristas_mst.size(), peso_total);
        dl->AddText(ImVec2(ox, oy), IM_COL32(200, 50, 200, 240), buf);
        oy += 22;
    }

    if (grafos.ciclo_analizado) {
        ImU32 col = grafos.resultado_ciclos.tiene_ciclo
            ? IM_COL32(255, 180, 0, 240) : IM_COL32(0, 255, 120, 240);
        const char* txt = grafos.resultado_ciclos.tiene_ciclo
            ? "ciclo detectado" : "grafo aciclico (arbol)";
        dl->AddText(ImVec2(ox, oy), col, txt);
        oy += 22;
    }

    if (grafos.anim_estado.activa && grafos.anim_estado.paso_actual >= 0 &&
        grafos.anim_estado.paso_actual < (int)grafos.anim_estado.pasos.size()) {
        const auto& paso = grafos.anim_estado.pasos[grafos.anim_estado.paso_actual];
        if (!paso.descripcion.empty()) {
            dl->AddText(ImVec2(ox, oy), IM_COL32(180, 230, 255, 240), paso.descripcion.c_str());
            oy += 22;
        }
    }
}



// entrada principal
namespace LienzoRed {

void dibujar(Grafo& red, Interfaz& self) {
    auto& ui = self.estado_ui;
    auto& grafos = self.estado_grafos;
    static int frames_init = 0;
    if (frames_init < 5) {
        ImGui::SetNextWindowFocus();
        frames_init++;
    }
    ImGui::Begin("Lienzo de Red");
    ImVec2 tamano = ImGui::GetContentRegionAvail();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float tiempo = (float)ImGui::GetTime();

    bool tooltip_mostrado = false;

    // guardar rect del lienzo para exportar png
    ui.lienzo_origin = origin;
    ui.lienzo_size   = tamano;

    // Grid
    dibujarGrid(dl, ui, origin, tamano);

    // raton
    ImVec2 mouse = ImGui::GetMousePos();
    bool en_canvas = ImGui::IsWindowHovered();

    bool editando_g2 = (ui.herramienta_activa == EstadoUI::CatIsomorfismo && grafos.iso_editando_g2);
    Grafo& grafo_actual = editando_g2 ? grafos.grafo_iso_g2 : red;

    ui.nodo_hover = -1;
    for (auto& n : grafo_actual.nodos) {
        float screen_nx = n.posicion.x * ui.zoom_lienzo + ui.offset_lienzo.x;
        float screen_ny = n.posicion.y * ui.zoom_lienzo + ui.offset_lienzo.y;
        float screen_r  = n.radio * ui.zoom_lienzo;
        float dx = mouse.x - screen_nx, dy = mouse.y - screen_ny;
        if (sqrtf(dx * dx + dy * dy) <= screen_r) ui.nodo_hover = n.id;
    }

    // Adjacency highlighting: neighbor set (computed once per frame)
    std::unordered_set<int> vecinos_hover_set;
    const std::unordered_set<int>* ptr_vecinos_hover = nullptr;
    {
        bool hay_anim = grafos.anim_estado.activa || grafos.anim_estado.paso_actual >= 0;
        if (ui.resaltado_vecinos && ui.nodo_hover != -1 && !hay_anim && !grafos.mostrar_coloreo) {
            auto v = grafo_actual.vecinos(ui.nodo_hover);
            vecinos_hover_set.insert(v.begin(), v.end());
            vecinos_hover_set.insert(ui.nodo_hover);
            ptr_vecinos_hover = &vecinos_hover_set;
        }
    }

    // Click izquierdo
    if (en_canvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ui.nodo_seleccionado = ui.nodo_hover;
        if (ui.nodo_seleccionado != -1) {
            self.historial.capturar(grafo_actual);
            ui.arrastrando = true;
        }
    }

    // Arrastre
    if (ui.arrastrando && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Nodo* n = grafo_actual.obtenerNodo(ui.nodo_seleccionado);
            if (n) {
                ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                if (ImGui::GetIO().KeyShift) {
                    for (auto& n_iter : grafo_actual.nodos) {
                        n_iter.posicion.x += d.x / ui.zoom_lienzo;
                        n_iter.posicion.y += d.y / ui.zoom_lienzo;
                    }
                } else {
                    n->posicion.x += d.x / ui.zoom_lienzo;
                    n->posicion.y += d.y / ui.zoom_lienzo;
                }
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) ui.arrastrando = false;

    // Paneo
    bool panning_activo = ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
        (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ui.nodo_seleccionado == -1 && ui.nodo_hover == -1);

    if (en_canvas && panning_activo) {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        ui.pan_velocity.x = d.x;
        ui.pan_velocity.y = d.y;
    }

    if (std::abs(ui.pan_velocity.x) > 0.1f || std::abs(ui.pan_velocity.y) > 0.1f) {
        ui.offset_lienzo.x += ui.pan_velocity.x;
        ui.offset_lienzo.y += ui.pan_velocity.y;
        ui.pan_velocity.x *= 0.85f;
        ui.pan_velocity.y *= 0.85f;
    }

    // Zoom
    float wheel = ImGui::GetIO().MouseWheel;
    if (en_canvas && wheel != 0.0f) {
        ui.zoom_velocity += wheel * 0.1f;
    }

    if (en_canvas || ImGui::IsWindowFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) ui.zoom_velocity += 0.2f;
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) ui.zoom_velocity -= 0.2f;
    }

    if (std::abs(ui.zoom_velocity) > 0.001f) {
        float factor = 1.0f + ui.zoom_velocity;
        ImVec2 centro = en_canvas ? mouse : ImVec2(origin.x + tamano.x * 0.5f, origin.y + tamano.y * 0.5f);
        ui.zoom_lienzo *= factor;
        ui.offset_lienzo.x = centro.x + (ui.offset_lienzo.x - centro.x) * factor;
        ui.offset_lienzo.y = centro.y + (ui.offset_lienzo.y - centro.y) * factor;
        ui.zoom_velocity *= 0.7f;
    }

    // Fisicas
    if (ui.fisicas_estado_cambiado) {
        ui.fisicas_estado_cambiado = false;
        if (ui.fisicas_activas) {
            ui.fisicas_posiciones_guardadas.clear();
            for (const auto& n : grafo_actual.nodos) {
                ui.fisicas_posiciones_guardadas[n.id] = n.posicion;
            }
            self.registrarLog("Físicas activadas. Posiciones guardadas.");
        } else {
            for (auto& n : grafo_actual.nodos) {
                if (ui.fisicas_posiciones_guardadas.count(n.id)) {
                    n.posicion = ui.fisicas_posiciones_guardadas[n.id];
                }
            }
            ui.fisicas_posiciones_guardadas.clear();
            self.registrarLog("Físicas desactivadas. Posiciones originales restauradas.");
        }
    }

    if (ui.fisicas_activas) {
        ui.fa2.nodo_arrastrado = ui.arrastrando ? ui.nodo_seleccionado : -1;
        static int skip_fa2 = 0;
        if (skip_fa2 > 0) { skip_fa2--; }
        else {
            double fa2_t0 = ImGui::GetTime();
            ui.fa2.step(grafo_actual, ui.fa2_params);
            double fa2_ms = (ImGui::GetTime() - fa2_t0) * 1000.0;
            if (fa2_ms > 20.0) skip_fa2 = (int)(fa2_ms / 16.0);
        }
    }

    // Creacion de arista (drag con boton derecho)
    if (en_canvas && ImGui::IsMouseDragging(ImGuiMouseButton_Right) && ui.nodo_hover != -1 && !ui.creando_arista_drag) {
        ui.creando_arista_drag = true;
        ui.drag_arista_origen = ui.nodo_hover;
    }

    if (ui.creando_arista_drag) {
        Nodo* origen = grafo_actual.obtenerNodo(ui.drag_arista_origen);
        if (origen) {
            ImVec2 screen_origen(origen->posicion.x * ui.zoom_lienzo + ui.offset_lienzo.x,
                                 origen->posicion.y * ui.zoom_lienzo + ui.offset_lienzo.y);
            dl->AddLine(screen_origen, mouse, IM_COL32(0, 255, 200, 150), 4.0f);
            dl->AddLine(screen_origen, mouse, IM_COL32(200, 255, 255, 255), 1.5f);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            if (ui.nodo_hover != -1) {
                ui.pendiente_arista_origen = ui.drag_arista_origen;
                ui.pendiente_arista_destino = ui.nodo_hover;
                ui.pendiente_arista_peso = 1.0f;
                ui.pendiente_arista_dirigida = ui.aristas_dirigidas;
                ImGui::OpenPopup("CrearArista");
            }
            ui.creando_arista_drag = false;
            ui.drag_arista_origen = -1;
        }
    } else if (en_canvas && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        if (ui.nodo_hover == -1) {
            ImVec2 mouse_mundo((mouse.x - ui.offset_lienzo.x) / ui.zoom_lienzo,
                               (mouse.y - ui.offset_lienzo.y) / ui.zoom_lienzo);
            ui.pos_click_derecho = mouse;
            float current_radius = grafo_actual.nodos.empty() ? 20.0f : grafo_actual.nodos[0].radio;
            if (editando_g2) {
                self.historial.capturar(grafo_actual);
                grafo_actual.agregarNodo(mouse_mundo);
                grafo_actual.nodos.back().nombre = "U" + std::to_string(grafo_actual.nodos.back().id);
                grafo_actual.nodos.back().radio = current_radius;
                grafos.iso_analizado = false;
            } else if (ui.modo_actual == Interfaz::ModoApp::AeroGrafos) {
                ImGui::OpenPopup("CrearEquipo");
            } else {
                self.historial.capturar(grafo_actual);
                grafo_actual.agregarNodo(mouse_mundo);
                grafo_actual.nodos.back().nombre = "V" + std::to_string(grafo_actual.nodos.back().id);
                grafo_actual.nodos.back().radio = current_radius;
                self.registrarLog("Nodo creado: " + grafo_actual.nodos.back().nombre);
                grafos.ruta_optima.clear(); grafos.aristas_mst.clear(); grafos.mostrar_mst = false;
            }
        } else if (ui.nodo_hover != -1) {
            if (ui.modo_actual == Interfaz::ModoApp::AeroGrafos && ui.nodo_seleccionado == -1) {
                ui.nodo_seleccionado = ui.nodo_hover;
                ImGui::OpenPopup("MenuNodo");
            } else if (ui.nodo_seleccionado != -1) {
                ui.pendiente_arista_origen = ui.nodo_seleccionado;
                ui.pendiente_arista_destino = ui.nodo_hover;
                ui.pendiente_arista_peso = 1.0f;
                ui.pendiente_arista_dirigida = ui.aristas_dirigidas;
                ImGui::OpenPopup("CrearArista");
            } else {
                ui.nodo_seleccionado = ui.nodo_hover;
            }
        }
    }

    // borrar
    if (en_canvas && ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().WantTextInput) {
        grafo_actual.limpiar();
        grafos.ruta_optima.clear(); grafos.aristas_mst.clear(); grafos.mostrar_mst = false;
        self.registrarLog("Grafo entero eliminado (Ctrl+N)");
        ui.nodo_seleccionado = -1; ui.arrastrando = false;
    } else if (ui.nodo_seleccionado != -1 && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && !ImGui::GetIO().WantTextInput) {
        if (editando_g2) {
            grafo_actual.eliminarNodo(ui.nodo_seleccionado);
            grafos.iso_analizado = false;
        } else {
            self.registrarLog("Nodo eliminado: " + grafo_actual.nombreNodo(ui.nodo_seleccionado));
            grafo_actual.eliminarNodo(ui.nodo_seleccionado);
            grafos.ruta_optima.clear(); grafos.aristas_mst.clear(); grafos.mostrar_mst = false;
        }
        ui.nodo_seleccionado = -1; ui.arrastrando = false;
    }

    // Popups
    if (ImGui::BeginPopup("CrearEquipo")) {
        ImGui::Text(ICON_FA_PLUS " Nuevo Equipamiento"); ImGui::Separator();
        static int tipo_sel = 0;
        const char* tipos[] = {"Servidor", "Router", "Switch", "Firewall", "Terminal"};
        const char* iconos[] = {ICON_FA_SERVER, ICON_FA_NETWORK_WIRED, ICON_FA_RIGHT_LEFT, ICON_FA_SHIELD_HALVED, ICON_FA_DESKTOP};
        for (int i = 0; i < 5; i++) {
            char label[64]; snprintf(label, sizeof(label), "%s %s", iconos[i], tipos[i]);
            if (ImGui::Selectable(label)) {
                self.historial.capturar(grafo_actual);
                grafo_actual.agregarNodo(ui.pos_click_derecho, (TipoHardware)i);
                self.registrarLog("Hardware desplegado: " + std::string(tipos[i]) + " " + grafo_actual.nodos.back().nombre);
                grafos.ruta_optima.clear(); grafos.aristas_mst.clear(); grafos.mostrar_mst = false;
            }
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("CrearArista")) {
        ImGui::Text(ICON_FA_LINK " Nueva Conexion");
        ImGui::Text("%s -> %s", grafo_actual.nombreNodo(ui.pendiente_arista_origen).c_str(),
                    grafo_actual.nombreNodo(ui.pendiente_arista_destino).c_str());
        ImGui::Separator();
        ImGui::InputFloat("Peso / Latencia", &ui.pendiente_arista_peso, 0.5f, 5.0f, "%.1f");
        if (ui.pendiente_arista_peso < 0.1f) ui.pendiente_arista_peso = 0.1f;
        bool es_bucle = (ui.pendiente_arista_origen == ui.pendiente_arista_destino);
        if (es_bucle) {
            ui.pendiente_arista_dirigida = true;
            ImGui::TextDisabled(ICON_FA_ARROW_RIGHT " Bucle - dirigida forzada");
        } else {
            ImGui::Checkbox("Dirigida", &ui.pendiente_arista_dirigida);
        }
        if (ImGui::Button(ICON_FA_CHECK " Crear", ImVec2(100, 0))) {
            self.historial.capturar(grafo_actual);
            grafo_actual.agregarArista(ui.pendiente_arista_origen, ui.pendiente_arista_destino, ui.pendiente_arista_peso, ui.pendiente_arista_dirigida);
            if (!editando_g2) {
                self.registrarLog("Arista creada: " + grafo_actual.nombreNodo(ui.pendiente_arista_origen) + " - " +
                    grafo_actual.nombreNodo(ui.pendiente_arista_destino) + " (peso=" + std::to_string((int)ui.pendiente_arista_peso) + ")");
                grafos.ruta_optima.clear(); grafos.aristas_mst.clear(); grafos.mostrar_mst = false;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK " Cancelar", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("MenuNodo")) {
        Nodo* n = grafo_actual.obtenerNodo(ui.nodo_seleccionado);
        if (n) {
            ImGui::TextDisabled("nodo: %s", n->nombre.c_str());
            if (ImGui::Selectable(ICON_FA_TRASH " eliminar nodo")) {
                grafo_actual.eliminarNodo(n->id);
                ui.nodo_seleccionado = -1;
            }
        }
        ImGui::EndPopup();
    }

    // Transformacion visual para dibujo
    Grafo red_dibujo = red;
    Grafo iso_dibujo = grafos.grafo_iso_g2;
    for(auto& n : red_dibujo.nodos) {
        n.posicion.x = n.posicion.x * ui.zoom_lienzo + ui.offset_lienzo.x;
        n.posicion.y = n.posicion.y * ui.zoom_lienzo + ui.offset_lienzo.y;
        n.radio *= ui.zoom_lienzo;
    }
    for(auto& n : iso_dibujo.nodos) {
        n.posicion.x = n.posicion.x * ui.zoom_lienzo + ui.offset_lienzo.x;
        n.posicion.y = n.posicion.y * ui.zoom_lienzo + ui.offset_lienzo.y;
        n.radio *= ui.zoom_lienzo;
    }

    std::vector<Grafo*> grafos_a_dibujar = {&red_dibujo};
    if (ui.herramienta_activa == EstadoUI::CatIsomorfismo)
        grafos_a_dibujar.push_back(&iso_dibujo);

    if (editando_g2) {
        const char* cartel = "[ MODO ISOMORFISMO: DIBUJANDO GRAFO A COMPARAR (G2) ]";
        ImVec2 ts = ImGui::CalcTextSize(cartel);
        dl->AddRectFilled(ImVec2(origin.x + tamano.x/2 - ts.x/2 - 15, origin.y + 20),
                          ImVec2(origin.x + tamano.x/2 + ts.x/2 + 15, origin.y + 20 + ts.y + 15),
                          IM_COL32(220, 200, 30, 200), 8.0f);
        dl->AddText(ImVec2(origin.x + tamano.x/2 - ts.x/2, origin.y + 27), IM_COL32(0, 0, 0, 255), cartel);
    }

    // Recorrer grafos
    for (Grafo* ptr_g : grafos_a_dibujar) {
        Grafo& g_dib = *ptr_g;
        bool es_g2 = (&g_dib == &iso_dibujo);

        // Pre-calcular aristas para busqueda O(1)
        std::unordered_set<uint64_t> edge_lookup;
        if (!es_g2) {
            for (const auto& a : g_dib.aristas) {
                edge_lookup.insert(((uint64_t)a.origen_id << 32) | (uint32_t)a.destino_id);
            }
        }

        // Ranking ranges
        float rank_size_min = FLT_MAX, rank_size_max = -FLT_MAX;
        float rank_color_min = FLT_MAX, rank_color_max = -FLT_MAX;
        bool usar_ranking = (!es_g2 && ui.ranking.activo);

        auto getValorRank = [&](int nid, EstadoUI::EstadoRanking::Atributo attr) -> float {
            switch (attr) {
                case EstadoUI::EstadoRanking::GRADO: return (float)g_dib.gradoNodo(nid);
                default: return 0;
            }
        };

        if (usar_ranking) {
            auto& rk = ui.ranking;
            if (rk.atributo_size != EstadoUI::EstadoRanking::NINGUNO) {
                for (const auto& nn : g_dib.nodos) {
                    float v = getValorRank(nn.id, rk.atributo_size);
                    if (v < rank_size_min) rank_size_min = v;
                    if (v > rank_size_max) rank_size_max = v;
                }
            }
            if (rk.atributo_color != EstadoUI::EstadoRanking::NINGUNO) {
                for (const auto& nn : g_dib.nodos) {
                    float v = getValorRank(nn.id, rk.atributo_color);
                    if (v < rank_color_min) rank_color_min = v;
                    if (v > rank_color_max) rank_color_max = v;
                }
            }
        }

        // Dibujar aristas
        dibujarAristasEnGrafo(dl, g_dib, ui, grafos,
            edge_lookup, usar_ranking, rank_size_min, rank_size_max,
            rank_color_min, rank_color_max,
            es_g2, editando_g2,
            origin, tamano, tiempo, mouse, tooltip_mostrado,
            ptr_vecinos_hover);

        // Dibujar nodos
        dibujarNodosEnGrafo(dl, g_dib, ui, grafos,
            usar_ranking, rank_size_min, rank_size_max,
            rank_color_min, rank_color_max,
            es_g2, editando_g2,
            origin, tamano, tiempo, mouse, tooltip_mostrado,
            ptr_vecinos_hover);

        // Particula de animacion
        dibujarParticulaAnimacion(dl, grafos, ui, g_dib, es_g2);
    }

    // Overlay de algoritmos
    dibujarOverlayAlgoritmos(dl, ui, grafos, red, origin);

    ImGui::End();
}

} // namespace LienzoRed
