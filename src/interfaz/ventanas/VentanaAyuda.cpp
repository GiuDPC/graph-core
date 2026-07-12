#include "interfaz/ventanas/VentanaAyuda.hpp"
#include "interfaz/estado/EstadoUI.hpp"
#include "imgui.h"
#include "IconsFontAwesome6.h"

void VentanaAyuda::dibujar(EstadoUI& ui) {
    if (!ui.mostrar_ventana_ayuda) return;

    ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.11f, 0.98f));
    if (ImGui::Begin(ICON_FA_BOOK_OPEN " Enciclopedia GraphCore", &ui.mostrar_ventana_ayuda, ImGuiWindowFlags_NoCollapse)) {
        
        if (ImGui::BeginTable("split_ayuda", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("nav", ImGuiTableColumnFlags_WidthFixed, 220.0f);
            ImGui::TableSetupColumn("body", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::BeginChild("nav_ayuda", ImVec2(0, 0), false);
            
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "NAVEGACION PRINCIPAL");
            ImGui::Separator();
            ImGui::Spacing();

            struct SeccionInfo { const char* icono; const char* nombre; };
            SeccionInfo secciones[] = {
                { ICON_FA_HOUSE,            "Introduccion"         },
                { ICON_FA_KEYBOARD,         "Controles y Atajos"   },
                { ICON_FA_PLANE,            "Modo AeroGrafos"      },
                { ICON_FA_DIAGRAM_PROJECT,  "Modo Grafos Libres"   },
                { ICON_FA_MAGNET,           "Fisicas (ForceAtlas2)"},
                { ICON_FA_ROUTE,            "Dijkstra (Corto)"     },
                { ICON_FA_SITEMAP,          "Kruskal (MST)"        },
                { ICON_FA_CIRCLE_NODES,     "Euler y Hamilton"     },
                { ICON_FA_MAGNIFYING_GLASS, "Busquedas BFS/DFS"    },
                { ICON_FA_PAINTBRUSH,       "Coloreo (Greedy)"     },
                { ICON_FA_CLONE,            "Isomorfismo (VF2)"    },
                { ICON_FA_GLOBE,            "Analisis de Red"      },
                { ICON_FA_TABLE_CELLS,      "Matrices (Ady/Inc)"   },
                { ICON_FA_SHAPES,           "Plantillas de Grafos" },
                { ICON_FA_HIGHLIGHTER,      "Resaltado Vecinos"    },
            };
            int total = IM_ARRAYSIZE(secciones);

            for (int i = 0; i < total; i++) {
                char label[128];
                snprintf(label, sizeof(label), "%s %s", secciones[i].icono, secciones[i].nombre);
                bool sel = (ui.seccion_ayuda_actual == i);
                
                if (sel) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.6f, 0.5f, 0.4f));
                }
                if (ImGui::Selectable(label, sel)) {
                    ui.seccion_ayuda_actual = i;
                }
                if (sel) {
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();
            
            ImGui::TableNextColumn();
            ImGui::BeginChild("body_ayuda", ImVec2(0, 0), false, ImGuiChildFlags_AlwaysUseWindowPadding);
            
            dibujarContenido(ui.seccion_ayuda_actual);
            
            ImGui::EndChild();
            ImGui::EndTable();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void VentanaAyuda::dibujarTutorialRapido(EstadoUI& ui) {
    if (!ui.mostrar_tutorial_rapido) return;
    
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Tutorial Rapido", &ui.mostrar_tutorial_rapido, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.7f, 1.0f), ICON_FA_GRADUATION_CAP " Conceptos Básicos");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.5f, 1.0f), "Nodos y Aristas:");
        ImGui::BulletText("Clic derecho en el lienzo -> Crea un nodo");
        ImGui::BulletText("Clic derecho en nodo y soltar en otro -> Crea una arista");
        ImGui::BulletText("Clic derecho y soltar en el mismo nodo -> Crea un Bucle (Self-Loop)");
        ImGui::BulletText("Crear multiples aristas entre los mismos nodos -> Aristas Paralelas");
        ImGui::BulletText("Clic izquierdo en un nodo -> Selecciona y mueve");
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.5f, 1.0f), "Navegaci\u00f3n y Atajos:");
        ImGui::BulletText("Rueda del rat\u00f3n / Teclado (+/-) -> Zoom");
        ImGui::BulletText("Clic central / Clic izq. en vacio -> Paneos del lienzo (Mover c\u00e1mara)");
        ImGui::BulletText("Ctrl+Z -> Deshacer \u00faltima acci\u00f3n");
        ImGui::BulletText("Suprimir (Delete) / Backspace -> Borrar nodo seleccionado");
        
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Entendido", ImVec2(120, 0))) {
            ui.mostrar_tutorial_rapido = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else {
        ImGui::OpenPopup("Tutorial Rapido");
    }
}

