# Guía Completa: Modo AeroGrafos ✈️🌍

Este documento está diseñado para ayudarte a entender la lógica, funcionamiento y justificación matemática detrás del módulo **AeroGrafos** dentro del simulador *GraphCore*. Está redactado de manera sencilla pero con el rigor técnico necesario para defender el proyecto.

## ¿Qué es AeroGrafos?
AeroGrafos es un simulador geopolítico que modela una red real de vuelos entre las 63 ciudades/hubs más importantes del mundo. Cada ciudad es un "nodo" y cada vuelo directo es una "arista". 

A diferencia del modo libre, aquí **las posiciones de las ciudades son inmutables** (están atadas a sus coordenadas de Latitud y Longitud reales) y la distancia visual que ves en pantalla se calcula usando **Distancias Haversine** (geometría esférica de la Tierra) en lugar de una simple línea recta, por lo que las trayectorias se curvan imitando las verdaderas rutas de vuelo ortodrómicas.

---

## 🧭 Algoritmos Implementados

Aquí te explicamos qué hace cada algoritmo en el panel, cómo funciona y qué resultados debes esperar en la defensa.

### 1. 📍 Ruta más corta (Dijkstra)
- **Qué hace:** Encuentra el camino de menor distancia real entre la ciudad inicial que elijas y la ciudad de destino.
- **Cómo lo hace:** Evalúa el "peso" (los kilómetros Haversine) de cada conexión, priorizando siempre la ramificación que acumule menos kilómetros totales.
- **Qué debes mostrar:** Selecciona dos ciudades lejanas (ej. Buenos Aires a Tokio). Al darle Play a la animación, verás cómo Dijkstra explora en forma de "mancha" expansiva hasta llegar al destino.

### 2. 🔌 Conectar Todo el Mapa (Kruskal - MST)
- **Qué hace:** Crea un Árbol de Expansión Mínima (Minimum Spanning Tree). Busca conectar las 63 ciudades del mundo usando la menor cantidad de kilómetros totales de asfalto/vuelos, garantizando que no queden ciudades aisladas.
- **Cómo lo hace:** Ordena todos los vuelos del mundo de menor a mayor distancia. Va agarrando los más cortos y los aprueba (líneas verdes) *siempre y cuando* no formen un ciclo cerrado (para eso usa la estructura de datos Disjoint-Set o Union-Find).
- **Qué debes mostrar:** Explica que esto sirve en la vida real para tirar cables de fibra óptica o tuberías de petróleo minimizando costos. Al terminar la animación, no verás "anillos" de rutas, solo ramificaciones.

### 3. ♻️ Ruta de Mantenimiento (Camino Euleriano)
- **Qué hace:** Busca una ruta que recorra **TODAS LAS RUTAS DE VUELO (Aristas)** exactamente una vez sin repetir ninguna.
- **Cómo lo hace:** Utiliza el algoritmo de Hierholzer. Primero valida que el grafo cumpla la estricta regla matemática de Euler: todo debe estar conectado y deben existir **exactamente 0 o 2 ciudades** con cantidad impar de conexiones. 
- **⚠️ ¿Por qué falla en el mundo real?:** Los aeropuertos de la vida real tienen grados caóticos (ciudades con 1 vuelo, otras con 15 vuelos). Por ende, es matemáticamente imposible recorrer todas las conexiones del planeta sin repetir alguna. Si cargas un archivo JSON personalizado perfecto, el algoritmo trazará el recorrido con éxito.

### 4. 🌍 Vuelta al Mundo (Camino Hamiltoniano)
- **Qué hace:** Busca recorrer **TODAS LAS CIUDADES (Nodos)** exactamente una vez sin repetir ciudad.
- **Cómo lo hace:** Al ser un problema NP-Completo (matemáticamente lentísimo de calcular a fuerza bruta para 63 nodos), usamos una heurística del "Vecino más cercano". Desde la ciudad que elijas, siempre salta a la ciudad no visitada más cercana.
- **⚠️ ¿Por qué se atasca?:** El mundo tiene aeropuertos remotos que actúan como "callejones sin salida" (solo tienen 1 conexión de entrada/salida). Si entras a esa ciudad, la única forma de salir es regresando por donde viniste, violando la regla de "no repetir nodos". La simulación detectará este atasco y mostrará cuántas ciudades lograste visitar antes de quedar acorralado.

### 5. 🔍 Búsquedas por Capas (BFS y DFS)
- **BFS (Exploración por Niveles):** Avanza como ondas de agua en un estanque. Primero visita los destinos directos, luego los destinos de esos destinos. Sirve para ver a cuántas "escalas" está una ciudad.
- **DFS (Exploración Profunda):** Avanza a ciegas tomando siempre un avión tras otro hasta llegar a un callejón sin salida, donde da marcha atrás. Sirve para resolver laberintos.

---

## ⚙️ Uso del Dashboard Analítico
Si haces clic en el botón **"Analizar Red"** (icono de lupa en el combo box), el sistema arrojará estadísticas vitales:
- **Grado Promedio:** Media de vuelos por ciudad.
- **Hub más grande:** La ciudad con más conexiones (el centro más congestionado).
- **Densidad:** Qué tan interconectado está el mundo respecto al máximo teórico posible.
- **Diámetro:** La cantidad máxima de escalas que tomaría ir de una punta a la otra en el peor de los casos óptimos.

---

## 👩‍💻 Tips de Defensa
1. **Pausas:** Usa los botones de `< Paso atrás` y `Pausa` durante las animaciones de BFS o Kruskal para explicar qué decisión matemática está tomando la computadora en ese momento exacto.
2. **Contexto:** Deja claro a los profesores que los errores de Euler y Hamilton **no son errores de código**, sino validaciones matemáticas formales de topologías incompatibles.
3. **Fluidez:** Muestra el zoom y el paneo (arrastrando con el click del medio). Al usar el mouse, el auto-lerp se desactiva instantáneamente, haciendo que la aplicación se sienta nativa y profesional.

¡Mucho éxito en la defensa!
