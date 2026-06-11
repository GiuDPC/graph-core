# Manual de Usuario Completo - OptiClusters (NetSim Pro) v4.0

Bienvenido a OptiClusters. Este manual detallado te guiará paso a paso para que puedas aprovechar al máximo todas las funciones de simulación, análisis de grafos y redes.

---

## 1. MODO GRAFOS: Teoría y Algoritmos
En este modo, la aplicación se comporta como un laboratorio matemático puro. Los nodos son genéricos (V0, V1, V2) y las herramientas están enfocadas en la teoría de grafos.

### ¿Cómo probarlo?
1. **Crear un grafo de prueba**:
   * Haz click derecho en varias zonas oscuras de la pantalla para crear nodos.
   * Para conectarlos, haz click derecho sobre un nodo (se seleccionará) y luego click derecho sobre otro. Ponles un "peso" (por ejemplo, 10) y elige si la conexión es de un solo sentido (dirigida) o de doble sentido.
2. **Jugar con el Isomorfismo**:
   * Ve a la pestaña **Isomorfismo** a la izquierda.
   * Toca el botón mágico **"Generar Isomorfo Automático"**.
   * Verás que aparece un segundo grafo (G2) a la derecha, totalmente desordenado.
   * Presiona **Verificar Isomorfismo**. El sistema te dirá que son idénticos porque, aunque las posiciones cambien, las conexiones internas son exactamente las mismas.
3. **Analizar Árboles y Kruskal**:
   * Si tu grafo tiene un bucle (ej: 3 nodos conectados en triángulo), ve a la pestaña **Árboles** y presiona "Verificar si es Árbol". Te dirá que no lo es.
   * Ahora presiona **Ejecutar Kruskal**. El algoritmo romperá visualmente las aristas más "caras" (las de mayor peso) y te dejará un árbol perfecto que conecta todos los nodos gastando la menor cantidad de peso posible. Verás la solución resaltada en color morado.

---

## 2. MODO REDES: Simulación de Tráfico
En este modo, OptiClusters se transforma en un simulador de topologías de red (estilo Packet Tracer).

### ¿Cómo probarlo?
1. Cambia el interruptor arriba a la izquierda de "GRAFOS" a **"REDES"**.
2. **Crear la Topología**:
   * Haz click derecho en el lienzo. Verás que ahora te pregunta qué tipo de dispositivo quieres crear: Router, Switch, Servidor, PC o Antena.
   * Crea 1 Router, 2 Switches y 4 PCs.
   * Conéctalos haciendo click derecho entre ellos: El Router a los Switches, y los Switches a las PCs. Ponles peso (latencia o distancia del cable).
3. **Simular Envío de Paquetes (Ruteo)**:
   * Ve a la pestaña **Redes (Ruteo)**.
   * **Origen**: Selecciona una PC.
   * **Destino**: Selecciona otra PC en el otro extremo de la red.
   * **Velocidad**: Ajusta el deslizador a `1.00 s/paso` para verlo claro.
   * **¡Presiona Simular Envío!**
   * **¿Qué sucede?** Verás una partícula verde salir de la PC origen. El algoritmo (basado en el protocolo OSPF/Dijkstra) evaluará en tiempo real el camino más corto/rápido hacia el destino. La partícula viajará nodo por nodo, y en cada parada podrás ver en el panel el "Log" de por dónde está pasando, simulando un `traceroute`. Al llegar, el nodo destino "saltará" con un efecto visual confirmando la recepción.

---

## 3. Comprendiendo las Matrices
En la zona derecha de la pantalla tienes el panel matemático en tiempo real. 
* **Matriz de Adyacencia**: Muestra una cuadrícula que cruza todos los nodos contra todos los nodos. Si el nodo V0 se conecta con V1 con un peso de 10, verás un "10" en color amarillo o rojo según qué tan grande sea el número. Es útil para ver rápidamente quién no está conectado con quién (marcado con `-`).
* **Matriz de Incidencia**: Útil para ingeniería avanzada. Las filas son los Nodos y las columnas son los Cables (Aristas e0, e1, etc). Muestra un `1` si el nodo está enviando datos por ese cable, un `-1` si los está recibiendo, y un `2` si es un cable bidireccional puro.

---

## 4. Archivos de Ejemplo (Cargar)
En la parte superior, ve a `Archivo -> Cargar...` y entra a la carpeta `ejemplos_opticlusters`. Allí encontrarás:
*   **`arbol_disparejo.json`**: Cárgalo, ve a la pestaña Árboles y presiona **Aplicar Layout Jerárquico**. Verás cómo el programa desenreda el caos y lo organiza como el organigrama de una empresa perfecta.
*   **`red_jerarquica_compleja.json`**: Cárgalo en Modo Redes. Simula un envío desde un extremo al otro para ver cómo la partícula navega por el "Core" de la red.
*   **`grafo_exotico_espiral.json`**: Cárgalo en Modo Grafos. Prueba a aplicarle "Kruskal" para ver cómo destruye la doble hélice manteniendo solo la columna vertebral.

---

## 5. Controles Universales
*   **Mover un solo nodo**: Click izquierdo sostenido.
*   **Mover TODO el grafo**: Mantén `SHIFT` en tu teclado + Click izquierdo en cualquier nodo y arrastra.
*   **Mover la cámara general**: Mantén Click izquierdo en una zona vacía oscura y arrastra el mouse.
*   **Zoom**: Rueda del mouse.