void VentanaAyuda::titulo(const char* t) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.85f, 0.7f, 1.0f));
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextWrapped("%s", t);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
}

void VentanaAyuda::subtitulo(const char* t) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
    ImGui::TextWrapped("%s", t);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void VentanaAyuda::bullet_text_wrapped(const char* fmt) {
    ImGui::Bullet();
    ImGui::TextWrapped("%s", fmt);
}

void VentanaAyuda::tip(const char* t) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
    ImGui::TextWrapped(ICON_FA_LIGHTBULB " Tip Pro: %s", t);
    ImGui::PopStyleColor();
}

void VentanaAyuda::aviso(const char* t) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
    ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION " Atencion: %s", t);
    ImGui::PopStyleColor();
}

void VentanaAyuda::dibujarContenido(int sec) {
    switch (sec) {
    case 0: secIntroduccion(); break;
    case 1: secControles(); break;
    case 2: secAeroGrafos(); break;
    case 3: secGrafos(); break;
    case 4: secFisicas(); break;
    case 5: secDijkstra(); break;
    case 6: secKruskal(); break;
    case 7: secEuler(); break;
    case 8: secBFSDFS(); break;
    case 9: secColoreo(); break;
    case 10: secIsomorfismo(); break;
    case 11: secAnalizarRed(); break;
    case 12: secMatrices(); break;
    case 13: secPlantillas(); break;
    case 14: secResaltadoVecinos(); break;
    default: secIntroduccion(); break;
    }
}

void VentanaAyuda::secIntroduccion() {
    titulo(ICON_FA_HOUSE " Bienvenido a GraphCore");
    ImGui::TextWrapped("GraphCore es un entorno profesional avanzado para el analisis visual, simulacion termodinamica y resolucion de algoritmos sobre Grafos Matematicos y Redes Computacionales Globales.");
    ImGui::Spacing();
    
    subtitulo("El Enfoque Dual de GraphCore");
    ImGui::TextWrapped("El software se divide estructuralmente en dos motores independientes, seleccionables desde la barra superior:");
    bullet_text_wrapped("Modo Grafos Libres (Laboratorio Abstracto): Lienzo en blanco, sin restricciones fisicas. Aqui puedes crear desde cero cualquier estructura topologica, aplicar fuerzas fisicas (ForceAtlas2) para desenredarlos, buscar isomorfismos o aplicar coloreos.");
    bullet_text_wrapped("Modo AeroGrafos (Simulador Logistico Real): Carga una red fija de 63 aeropuertos mundiales reales usando coordenadas geograficas de satelite. Aplica distancias ortodromicas (curvatura terrestre) para encontrar rutas con Dijkstra o Kruskal, enfrentandote a crisis geopoliticas.");
    ImGui::Spacing();
    
    subtitulo("Organizacion de la Interfaz Visual");
    bullet_text_wrapped("Barra Superior (Toolbar): Contiene el switch maestro (Grafos/Redes), el boton para encender/apagar Fisicas (ForceAtlas2) y esta Enciclopedia.");
    bullet_text_wrapped("Panel Izquierdo (Inspector y Contexto): Muestra propiedades matematicas en tiempo real (Conexo, Euleriano, Bipartito). En el modo AeroGrafos, te permite activar Restricciones Geopoliticas.");
    bullet_text_wrapped("Lienzo Central (El Workspace): Tu area de dibujo o el mapa del mundo. Totalmente interactivo con el mouse.");
    bullet_text_wrapped("Consola Inferior (Logs): Un registro tecnico paso a paso de lo que la Inteligencia Artificial esta haciendo internamente.");
    bullet_text_wrapped("Panel Derecho (El Cerebro): Ahi se encuentran todas las herramientas y algoritmos (Dijkstra, Kruskal, etc).");
    
    tip("No tengas miedo de experimentar. Usa Ctrl+N para limpiar el lienzo cuando quieras empezar un nuevo experimento en el Modo Grafos Libres.");
}

