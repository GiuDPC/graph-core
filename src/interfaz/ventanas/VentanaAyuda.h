#pragma once

#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "interfaz/estado/EstadoUI.h"

struct VentanaAyuda {
    static void dibujar(EstadoUI& ui) {
        if (!ui.mostrar_ventana_ayuda) return;

        ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.11f, 0.98f));
        if (ImGui::Begin(ICON_FA_BOOK_OPEN " Enciclopedia OptiClusters", &ui.mostrar_ventana_ayuda, ImGuiWindowFlags_NoCollapse)) {
            
            if (ImGui::BeginTable("split_ayuda", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("nav", ImGuiTableColumnFlags_WidthFixed, 210.0f);
                ImGui::TableSetupColumn("body", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                
                ImGui::TableNextColumn();
                ImGui::BeginChild("nav_ayuda", ImVec2(0, 0), false);
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "NAVEGACION");
                ImGui::Separator();
                ImGui::Spacing();

                struct SeccionInfo { const char* icono; const char* nombre; };
                SeccionInfo secciones[] = {
                    { ICON_FA_HOUSE,            "Introduccion"         },
                    { ICON_FA_KEYBOARD,         "Controles y Atajos"   },
                    { ICON_FA_DIAGRAM_PROJECT,  "Modo Grafos"          },
                    { ICON_FA_PLANE,            "Modo AeroGrafos"      },
                    { ICON_FA_MAGNET,           "Fisicas (FR)"         },
                    { ICON_FA_PAINTBRUSH,       "Coloreo de Grafos"    },
                    { ICON_FA_CLONE,            "Isomorfismo"          },
                    { ICON_FA_ROUTE,            "Dijkstra"             },
                    { ICON_FA_SITEMAP,          "Kruskal (MST)"        },
                    { ICON_FA_CIRCLE_NODES,     "Euler y Hamilton"     },
                    { ICON_FA_MAGNIFYING_GLASS, "BFS y DFS"            },
                    { ICON_FA_CIRCLE_HALF_STROKE, "Bipartito"          },
                    { ICON_FA_TABLE_CELLS,      "Matrices"             },
                    { ICON_FA_SLIDERS,          "Panel de Info"        },
                    { ICON_FA_CHART_LINE,       "Dashboard y Extras"   },
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
                ImGui::BeginChild("body_ayuda", ImVec2(0, 0), false);
                ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x - 15.0f);
                
                dibujarContenido(ui.seccion_ayuda_actual);
                
                ImGui::PopTextWrapPos();
                ImGui::EndChild();
                ImGui::EndTable();
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

private:
    static void titulo(const char* t) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.85f, 0.7f, 1.0f));
        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("%s", t);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
    }
    static void subtitulo(const char* t) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
        ImGui::Text("%s", t);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    static void tip(const char* t) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
        ImGui::Text(ICON_FA_LIGHTBULB " Tip: %s", t);
        ImGui::PopStyleColor();
    }
    static void aviso(const char* t) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
        ImGui::Text(ICON_FA_TRIANGLE_EXCLAMATION " %s", t);
        ImGui::PopStyleColor();
    }

    static void dibujarContenido(int sec) {
        switch (sec) {
        case 0: secIntroduccion(); break;
        case 1: secControles(); break;
        case 2: secGrafos(); break;
        case 3: secRedes(); break;
        case 4: secFisicas(); break;
        case 5: secColoreo(); break;
        case 6: secIsomorfismo(); break;
        case 7: secDijkstra(); break;
        case 8: secKruskal(); break;
        case 9: secEuler(); break;
        case 10: secBFSDFS(); break;
        case 11: secBipartito(); break;
        case 12: secMatrices(); break;
        case 13: secPanelInfo(); break;
        case 14: secSimulacion(); break;
        default: secIntroduccion(); break;
        }
    }

    // secciones
    
    static void secIntroduccion() {
        titulo(ICON_FA_HOUSE " Bienvenido a OptiClusters");
        ImGui::Text("OptiClusters es una herramienta profesional de analisis visual para Grafos Matematicos y Redes Computacionales. Permite crear, manipular y analizar estructuras de datos usando algoritmos clasicos de la teoria de grafos.");
        ImGui::Spacing();
        
        subtitulo("Que puedes hacer aqui?");
        ImGui::BulletText("Crear grafos interactivos dibujando nodos y conexiones directamente en el lienzo.");
        ImGui::BulletText("Ejecutar algoritmos clasicos: Dijkstra, Kruskal, Coloreo, Isomorfismo, BFS, DFS, Euler.");
        ImGui::BulletText("Simular redes de computadoras con latencias, jitter y perdida de paquetes.");
        ImGui::BulletText("Visualizar matrices de adyacencia e incidencia en tiempo real.");
        ImGui::BulletText("Activar fisicas para que el grafo se organice solo de forma armonica.");
        ImGui::Spacing();
        
        subtitulo("Dos modos de trabajo");
        ImGui::BulletText("Modo Grafos: Analisis matematico puro en un lienzo libre. Ideal para estudiar propiedades topologicas (isomorfismo, coloreo, ciclos).");
        ImGui::BulletText("Modo AeroGrafos: Simulacion global del espacio aereo. 63 hubs mundiales reales con distancias haversine. Ideal para Dijkstra, Kruskal y analisis de densidad.");
        ImGui::Spacing();
        
        subtitulo("Arquitectura del programa");
        ImGui::Text("La interfaz se divide en tres zonas principales:");
        ImGui::BulletText("Panel Izquierdo: Informacion del grafo actual (nodos, aristas, densidad, propiedades).");
        ImGui::BulletText("Centro: Lienzo interactivo donde dibujas y manipulas el grafo. Tambien tiene pestanas para ver las Matrices y el Registro del Kernel (logs internos).");
        ImGui::BulletText("Panel Derecho: Algoritmos. Aqui seleccionas y configuras el algoritmo que quieras ejecutar.");
        ImGui::Spacing();
        
        tip("Usa la barra de zoom en la esquina inferior derecha del lienzo para acercarte o alejarte. Tambien puedes usar la rueda del mouse o las teclas +/-.");
    }

    static void secControles() {
        titulo(ICON_FA_KEYBOARD " Controles y Atajos de Teclado");
        
        subtitulo("Mouse");
        ImGui::BulletText("Clic Izquierdo sobre un nodo: Selecciona el nodo. Por si haces click en otro nodo, se abre el menu de conexion.");
        ImGui::BulletText("Clic Izquierdo + Arrastrar: Mueve un nodo existente por el lienzo.");
        ImGui::BulletText("Clic Derecho (en espacio vacio): Crea un nuevo nodo en esa posicion.");
        ImGui::BulletText("Clic Derecho + Arrastrar (desde un nodo): Activa el laser neon para conectar dos nodos visualmente. Suelta sobre otro nodo para crear la arista.");
        ImGui::BulletText("Rueda del Mouse: Zoom in / zoom out centrado en el cursor.");
        ImGui::Spacing();
        
        subtitulo("Teclado");
        ImGui::BulletText("Ctrl + N: Borrar todo el grafo (limpieza completa del lienzo).");
        ImGui::BulletText("+ / -: Zoom in / zoom out.");
        ImGui::BulletText("Supr (Delete): Eliminar el nodo seleccionado y todas sus conexiones.");
        ImGui::Spacing();
        
        subtitulo("Barra Superior (Toolbar)");
        ImGui::BulletText("Grafos / Redes: Cambia el modo de trabajo del programa.");
        ImGui::BulletText("Enciclopedia: Abre esta ventana de documentacion.");
        ImGui::BulletText("Fisicas (ON/OFF): Activa o desactiva la simulacion de fuerzas. Al desactivar, los nodos vuelven a sus posiciones originales.");
        ImGui::Spacing();
        
        subtitulo("Barra de Zoom (esquina inferior derecha)");
        ImGui::BulletText("Lupa (-): Alejar la vista.");
        ImGui::BulletText("Casita: Resetear el zoom a su valor predeterminado.");
        ImGui::BulletText("Lupa (+): Acercar la vista.");
        ImGui::Spacing();
        
        tip("Si las fisicas estan activas y apagas el boton, los nodos regresan a donde estaban antes de activarlas. Util para comparar el antes y despues.");
    }

    static void secGrafos() {
        titulo(ICON_FA_DIAGRAM_PROJECT " Modo Grafos: Fundamentos");
        
        ImGui::Text("En matematicas, un grafo G = (V, E) es una estructura compuesta por un conjunto de vertices (V) y un conjunto de aristas (E) que representan conexiones entre pares de vertices.");
        ImGui::Spacing();
        
        subtitulo("Conceptos clave");
        ImGui::BulletText("Vertice (Nodo): Un punto que representa una entidad. En la app se muestra como un circulo con un nombre (V0, V1...).");
        ImGui::BulletText("Arista (Conexion): Una linea que une dos vertices, indicando una relacion entre ellos.");
        ImGui::BulletText("Grado de un nodo: La cantidad de aristas que tocan a ese nodo. Un nodo aislado tiene grado 0.");
        ImGui::BulletText("Grafo Conexo: Un grafo donde puedes llegar de cualquier nodo a cualquier otro siguiendo aristas.");
        ImGui::BulletText("Densidad: Proporcion entre aristas existentes y el maximo posible. Un grafo completo tiene densidad 100%%.");
        ImGui::Spacing();
        
        subtitulo("Como crear un grafo en la app");
        ImGui::Text("1. Asegurate de estar en modo Grafos (boton superior izquierdo).");
        ImGui::Text("2. Haz clic derecho en el lienzo para crear nodos.");
        ImGui::Text("3. Para conectar: arrastra con clic derecho desde un nodo hacia otro (laser neon).");
        ImGui::Text("4. Para eliminar: selecciona un nodo con clic izquierdo y presiona Supr.");
        ImGui::Spacing();
        
        subtitulo("Diferencia con Modo AeroGrafos");
        ImGui::Text("En Modo Grafos el interes es puramente topologico. En AeroGrafos el interes es geografico y analitico sobre datos fijos mundiales.");
        ImGui::Spacing();
        
        tip("El panel izquierdo te muestra en tiempo real si tu grafo es Conexo, Bipartito o Euleriano.");
    }

    static void secRedes() {
        titulo(ICON_FA_PLANE " Modo AeroGrafos: Simulador Geopolitico");
        
        ImGui::Text("En este modo, el motor grafico despliega un mapa equirectangular mundial con 63 de los hubs aereos mas importantes del planeta (aeropuertos principales). Las aristas son rutas comerciales reales.");
        ImGui::Spacing();
        
        subtitulo("Conceptos clave");
        ImGui::BulletText("Distancia Haversine: Las distancias entre aeropuertos se calculan tomando en cuenta la curvatura de la Tierra (kilometros reales).");
        ImGui::BulletText("Rutas Ortodromicas: Las lineas en el mapa se curvan naturalmente simulando las rutas de vuelo optimas (Great Circle).");
        ImGui::Spacing();
        
        subtitulo("Como usarlo");
        ImGui::Text("1. Cambia a modo AeroGrafos (boton superior).");
        ImGui::Text("2. Selecciona un algoritmo en el panel derecho.");
        ImGui::Text("3. Elige los aeropuertos de origen y destino.");
        ImGui::Text("4. Haz clic en 'Ejecutar' (con o sin animacion).");
        ImGui::Spacing();
        
        subtitulo("Algoritmos disponibles");
        ImGui::BulletText("Dijkstra: Encuentra la ruta con menor kilometraje real.");
        ImGui::BulletText("Kruskal: Conecta todos los aeropuertos mundiales gastando la menor cantidad de asfalto/combustible.");
        ImGui::BulletText("Coloreo: Asigna aerolineas (colores) evitando monopolios locales.");
        ImGui::Spacing();
        
        tip("En modo AeroGrafos, la interaccion manual es de visualizacion (pan y zoom). Los nodos son fijos y representan la base de datos mundial.");
    }

    static void secFisicas() {
        titulo(ICON_FA_MAGNET " Fisicas: Algoritmo Fruchterman-Reingold");
        
        ImGui::Text("El algoritmo de Fruchterman y Reingold (1991) es un metodo de fuerza dirigida para dibujar grafos. Simula fuerzas fisicas para encontrar automaticamente una disposicion visual armonica de los nodos.");
        ImGui::Spacing();
        
        subtitulo("Dos fuerzas fundamentales");
        ImGui::Text("1. REPULSION (Ley de Coulomb):");
        ImGui::Text("   Todos los nodos se repelen entre si como cargas electricas del mismo signo. Esto evita que se amontonen.");
        ImGui::Spacing();
        ImGui::Text("2. ATRACCION (Ley de Hooke):");
        ImGui::Text("   Cada arista actua como un resorte que tira de sus dos nodos, acercandolos. Esto agrupa a los nodos que estan conectados.");
        ImGui::Spacing();
        
        subtitulo("Que resultado produce?");
        ImGui::Text("El sistema siempre busca un estado de MINIMA ENERGIA, donde todas las fuerzas se cancelan entre si. Esto produce figuras perfectamente equilibradas y simetricas:");
        ImGui::BulletText("Un ciclo de N nodos se dibuja como un poligono regular perfecto (circulo).");
        ImGui::BulletText("Un grafo completo se dibuja como un poligono con todas las diagonales visibles.");
        ImGui::BulletText("Nodos aislados (sin conexiones) son expulsados hacia los bordes por la repulsion.");
        ImGui::BulletText("Clusters (grupos densamente conectados) se agrupan visualmente, revelando comunidades ocultas.");
        ImGui::Spacing();
        
        subtitulo("Utilidad practica");
        ImGui::BulletText("Desenredar grafos desordenados automaticamente.");
        ImGui::BulletText("Descubrir simetrias ocultas en la estructura.");
        ImGui::BulletText("Revelar clusters y comunidades.");
        ImGui::Spacing();
        
        subtitulo("Como probarlo paso a paso");
        ImGui::Text("Prueba 1 - Desenredo:");
        ImGui::Text("  a) Desactiva las fisicas.");
        ImGui::Text("  b) Crea 8 nodos y conectalos en un circulo (V0-V1, V1-V2, ..., V7-V0).");
        ImGui::Text("  c) Arrastra los nodos desordenadamente.");
        ImGui::Text("  d) Activa las fisicas y observa como se forma un octagono perfecto.");
        ImGui::Spacing();
        ImGui::Text("Prueba 2 - Nodo aislado:");
        ImGui::Text("  a) Con las fisicas activas, crea un nodo SIN conectarlo a nada.");
        ImGui::Text("  b) Observa como es empujado hacia afuera (solo recibe repulsion, no atraccion).");
        ImGui::Spacing();
        ImGui::Text("Prueba 3 - Clusters:");
        ImGui::Text("  a) Crea dos grupos de 4 nodos, cada grupo completamente interconectado.");
        ImGui::Text("  b) Conecta los dos grupos con UNA sola arista.");
        ImGui::Text("  c) Activa las fisicas: veras dos agrupaciones densas unidas por un puente.");
        ImGui::Spacing();
        
        tip("Al desactivar las fisicas, los nodos vuelven a su posicion original (antes de activarlas).");
    }

    static void secColoreo() {
        titulo(ICON_FA_PAINTBRUSH " Algoritmo: Coloreo de Grafos");
        
        ImGui::Text("El problema del coloreo consiste en asignar un color a cada nodo del grafo de forma que dos nodos adyacentes (conectados por una arista) NUNCA compartan el mismo color, usando la menor cantidad de colores posible.");
        ImGui::Spacing();
        
        subtitulo("Numero Cromatico");
        ImGui::Text("El numero cromatico X(G) es la menor cantidad de colores necesaria para colorear el grafo correctamente. Calcularlo exactamente es un problema NP-completo (extremadamente dificil para grafos grandes). OptiClusters usa un algoritmo heuristico voraz (greedy) que da resultados muy buenos en la practica.");
        ImGui::Spacing();
        
        subtitulo("Como funciona el algoritmo (Greedy)");
        ImGui::Text("1. Se toman los nodos uno por uno.");
        ImGui::Text("2. Para cada nodo, se miran los colores de todos sus vecinos.");
        ImGui::Text("3. Se le asigna el color con el indice mas bajo que NO este en uso por ningun vecino.");
        ImGui::Text("4. Si todos los colores existentes estan ocupados, se crea un color nuevo.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones en la vida real");
        ImGui::BulletText("Asignacion de frecuencias de radio: dos antenas cercanas no pueden usar la misma frecuencia o se interfieren.");
        ImGui::BulletText("Programacion de examenes: un alumno no puede tener dos examenes a la misma hora.");
        ImGui::BulletText("Compiladores: asignar registros del procesador a variables (register allocation).");
        ImGui::BulletText("Mapas politicos: colorear paises de forma que dos paises fronterizos no compartan color.");
        ImGui::Spacing();
        
        subtitulo("Como usarlo en la app");
        ImGui::Text("1. Selecciona el algoritmo 'Coloreo' en el panel derecho.");
        ImGui::Text("2. Haz clic en 'Aplicar Color'.");
        ImGui::Text("3. Los nodos se tintan y puedes ver cuantos colores se necesitaron.");
        ImGui::Text("4. El boton 'Aristas' anima paso a paso como el algoritmo recorre el grafo.");
        ImGui::Spacing();
        
        tip("Un grafo bipartito siempre se puede colorear con exactamente 2 colores. Si el coloreo te da 2, comprueba si tu grafo es bipartito en el panel izquierdo.");
    }

    static void secIsomorfismo() {
        titulo(ICON_FA_CLONE " Algoritmo: Isomorfismo de Grafos");
        
        ImGui::Text("Dos grafos G1 y G2 son isomorfos si existe una forma de renombrar los nodos de G1 para obtener exactamente G2. En otras palabras, tienen la misma estructura de conexiones aunque se vean dibujados de forma diferente.");
        ImGui::Spacing();
        
        subtitulo("Por que es importante?");
        ImGui::Text("El isomorfismo permite reconocer que dos representaciones aparentemente distintas son el mismo objeto matematico. Es como reconocer que dos caras de una misma moneda son 'la misma moneda'.");
        ImGui::Spacing();
        
        subtitulo("Que verifica el algoritmo");
        ImGui::BulletText("Mismo numero de nodos.");
        ImGui::BulletText("Mismo numero de aristas.");
        ImGui::BulletText("Misma secuencia de grados (ordenados).");
        ImGui::BulletText("Existencia de un mapeo biyectivo que preserve las conexiones.");
        ImGui::Spacing();
        
        subtitulo("Como usarlo en la app");
        ImGui::Text("1. Selecciona 'Isomorfismo' en el panel derecho.");
        ImGui::Text("2. Activa 'Editar G2' para dibujar un segundo grafo.");
        ImGui::Text("3. Haz clic en 'Analizar'. El resultado te dira si son isomorfos o no, y mostrara el mapeo encontrado.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones");
        ImGui::BulletText("Reconocimiento de patrones en quimica (dos moleculas con la misma estructura).");
        ImGui::BulletText("Verificacion de circuitos digitales (dos circuitos son equivalentes?).");
        ImGui::BulletText("Biologia computacional (comparar redes de proteinas).");
        ImGui::Spacing();
        
        tip("Si los dos grafos tienen diferente numero de nodos o aristas, son AUTOMATICAMENTE no isomorfos.");
    }

    static void secDijkstra() {
        titulo(ICON_FA_ROUTE " Algoritmo: Dijkstra (Ruta Mas Corta)");
        
        ImGui::Text("El algoritmo de Dijkstra (1956) encuentra el camino con menor peso total desde un nodo origen hasta un nodo destino. Es el algoritmo que usan los GPS, Google Maps y los routers de Internet.");
        ImGui::Spacing();
        
        subtitulo("Como funciona (paso a paso)");
        ImGui::Text("1. Se marca el nodo origen con distancia 0 y todos los demas con distancia infinita.");
        ImGui::Text("2. Se visita el nodo con menor distancia acumulada.");
        ImGui::Text("3. Para cada vecino del nodo actual, se calcula: distancia_actual + peso_arista.");
        ImGui::Text("4. Si ese nuevo costo es menor que la distancia que ya tenia, se actualiza (relajacion: reemplazar un camino por uno mas optimo).");
        ImGui::Text("5. Se repite hasta llegar al destino o visitar todos los nodos.");
        ImGui::Spacing();
        
        subtitulo("Requisitos");
        ImGui::BulletText("Todos los pesos de las aristas deben ser NO NEGATIVOS (>= 0).");
        ImGui::BulletText("Si hay pesos negativos, Dijkstra puede dar resultados incorrectos.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones en la vida real");
        ImGui::BulletText("Navegacion GPS: encontrar la ruta mas rapida entre dos ciudades.");
        ImGui::BulletText("Enrutamiento IP: los routers OSPF usan Dijkstra para calcular tablas de enrutamiento.");
        ImGui::BulletText("Juegos: pathfinding (A* es una variante de Dijkstra con heuristica).");
        ImGui::BulletText("Logistica: minimizar costos de transporte.");
        ImGui::Spacing();
        
        subtitulo("Como usarlo en la app");
        ImGui::Text("1. En AeroGrafos, selecciona 'Dijkstra' en el panel derecho.");
        ImGui::Text("2. Elige el aeropuerto de origen y destino.");
        ImGui::Text("3. Haz clic en 'Ejecutar'. La ruta optima se resalta y la camara viaja automaticamente (Auto-encuadre).");
        ImGui::Text("4. Revisa la comparativa academica con BFS en el Dashboard de Resultado.");
        ImGui::Spacing();
        
        aviso("Si no existe camino entre origen y destino (grafo no conexo), el algoritmo lo indicara.");
    }

    static void secKruskal() {
        titulo(ICON_FA_SITEMAP " Algoritmo: Kruskal (Arbol de Expansion Minima)");
        
        ImGui::Text("Kruskal encuentra el subconjunto de aristas que conecta TODOS los nodos del grafo con el menor peso total posible, sin formar ciclos. El resultado es un arbol (Minimum Spanning Tree, MST).");
        ImGui::Spacing();
        
        subtitulo("Como funciona (paso a paso)");
        ImGui::Text("1. Se ordenan todas las aristas por peso, de menor a mayor.");
        ImGui::Text("2. Se toma la arista mas barata.");
        ImGui::Text("3. Si agregarla NO crea un ciclo, se incluye en el MST.");
        ImGui::Text("4. Si crearia un ciclo, se descarta.");
        ImGui::Text("5. Se repite hasta tener N-1 aristas (donde N = numero de nodos).");
        ImGui::Spacing();
        
        subtitulo("Union-Find (estructura interna)");
        ImGui::Text("Para verificar si una arista crea ciclo, Kruskal usa una estructura Union-Find que agrupa nodos en conjuntos disjuntos. Dos nodos en el mismo conjunto ya estan conectados; agregar una arista entre ellos crearia ciclo.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones en la vida real");
        ImGui::BulletText("Tender fibra optica: conectar todas las ciudades con el menor kilometraje de cable.");
        ImGui::BulletText("Redes electricas: disenar el tendido electrico mas economico.");
        ImGui::BulletText("Clustering: en machine learning, variantes de MST se usan para agrupar datos similares.");
        ImGui::Spacing();
        
        subtitulo("Como usarlo en la app");
        ImGui::Text("1. Selecciona 'Arbol' en el panel derecho.");
        ImGui::Text("2. Haz clic en 'Calcular MST'. Las aristas del arbol se resaltan.");
        ImGui::Text("3. Puedes activar la animacion para ver como el algoritmo evalua cada arista.");
        ImGui::Spacing();
        
        tip("El MST siempre tiene exactamente N-1 aristas para un grafo conexo con N nodos.");
    }

    static void secEuler() {
        titulo(ICON_FA_CIRCLE_NODES " Caminos de Euler y Hamilton");
        
        ImGui::Text("Son dos de los recorridos mas famosos en la teoria de grafos. Aunque suenan parecidos, sus requisitos matematicos son drasticamente diferentes.");
        ImGui::Spacing();
        
        subtitulo("Camino Euleriano (Ruta de Mantenimiento)");
        ImGui::Text("Recorre TODAS las ARISTAS (rutas) exactamente una vez.");
        ImGui::Text("Requisito estricto: El grafo debe ser conexo y tener EXACTAMENTE 0 o 2 nodos con cantidad impar de conexiones.");
        ImGui::Text("Por que falla en AeroGrafos? En el mundo real, los aeropuertos tienen cantidades caoticas de conexiones (unos tienen 1 vuelo, otros 20). Cumplir la regla de '0 o 2 nodos impares' en un mapa mundial real es practicamente imposible.");
        ImGui::Spacing();
        
        subtitulo("Camino Hamiltoniano (Vuelta al Mundo)");
        ImGui::Text("Recorre TODOS los NODOS (ciudades) exactamente una vez.");
        ImGui::Text("Requisito estricto: No existe formula rapida (es NP-Completo), pero si hay 'callejones sin salida' (ciudades con solo 1 conexion), es imposible recorrer el grafo sin quedar atrapado y tener que repetir ciudad para salir.");
        ImGui::Text("Por que falla en AeroGrafos? El mapa contiene aeropuertos remotos (ej. Isla de Pascua o hubs secundarios) que actuan como callejones sin salida. Si entras a ellos, debes salir por la misma ruta, repitiendo nodos y violando la regla de Hamilton.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones Reales");
        ImGui::BulletText("Euler: Rutas de camiones de basura, inspeccion de vias ferreas, barrido de nieve (importan las calles, no las esquinas).");
        ImGui::BulletText("Hamilton: Problema del Agente Viajero (TSP), logistica de entregas (importan los destinos, no el asfalto).");
        ImGui::Spacing();
        
        tip("En modo Grafos (lienzo libre) puedes dibujar a mano un poligono perfecto o un moño para probar que estos algoritmos si funcionan cuando la topologia es matematicamente perfecta.");
    }

    static void secBFSDFS() {
        titulo(ICON_FA_MAGNIFYING_GLASS " Busqueda: BFS y DFS");
        
        subtitulo("BFS (Breadth-First Search / Busqueda en Anchura)");
        ImGui::Text("Explora el grafo nivel por nivel, como ondas concentricas. Primero visita todos los vecinos directos, luego los vecinos de los vecinos, y asi sucesivamente.");
        ImGui::BulletText("Garantiza encontrar el camino mas corto (en numero de aristas).");
        ImGui::BulletText("Usa una cola (FIFO) internamente.");
        ImGui::BulletText("Util para: distancias minimas, verificar conectividad, niveles.");
        ImGui::Spacing();
        
        subtitulo("DFS (Depth-First Search / Busqueda en Profundidad)");
        ImGui::Text("Explora el grafo siguiendo un camino lo mas profundo posible antes de retroceder. Va 'hasta el fondo' de cada rama antes de volver.");
        ImGui::BulletText("Usa una pila (LIFO) o recursion.");
        ImGui::BulletText("Util para: deteccion de ciclos, componentes conexas, orden topologico.");
        ImGui::Spacing();
        
        subtitulo("BFS vs DFS");
        ImGui::BulletText("BFS usa mas memoria (guarda todo un nivel en cola) pero encuentra caminos minimos.");
        ImGui::BulletText("DFS usa menos memoria pero NO garantiza camino minimo.");
        ImGui::Spacing();
        
        subtitulo("Como usarlos en la app");
        ImGui::Text("1. Selecciona 'Busqueda' en el panel derecho.");
        ImGui::Text("2. Elige el nodo de inicio.");
        ImGui::Text("3. Ejecuta BFS o DFS. Los resultados muestran el orden de visita y el arbol de busqueda.");
    }

    static void secBipartito() {
        titulo(ICON_FA_CIRCLE_HALF_STROKE " Grafo Bipartito");
        
        ImGui::Text("Un grafo es bipartito si sus nodos se pueden dividir en dos conjuntos A y B de forma que TODAS las aristas van de un nodo de A a un nodo de B. Ninguna arista conecta dos nodos del mismo conjunto.");
        ImGui::Spacing();
        
        subtitulo("Propiedad fundamental");
        ImGui::Text("Un grafo es bipartito si y solo si NO contiene ciclos de longitud impar. Esto se puede verificar con una busqueda BFS intentando colorear con 2 colores.");
        ImGui::Spacing();
        
        subtitulo("Aplicaciones");
        ImGui::BulletText("Matching (asignacion): asignar trabajadores a tareas, alumnos a proyectos.");
        ImGui::BulletText("Recomendaciones: grafos usuario-producto (bipartitos por naturaleza).");
        ImGui::BulletText("Compiladores: coloreo de grafos de interferencia con 2 registros.");
        ImGui::Spacing();
        
        subtitulo("Como verificarlo en la app");
        ImGui::Text("El panel izquierdo muestra automaticamente si tu grafo es Bipartito. Si dice 'Si', significa que puedes dividir los nodos en exactamente dos grupos sin conflictos.");
        ImGui::Spacing();
        
        tip("Todo arbol es bipartito. Todo ciclo par es bipartito. Todo ciclo impar NO es bipartito.");
    }

    static void secMatrices() {
        titulo(ICON_FA_TABLE_CELLS " Matrices de Adyacencia e Incidencia");
        
        subtitulo("Matriz de Adyacencia");
        ImGui::Text("Es una tabla cuadrada NxN (N = numero de nodos). La celda (i,j) vale 1 si existe una arista entre el nodo i y el nodo j, y 0 en caso contrario. En grafos ponderados, el valor es el peso de la arista.");
        ImGui::BulletText("Es simetrica para grafos no dirigidos.");
        ImGui::BulletText("La diagonal siempre es 0 (un nodo no se conecta consigo mismo).");
        ImGui::BulletText("La suma de una fila = grado de ese nodo.");
        ImGui::Spacing();
        
        subtitulo("Matriz de Incidencia");
        ImGui::Text("Es una tabla NxM (N nodos, M aristas). La celda (i,k) vale 1 si el nodo i es uno de los extremos de la arista k, y 0 si no.");
        ImGui::BulletText("Cada columna tiene exactamente dos 1s (los dos extremos de la arista).");
        ImGui::Spacing();
        
        subtitulo("Como verlas en la app");
        ImGui::Text("Haz clic en la pestana 'Matrices' en la zona central (junto al Lienzo y el Registro del Kernel). Se actualizan en tiempo real conforme modificas el grafo.");
    }

    static void secPanelInfo() {
        titulo(ICON_FA_SLIDERS " Panel de Informacion del Grafo");
        
        ImGui::Text("El panel izquierdo muestra informacion en tiempo real sobre el estado actual del grafo:");
        ImGui::Spacing();
        ImGui::BulletText("Modo actual: Grafos o AeroGrafos.");
        ImGui::BulletText("Cantidad de nodos y aristas.");
        ImGui::BulletText("Densidad del grafo (aristas existentes / aristas posibles).");
        ImGui::BulletText("Propiedades automaticas:");
        ImGui::Spacing();
        ImGui::Text("  - Conexo: Se puede llegar de cualquier nodo a cualquier otro?");
        ImGui::Text("  - Bipartito: Se pueden dividir los nodos en dos grupos sin conflicto?");
        ImGui::Text("  - Euleriano: Se pueden recorrer todas las aristas sin repetir?");
        ImGui::Spacing();
        ImGui::Text("Estas propiedades se recalculan automaticamente cada vez que agregas o eliminas un nodo o arista.");
    }

    static void secSimulacion() {
        titulo(ICON_FA_CHART_LINE " Dashboard y Extras AeroGrafos");
        
        ImGui::Text("En modo AeroGrafos, el panel inferior derecho contiene el Dashboard Analitico y controles de visualizacion:");
        ImGui::Spacing();
        
        subtitulo("Dashboard Analitico");
        ImGui::BulletText("Densidad: Mide la cantidad de rutas existentes vs el maximo teorico.");
        ImGui::BulletText("Top Hubs: Identifica los 3 aeropuertos con mas conexiones del mundo.");
        ImGui::Spacing();
        
        subtitulo("Visualizacion");
        ImGui::Text("Puedes desactivar el Grid, ocultar las rutas no activas, o los nombres de las ciudades para ver mejor las rutas dibujadas por los algoritmos.");
        ImGui::Spacing();
        
        subtitulo("Restriccion Geopolitica (Rusia)");
        ImGui::Text("Puedes simular el cierre del espacio aereo ruso (ej. Moscu SVO) marcando la casilla de restricciones geopoliticas. Al hacer esto:");
        ImGui::BulletText("Las rutas de transito que tocan Moscu se invalidan automaticamente.");
        ImGui::BulletText("Los algoritmos como Dijkstra o Kruskal buscaran vias alternas (ej. conectando occidente y oriente via Dubai o Estambul).");
        ImGui::Spacing();
        
        tip("Activa la restriccion y luego ejecuta Dijkstra de Madrid a Tokio para comparar la nueva ruta y el costo extra en kilometros.");
    }
};
