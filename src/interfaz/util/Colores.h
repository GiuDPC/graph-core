#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"

// Constantes de color semánticas para la UI de OptiClusters
namespace Colores {

// ── Nodos ─────────────────────────────────────────────────────────────────
inline constexpr ImU32 NODO_VISITADO     = IM_COL32(0, 230, 118, 200);
inline constexpr ImU32 NODO_PROCESANDO   = IM_COL32(255, 215, 0, 200);
inline constexpr ImU32 NODO_DEFAULT      = IM_COL32(0, 160, 130, 255);
inline constexpr ImU32 NODO_DEFAULT_BORDE = IM_COL32(200, 220, 230, 200);

// ── Aristas (animación) ───────────────────────────────────────────────────
inline constexpr ImU32 ARISTA_CONFIRMADA  = IM_COL32(255, 179, 0, 255);
inline constexpr ImU32 ARISTA_EXPLORADA   = IM_COL32(0, 188, 212, 220);
inline constexpr ImU32 ARISTA_DESCARTADA  = IM_COL32(255, 68, 68, 255);
inline constexpr ImU32 ARISTA_DEFAULT     = IM_COL32(120, 130, 140, 120);

// ── Partículas de animación ───────────────────────────────────────────────
inline constexpr ImU32 PARTICULA_EXPLORAR  = IM_COL32(0, 200, 255, 255);
inline constexpr ImU32 PARTICULA_CONFIRMAR = IM_COL32(255, 180, 0, 255);
inline constexpr ImU32 PARTICULA_DESCARTAR = IM_COL32(255, 60, 60, 200);
inline constexpr ImU32 PARTICULA_DEFAULT   = IM_COL32(0, 255, 180, 255);

// ── Paquetes de simulación de red ─────────────────────────────────────────
inline constexpr ImU32 PAQUETE_PING  = IM_COL32(0, 255, 100, 220);
inline constexpr ImU32 PAQUETE_HTTP  = IM_COL32(80, 180, 255, 220);
inline constexpr ImU32 PAQUETE_VIDEO = IM_COL32(255, 150, 50, 220);
inline constexpr ImU32 PAQUETE_DDOS  = IM_COL32(255, 50, 50, 220);

// ── Hardware ──────────────────────────────────────────────────────────────
inline constexpr ImU32 BORDE_SERVIDOR = IM_COL32(0, 200, 150, 255);
inline constexpr ImU32 BORDE_ROUTER   = IM_COL32(255, 150, 0, 255);
inline constexpr ImU32 BORDE_SWITCH   = IM_COL32(77, 166, 255, 255);
inline constexpr ImU32 BORDE_FIREWALL = IM_COL32(255, 51, 51, 255);
inline constexpr ImU32 BORDE_TERMINAL = IM_COL32(179, 102, 255, 255);

// ── Paleta de coloreo greedy ──────────────────────────────────────────────
inline constexpr ImU32 COLOREO_PALETA[] = {
    IM_COL32(230, 60, 60, 255),   IM_COL32(60, 200, 80, 255),
    IM_COL32(60, 100, 230, 255),  IM_COL32(230, 200, 50, 255),
    IM_COL32(200, 60, 200, 255),  IM_COL32(60, 200, 200, 255)
};

// ── Isomorfismo G2 ────────────────────────────────────────────────────────
inline constexpr ImU32 G2_FONDO  = IM_COL32(180, 180, 0, 255);
inline constexpr ImU32 G2_BORDE  = IM_COL32(255, 255, 100, 255);

} // namespace Colores