void VentanaAyuda::secControles() {
    titulo(ICON_FA_KEYBOARD " Controles Maestros y Shortcuts");
    
    subtitulo("Manipulacion del Mouse (Workspace)");
    bullet_text_wrapped("Crear Nodo (Solo Modo Grafos): Clic Derecho en cualquier zona vacia del lienzo negro.");
    bullet_text_wrapped("Mover Nodo: Clic Izquierdo sostenido sobre un nodo + Arrastrar. Siente como las aristas elasticas lo siguen.");
    bullet_text_wrapped("Seleccionar Nodo: Un solo Clic Izquierdo sobre un nodo lo resalta. En el panel izquierdo veras su Grado (numero de conexiones).");
    bullet_text_wrapped("Conectar Nodos (Crear Arista): Manten Clic Derecho sobre el Nodo A y arrastra el laser neon hasta el Nodo B. Suelta el clic para soldarlos.");
    bullet_text_wrapped("Zoom Dinamico: Rueda del mouse (Scroll) hacia arriba o hacia abajo. El zoom sigue inteligentemente la posicion de tu cursor.");
    ImGui::Spacing();
    
    subtitulo("Teclado (Shortcuts de Alta Productividad)");
    bullet_text_wrapped("Supr / Delete: Elimina instantaneamente el nodo actualmente seleccionado y arranca de raiz todas las conexiones atadas a el.");
    bullet_text_wrapped("Ctrl + Z: Deshacer. Revierte la ultima accion topologica que realizaste (agregar/borrar nodos o aristas).");
    bullet_text_wrapped("Ctrl + Y: Rehacer. Restaura una accion previamente deshecha.");
    bullet_text_wrapped("Ctrl + N: Boton del Panico / Reset. Borra absolutamente toda la memoria del grafo libre actual.");
    bullet_text_wrapped("Teclas [+] y [-]: Alternativas de hardware para hacer Zoom si no tienes rueda de mouse.");
    subtitulo("Controles del Lienzo");
    bullet_text_wrapped("Shift + Arrastrar Nodo: Mueve todos los nodos del grafo a la vez.");
}

void VentanaAyuda::secAeroGrafos() {
    titulo(ICON_FA_PLANE " Modo AeroGrafos: Simulacion Terrestre Avanzada");
    
    ImGui::TextWrapped("El Modo AeroGrafos transforma GraphCore en un Centro de Control Logistico Global (ATC). El motor grafico proyecta la Tierra utilizando proyeccion equirectangular, mapeando 63 'Hubs' (los aeropuertos internacionales mas masivos del mundo).");
    ImGui::Spacing();
    
    subtitulo("Matematica de Curvatura (Formula del Semiverseno)");
    ImGui::TextWrapped("La Tierra no es plana. Si el algoritmo calculara la distancia usando linea recta 2D, el sistema fracasaria. Implementamos la formula Haversine (Ortodromica), que calcula la distancia mas corta sobre la superficie de una esfera usando radios de 6371km. Veras las rutas doblarse graciosamente; esto es la trayectoria balistica real (Gran Circulo).");
    ImGui::Spacing();
    
    subtitulo("Bloqueos Geopoliticos (La variable del caos)");
    ImGui::TextWrapped("Al abrir el Panel Izquierdo encontraras un Checkbox llamado 'Bloqueo Geopolitico (Rusia)'.");
    bullet_text_wrapped("Al activarlo, el programa inyecta un factor de multiplicacion extremo (peso penalizador) a TODAS las rutas de vuelo que crucen el poligono de conflicto (zona roja neon).");
    bullet_text_wrapped("Los aeropuertos Rusos se pinchan de color rojo y bloquean fisicamente tu interaccion. Si intentas seleccionarlos como origen/destino escucharas un sonido de error, simulando una sancion aerea total.");
    bullet_text_wrapped("Dijkstra desviara a los aviones por el polo norte o por el sur de asia automaticamente para evitar la zona de exclusion.");
    ImGui::Spacing();
    
    subtitulo("Guia Paso a Paso en la Interfaz");
    bullet_text_wrapped("1. Ve al Toolbar superior y asegurate que 'Modo Redes' este activo.");
    bullet_text_wrapped("2. En el panel izquierdo, activa el 'Modo Noche' para mejorar el contraste de los laseres neon.");
    bullet_text_wrapped("3. Activa el 'Bloqueo Geopolitico' para estresar los algoritmos.");
    bullet_text_wrapped("4. Ve al panel derecho y ejecuta Dijkstra o Kruskal para ver como resuelven la conectividad bajo estas restricciones.");
}

