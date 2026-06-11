# ============================================================================
# OptiClusters — Sistema de compilación principal
# ============================================================================
# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.20)

# Nombre del proyecto, versión y lenguaje
project(OptiClusters
    VERSION 4.0.0
    LANGUAGES CXX
)

# Usamos C++17 obligatorio (no opcional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ============================================================================
# Dependencias externas (se descargan automáticamente)
# ============================================================================
include(FetchContent)

# --- GLFW (crea la ventana del sistema operativo) ---
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
# Desactivar cosas de GLFW que no necesitamos
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- Dear ImGui (la librería de interfaz gráfica) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

# --- nlohmann/json (serialización JSON) ---
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)



# ============================================================================
# Crear librería de ImGui (ImGui no tiene CMakeLists.txt propio)
# ============================================================================
# ImGui necesita que nosotros le digamos qué archivos compilar
add_library(imgui_lib STATIC
    # Archivos core de ImGui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    # Backend: cómo ImGui se conecta con GLFW y OpenGL
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Decirle a ImGui dónde están sus headers
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Conectar ImGui con GLFW y OpenGL
target_link_libraries(imgui_lib PUBLIC glfw)

# En Linux necesitamos linkear con OpenGL explícitamente
find_package(OpenGL REQUIRED)
target_link_libraries(imgui_lib PUBLIC OpenGL::GL)

# ============================================================================
# Nuestro ejecutable
# ============================================================================
# OptiClusters — con audio v4.0

add_executable(OptiClusters
    src/main.cpp
    src/audio/miniaudio_impl.cpp
)

# Cuando compilamos para Windows (nativo o cross), ocultamos la consola
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(OptiClusters PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Linkear nuestro programa con ImGui y JSON
target_link_libraries(OptiClusters PRIVATE
    imgui_lib
    nlohmann_json::nlohmann_json
)

# Que nuestro código pueda hacer #include de nuestros propios headers
target_include_directories(OptiClusters PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/nucleo
    ${CMAKE_SOURCE_DIR}/src/nucleo/tipos
    ${CMAKE_SOURCE_DIR}/src/nucleo/algoritmos
    ${CMAKE_SOURCE_DIR}/src/persistencia
    ${CMAKE_SOURCE_DIR}/src/interfaz
    ${CMAKE_SOURCE_DIR}/src/audio
)