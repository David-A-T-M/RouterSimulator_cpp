cmake_minimum_required(VERSION 3.10)
project(RouterSimulator_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# --- 1. Descargar Google Test automáticamente ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- 2. Configurar Directorios de Inclusión ---
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)
add_executable(RouterSimulator_cpp main.cpp)

add_library(RouterLib ${SOURCES})
target_include_directories(RouterLib
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# --- 3. Recopilar archivos fuente (.cpp) ---


# --- 4. Crear la librería y el ejecutable principal ---
target_link_libraries(RouterSimulator_cpp RouterLib)

# --- 5. Configurar Tests ---
enable_testing()

# Buscamos todos los archivos que empiecen con test_ en la carpeta tests/
file(GLOB TEST_SOURCES "tests/test_*.cpp")

foreach(test_file ${TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} RouterLib gtest_main)

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