void VentanaAyuda::secGrafos() {
    titulo(ICON_FA_DIAGRAM_PROJECT " Modo Grafos Libres: El Lienzo Topologico");
    
    ImGui::TextWrapped("Este modo es un laboratorio matematico puro y aislado. No existen distancias en kilometros ni aeropuertos, solo Nodos abstractos y Aristas logicas. Su unico proposito es el estudio de la Topologia, la teoria de grafos profunda.");
    ImGui::Spacing();
    
    subtitulo("Dashboard Izquierdo (Inspector Topologico)");
    ImGui::TextWrapped("A diferencia del modo AeroGrafos, el panel izquierdo de este modo es un analizador matematico que se recalcula en microsegundos cada vez que agregas una arista o un nodo.");
    bullet_text_wrapped("Es Conexo?: Dice 'Si' unicamente si existe un camino (directo o indirecto) que vincule a todos los nodos del lienzo sin dejar islas.");
    bullet_text_wrapped("Es Euleriano?: Dice 'Si' si el grafo tiene 0 o exactamente 2 nodos con cantidad de conexiones impares (Requisito para trazarlo de un solo trazo sin repetir).");
    bullet_text_wrapped("Es Bipartito?: Dice 'Si' si la IA logro dividir tu red en dos equipos distintos sin que ningun miembro de un equipo se conecte con alguien de su mismo equipo (Carece de ciclos impares).");
    ImGui::Spacing();
    
    subtitulo("Densidad Porcentual");
    ImGui::TextWrapped("La barra de progreso de Densidad mide que tan conectada esta tu red respecto al limite fisico posible. Un grafo con 5 nodos y 10 aristas tiene 100% de densidad (Grafo Completo / K5), ya que no cabe ni una sola conexion mas.");
}

void VentanaAyuda::secFisicas() {
    titulo(ICON_FA_MAGNET " ForceAtlas2: Termodinamica de Grafos");
    
    ImGui::TextWrapped("ForceAtlas2 (FA2) no es un algoritmo estatico de inicio-fin. Es una simulacion continua de sistemas de particulas interactuando en el vacio. Se utiliza para desenredar automaticamente grafos espagueti caoticos, transformandolos en estructuras visuales intuitivas.");
    ImGui::Spacing();
    
    subtitulo("Teoria: Las Dos Fuerzas Universales");
    bullet_text_wrapped("Ley 1 (Repulsion de Coulomb): Todos los nodos se odian. Tratan de alejarse constantemente para no solaparse en el espacio.");
    bullet_text_wrapped("Ley 2 (Atraccion de Hooke): Las aristas actuan como resortes tensores elasticos. Atrapan y tiran de los nodos conectados hacia el centro.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz (Como activarlo y controlarlo)");
    bullet_text_wrapped("Boton 'Fisicas': En la barra superior, haz click en el icono de iman. El boton se iluminara en Verde brillante y el motor FA2 tomara el control animado.");
    bullet_text_wrapped("Interaccion Manual: MIENTRAS el boton esta verde, haz clic en un nodo y arrastralo bruscamente. Sentiras como los resortes luchan contra tu mouse.");
    bullet_text_wrapped("Memoria Flash: Apaga el boton 'Fisicas'. Todos los nodos del lienzo regresaran a la velocidad de la luz EXACTAMENTE a las posiciones donde los dibujaste originalmente. Esto te permite desenredar, observar y revertir sin danar tu trabajo.");
    
    aviso("Las Fisicas consumen GPU iterativa. El algoritmo ajusta automaticamente el Global Swing (enfriamiento termico) en grafos inmensos para no colapsar la PC.");
}

