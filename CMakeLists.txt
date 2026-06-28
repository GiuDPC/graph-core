# ============================================================================
# graph-core — Sistema de compilación principal
# ============================================================================
# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.20)

# Nombre del proyecto, versión y lenguaje
project(graph-core
    VERSION 3.0.0
    LANGUAGES CXX
)

# Usamos C++17 obligatorio (no opcional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math -ftree-vectorize")
endif()


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
    $<$<CONFIG:Debug>:${imgui_SOURCE_DIR}/imgui_demo.cpp>
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

# Sistema de hilos (necesario para miniaudio — audio async)
find_package(Threads REQUIRED)

# ============================================================================
# Nuestro ejecutable
# ============================================================================
# Ejecutable principal

add_executable(graph-core
    src/main.cpp
    src/audio/miniaudio_impl.cpp
)

# Cuando compilamos para Windows (nativo o cross), ocultamos la consola
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(graph-core PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Linkear nuestro programa con ImGui, JSON, y sistema
target_link_libraries(graph-core PRIVATE
    imgui_lib
    nlohmann_json::nlohmann_json
    ${CMAKE_DL_LIBS}    # dlopen() → miniaudio carga PulseAudio/ALSA en runtime
    Threads::Threads     # hilos de audio (miniaudio async engine)
)

# Que nuestro código pueda hacer #include de nuestros propios headers
target_include_directories(graph-core PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/nucleo
    ${CMAKE_SOURCE_DIR}/src/nucleo/tipos
    ${CMAKE_SOURCE_DIR}/src/nucleo/algoritmos
    ${CMAKE_SOURCE_DIR}/src/persistencia
    ${CMAKE_SOURCE_DIR}/src/interfaz
    ${CMAKE_SOURCE_DIR}/src/audio
)

# Copiar carpeta de recursos (fuentes, iconos, etc.) al directorio de build
add_custom_command(TARGET graph-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/recursos
        ${CMAKE_BINARY_DIR}/recursos
)

# ============================================================================
# Tests unitarios (GoogleTest)
# ============================================================================
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Include paths para tests (necesitan imgui para ImVec2 en Nodo.h)
    set(TEST_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/nucleo
        ${CMAKE_SOURCE_DIR}/src/nucleo/tipos
        ${CMAKE_SOURCE_DIR}/src/nucleo/algoritmos
        ${CMAKE_SOURCE_DIR}/src/audio
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    add_executable(test_grafo
        tests/test_grafo.cpp
    )
    target_include_directories(test_grafo PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(test_grafo PRIVATE GTest::gtest_main)

    add_executable(test_dijkstra
        tests/test_dijkstra.cpp
    )
    target_include_directories(test_dijkstra PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(test_dijkstra PRIVATE GTest::gtest_main)

    add_executable(test_unionfind
        tests/test_unionfind.cpp
    )
    target_include_directories(test_unionfind PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(test_unionfind PRIVATE GTest::gtest_main)

    add_executable(test_bfs
        tests/test_bfs.cpp
    )
    target_include_directories(test_bfs PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(test_bfs PRIVATE GTest::gtest_main)

    add_executable(test_ciclos
        tests/test_ciclos.cpp
    )
    target_include_directories(test_ciclos PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(test_ciclos PRIVATE GTest::gtest_main)

    # Habilitar descubrimiento automatico de tests con CTest
    include(GoogleTest)
    gtest_discover_tests(test_grafo)
    gtest_discover_tests(test_dijkstra)
    gtest_discover_tests(test_unionfind)
    gtest_discover_tests(test_bfs)
    gtest_discover_tests(test_ciclos)
endif()