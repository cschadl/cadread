cmake_minimum_required(VERSION 3.16)

PROJECT(cadread LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(OCC REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options system filesystem)

if (UNIX)
    find_library(FreeType REQUIRED)
    find_path(FONTCONFIG_INCLUDE_DIRS "fontconfig/fontconfig.h")
endif(UNIX)

set(STLIMPORT_PATH ${CMAKE_SOURCE_DIR}/stl-import CACHE STRING "path to stl-import")
set(MATHSTUFF_PATH ${STLIMPORT_PATH}/submodules/mathstuff CACHE STRING "")
set(STLUTIL_PATH ${STLIMPORT_PATH}/submodules/stlutil CACHE STRING "")
add_subdirectory(${STLIMPORT_PATH})

set(CADREAD_SRC
    src/cadread_ConsoleProgressIndicator.cpp
    src/cadread.cpp
    src/Importing.cpp
)

add_executable("cadread" ${CADREAD_SRC})

target_include_directories("cadread" PUBLIC ${OCC_INCLUDE_DIR})
include_directories("cadread" src)

target_include_directories("cadread" PRIVATE ${STLIMPORT_PATH}/stl_import)
target_include_directories("cadread" PUBLIC ${MATHSTUFF_PATH})
target_include_directories("cadread" PUBLIC ${STLUTIL_PATH})
target_include_directories("cadread" PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries("cadread" PUBLIC stl_import ${OCC_LIBRARIES} ${Boost_LIBRARIES})