void VentanaAyuda::secDijkstra() {
    titulo(ICON_FA_ROUTE " Algoritmo de Dijkstra: GPS de Inteligencia Artificial");
    
    ImGui::TextWrapped("El Algoritmo Pathfinding mas famoso del mundo (1956). Dijkstra garantiza encontrar la ruta mas corta (menor costo acumulado) entre un Origen y un Destino.");
    ImGui::Spacing();
    
    subtitulo("Teoria de Relajacion");
    ImGui::TextWrapped("Dijkstra es como agua derramandose por calles. Al llegar a un nodo, examina todas sus salidas. Si el costo acumulado por una calle nueva es MENOR a lo que conocia previamente, borra el registro viejo y escribe este 'Atajo' (Relajacion de la arista).");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz (Pasos de Ejecucion)");
    bullet_text_wrapped("1. Abre el bloque 'Dijkstra' en el panel derecho.");
    bullet_text_wrapped("2. Selector de Origen: Haz clic en el primer Dropdown y busca tu punto de inicio.");
    bullet_text_wrapped("3. Selector de Destino: Haz clic en el segundo Dropdown y busca tu meta.");
    bullet_text_wrapped("4. Boton EJECUTAR: El sistema marcara la ruta ganadora con un trayecto Grueso y Brillante (Verde neon).");
    bullet_text_wrapped("5. Resultado: Debajo de 'Ejecutar', veras los kilometros totales o el peso numerico exacto de esa ruta.");
    bullet_text_wrapped("6. Boton LIMPIAR: Apaga las luces neon y devuelve el grafo a la normalidad.");
    
    tip("En Modo AeroGrafos: Prende el Bloqueo de Rusia y pidele a Dijkstra ir de Londres a Tokyo. Veras como desvia el avion por el Polo Norte (Alaska).");
}

void VentanaAyuda::secKruskal() {
    titulo(ICON_FA_SITEMAP " Kruskal: Arbol de Expansion Minima (MST)");
    
    ImGui::TextWrapped("Dijkstra es individualista (A hacia B). Kruskal, en cambio, busca unificar TODA LA RED pavimentando la minima cantidad de calles y gastando la menor cantidad de recursos (Costo total minimo).");
    ImGui::Spacing();
    
    subtitulo("Teoria y Union-Find (Prevencion de Ciclos)");
    ImGui::TextWrapped("El resultado de Kruskal siempre es un 'Arbol' (red sin anillos/ciclos).");
    bullet_text_wrapped("Ordena todas las aristas de la mas barata a la mas cara.");
    bullet_text_wrapped("Intenta soldar la red empezando siempre por la calle mas barata.");
    bullet_text_wrapped("Union-Find: Antes de soldar A y B, pregunta: '¿Ya estan conectados por otro lado?'. Si la respuesta es Si, desecha la arista porque generaria un ciclo inutil.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz (La Animacion es clave)");
    bullet_text_wrapped("1. Abre 'Kruskal' en el panel derecho.");
    bullet_text_wrapped("2. Checkbox 'Animacion' (CRITICO): Activalo! El poder de Kruskal se aprecia visualmente.");
    bullet_text_wrapped("3. Ejecutar: El motor evaluara arista por arista. Las aprobadas se pintan de VERDE NEON. Las que generan un ciclo indeseado se pintan un milisegundo de ROJO SANGRE y son descartadas.");
    bullet_text_wrapped("4. Resultado: El costo total del Asfalto (peso) se mostrara en el panel. Usa 'Limpiar MST' para borrar el rastro.");
}

void VentanaAyuda::secBFSDFS() {
    titulo(ICON_FA_MAGNIFYING_GLASS " Busquedas Topologicas: BFS y DFS");
    
    ImGui::TextWrapped("Dos paradigmas mentales opuestos para explorar territorios desconocidos.");
    ImGui::Spacing();
    
    subtitulo("BFS (Anchura) - La Onda de Choque");
    bullet_text_wrapped("Estrategia Radial: Avanza como ondas en un estanque. Revisa primero todo el Nivel 1, luego el Nivel 2.");
    bullet_text_wrapped("Superpoder: Es el algoritmo supremo para encontrar el camino con la MENOR CANTIDAD DE ESCALAS (saltos), ignorando el kilometraje.");
    ImGui::Spacing();
    
    subtitulo("DFS (Profundidad) - El Hilo de Ariadna");
    bullet_text_wrapped("Estrategia Suicida: Corre ciegamente por un pasillo hasta chocar con un callejon sin salida. Luego recula (Backtracking) y busca otra rama.");
    bullet_text_wrapped("Uso: Ideal para detectar ciclos infinitos en dependencias y realizar Ordenamientos Topologicos.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. Abre 'Busquedas' en el panel derecho.");
    bullet_text_wrapped("2. Selector de Estrategia: Elige 'BFS (Anchura)' o 'DFS (Profundidad)'.");
    bullet_text_wrapped("3. Nodo Inicial: Elige de donde va a salir el rayo escaneador.");
    bullet_text_wrapped("4. Toggle 'Animado': Activalo obligatoriamente. Veras a la IA pensar paso a paso.");
    bullet_text_wrapped("5. Ejecutar: Compara visualmente la onda radial expansiva de BFS contra el rayo zigzagueante y caotico de DFS en color Cian.");
}

void VentanaAyuda::secColoreo() {
    titulo(ICON_FA_PAINTBRUSH " Coloreo (Heuristica Voraz / Greedy)");
    
    ImGui::TextWrapped("Las reglas: Ningun par de nodos conectados por una arista puede tener el mismo color. El objetivo es usar la menor cantidad posible de colores (Numero Cromatico).");
    ImGui::Spacing();
    
    subtitulo("Teoria: Greedy (Voraz)");
    ImGui::TextWrapped("GraphCore revisa a los vecinos de un nodo: 'Si no usan Rojo, te pinto Rojo. Si ya lo usan, pruebo Azul. Si todos estan ocupados, invento Verde'. Es local (no planea a futuro), pero es veloz.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz (Modo Grafos Libres)");
    bullet_text_wrapped("1. Abre el panel de 'Coloreo (Greedy)' a la derecha.");
    bullet_text_wrapped("2. Velocidad de Animacion (Slider): Izquierda es lentisimo (para estudiar), derecha es instantaneo.");
    bullet_text_wrapped("3. Ejecutar: El lienzo mostrara cajas de colores neon (Naranja, Cian, Violeta, Verde) asignandose a los nodos.");
    bullet_text_wrapped("4. Resultado: Debajo del boton, te dira 'Colores Usados: X'. Intenta crear un cuadrado con una diagonal cruzada y observa cuantas cubetas necesita.");
}

void VentanaAyuda::secEuler() {
    titulo(ICON_FA_CIRCLE_NODES " Euler & Hamilton: Circuitos");
    
    ImGui::TextWrapped("La dualidad de los recorridos matematicos.");
    ImGui::Spacing();
    
    subtitulo("Camino Euleriano: La Perfeccion de las Aristas");
    bullet_text_wrapped("Mision: Caminar por TODAS las aristas del mapa sin repetir ninguna.");
    bullet_text_wrapped("Regla Dorada: Solo es posible si TODO nodo tiene un grado par, o si hay exactamente 2 nodos de grado impar. La IA de GraphCore lo sabe al instante comprobando los grados.");
    ImGui::Spacing();
    
    subtitulo("Circuito Hamiltoniano: El Terror NP");
    bullet_text_wrapped("Mision: Tocar TODOS los nodos exactamente una vez y regresar al inicio.");
    bullet_text_wrapped("Dificultad: Es el Problema del Agente Viajero. La IA tiene que intentarlo a la fuerza bruta (Backtracking). Requiere altisimo poder de CPU.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. Abre 'Recorridos Euler/Hamilton' en el panel derecho.");
    bullet_text_wrapped("2. Selecciona 'Camino Euleriano' o 'Camino Hamiltoniano'.");
    bullet_text_wrapped("3. Ejecutar: Si pides Euler y fallas las reglas pares/impares, imprimira un error rojo de inmediato. Si pides Hamilton en grafos medianos, veras a la linea verde recorrer obsesivamente cada permutacion posible.");
}

void VentanaAyuda::secIsomorfismo() {
    titulo(ICON_FA_CLONE " Isomorfismo (VF2): Gemelos Estructurales");
    
    ImGui::TextWrapped("Dos redes que, aunque esten dibujadas diferente, poseen la EXACTA misma topologia de conexiones (Biyeccion perfecta).");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz (El Modo Edicion Dual)");
    bullet_text_wrapped("1. Entorno: Modo Grafos Libres. Dibuja el Grafo 1 (G1) en el centro de la pantalla (ej. un Cuadrado).");
    bullet_text_wrapped("2. El Cambio de Fase: Ve al panel 'Isomorfismo' a la derecha. Enciende el interruptor 'Editar G2 (Grafo de Pruebas)'.");
    bullet_text_wrapped("3. Magia Visual: La interfaz y etiquetas cambiaran a Violeta. G1 esta a salvo en memoria. Ahora todo lo que dibujes es G2. Dibuja un cuadrado aplastado a un costado.");
    bullet_text_wrapped("4. Formas Geometricas: En G2 tendras botones especiales para generar figuras aleatorias o Forzar Poligonos Regulares basados en la cantidad de nodos. Esto es ideal para demostrar como un grafo feo puede ser isomorfo a un hexagono perfecto.");
    bullet_text_wrapped("5. Veredicto: Presiona 'COMPROBAR ISOMORFISMO'. Si hay una diferencia de nodos, el programa abortara y te indicara la discrepancia matematica.");
    bullet_text_wrapped("6. Resultado: Recibiras un cartel verde confirmatorio si ambos son identicos, mostrandote el mapeo exacto de nodos (Ej. V0 = U2).");
}

void VentanaAyuda::secMatrices() {
    titulo(ICON_FA_TABLE_CELLS " Matrices: El ADN Algebraico");
    
    ImGui::TextWrapped("Las computadoras leen Tablas Matem\u00e1ticas bidimensionales llenas de Ceros y Unos. Esto es la teoria Algebraica.");
    ImGui::Spacing();
    
    subtitulo("Matriz de Adyacencia (NxN)");
    bullet_text_wrapped("Mapea Nodos vs Nodos.");
    bullet_text_wrapped("Si existe una conexion entre Nodo 1 y Nodo 3, en la celda [1][3] habra un '1' (o el peso). Si no, un '0'.");
    bullet_text_wrapped("La diagonal principal siempre tiene ceros, porque un nodo no se conecta a si mismo.");
    ImGui::Spacing();
    
    subtitulo("Matriz de Incidencia (NxM)");
    bullet_text_wrapped("Mapea Nodos vs Aristas.");
    bullet_text_wrapped("En cualquier columna, SIEMPRE habra unicamente dos '1's. Estos marcan los dos extremos de la arista.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. En la zona central superior del Lienzo, veras una Pestana llamada 'Matrices'.");
    bullet_text_wrapped("2. Al hacer clic, estas gigantescas tablas se redibujaran automaticamente en tiempo real mientras agregas nodos o aristas en la otra pestana.");
}

void VentanaAyuda::secAnalizarRed() {
    titulo(ICON_FA_GLOBE " Analisis de Red (Escaner de Vulnerabilidad)");
    
    ImGui::TextWrapped("No es un algoritmo, es una suite de auditoria de Defensa e Inteligencia que escanea el mapa para evaluar vulnerabilidades logicas.");
    ImGui::Spacing();
    
    subtitulo("Las Tres Metricas");
    bullet_text_wrapped("El Super-Hub Principal (SPOF): El nodo con mayor conectividad. Si cae, la red colapsa. El motor grafico lo hara parpadear en ROJO ELECTRICO.");
    bullet_text_wrapped("Cuello de Botella: Nodos con grado Minimo. Si envias datos aqui, se atascaran.");
    bullet_text_wrapped("Densidad Ponderada: Porcentaje matematico de conectividad real vs teorica.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. Excelente para usar en Modo AeroGrafos.");
    bullet_text_wrapped("2. Abre el modulo 'Analisis de Red' en el panel derecho.");
    bullet_text_wrapped("3. Presiona 'Analizar Toda la Red'.");
    bullet_text_wrapped("4. El Super-Hub latira con un radar rojo en el lienzo. El panel de resultados imprimira el diagnostico exacto.");
}

void VentanaAyuda::secPlantillas() {
    titulo(ICON_FA_SHAPES " Plantillas de Grafos");
    
    ImGui::TextWrapped("GraphCore incluye generadores instantaneos para crear estructuras clasicas de grafos con un solo clic. Ideal para estudiar propiedades topologicas sin tener que dibujar nodo por nodo.");
    ImGui::Spacing();
    
    subtitulo("Plantillas Disponibles");
    bullet_text_wrapped("Grafo Completo (Kn): Todos los nodos conectados entre si. Densidad 100%%.");
    bullet_text_wrapped("Ciclo (Cn): Anillo simple donde cada nodo tiene exactamente grado 2.");
    bullet_text_wrapped("Malla (Grid): Cuadricula rectangular. Util para pathfinding y Dijkstra.");
    bullet_text_wrapped("Arbol Binario: Estructura jerarquica de profundidad configurable.");
    bullet_text_wrapped("Estrella (Star): Un hub central conectado a N hojas.");
    bullet_text_wrapped("Watts-Strogatz: Red de mundo pequeno con rewiring aleatorio.");
    bullet_text_wrapped("Petersen: Grafo clasico de teoria de grafos, 3-regular, no-planar.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. Abre el panel derecho y selecciona 'Plantillas' en el menu desplegable.");
    bullet_text_wrapped("2. Elige una plantilla y configura sus parametros (numero de nodos, profundidad, etc).");
    bullet_text_wrapped("3. Presiona 'Generar'. El grafo aparecera en el lienzo listo para analizar.");
    
    tip("Combina plantillas con ForceAtlas2 para ver como las fisicas desenredan la estructura automaticamente.");
}

void VentanaAyuda::secResaltadoVecinos() {
    titulo(ICON_FA_HIGHLIGHTER " Resaltado por Adyacencia (Adjacency Highlighting)");
    
    ImGui::TextWrapped("Efecto visual inspirado en Gephi. Al pasar el mouse sobre un nodo, se resaltan el nodo y todos sus vecinos directos, mientras el resto del grafo se atenua. Permite explorar visualmente la estructura local de un grafo de forma intuitiva.");
    ImGui::Spacing();
    
    subtitulo("Como Funciona");
    bullet_text_wrapped("El nodo bajo el cursor y sus vecinos directos mantienen opacidad completa.");
    bullet_text_wrapped("Las aristas que conectan al nodo con sus vecinos se resaltan con mayor grosor y brillo.");
    bullet_text_wrapped("Los nodos y aristas no conectados se atenuan (alpha reducido) para crear contraste visual.");
    bullet_text_wrapped("El efecto se desactiva automaticamente durante animaciones de algoritmos (BFS, DFS) y coloreo para no interferir.");
    ImGui::Spacing();
    
    subtitulo("Guia de Interfaz");
    bullet_text_wrapped("1. En el panel izquierdo 'Info del Grafo', activa el checkbox 'Resaltado Vecinos'.");
    bullet_text_wrapped("2. Pasa el mouse sobre cualquier nodo del lienzo.");
    bullet_text_wrapped("3. Observa como se iluminan solo el nodo y sus conexiones directas.");
    bullet_text_wrapped("4. Desactiva el checkbox para volver al comportamiento normal.");
    
    tip("Usa Resaltado Vecinos junto con Ranking Visual (tamano por grado) para identificar rapidamente los hubs mas importantes de tu red.");
}
