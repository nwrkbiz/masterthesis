# Autor: Daniel Giritzer
# Linux:
# mkdir build
# cd ./build
# cmake ..
# cmake --build .
# Windows:
# mkdir build
# cd ./build
# cmake ..
# cmake --build .
cmake_minimum_required(VERSION 3.5)

project(prototyp4 VERSION 1.0.0.0)

set(PUBLIC_HEADER_FILES "include/prototyp4.h")

# Quellcode
add_library(prototyp4 SHARED src/prototyp4.cpp ${CMAKE_CURRENT_BINARY_DIR}/prototyp.rc)

# allgemeine Optionen
set_property(TARGET prototyp4 PROPERTY CXX_STANDARD 11)
add_definitions(-D_BUILDING)
target_include_directories(prototyp4 PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "include"
                          )

# generiere Konfiguration
configure_file(include/prototyp.rc.in prototyp.rc)

# Warnungen als Fehler behandeln.
if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
endif()

# Installationseinstellungen
set_target_properties(prototyp4 PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER_FILES}"
                                         VERSION "${prototyp4_VERSION_MAJOR}.${prototyp4_VERSION_MINOR}.${prototyp4_VERSION_PATCH}.${prototyp4_VERSION_TWEAK}"
                                         SOVERSION "${prototyp4_VERSION_MAJOR}")
INSTALL(TARGETS prototyp4
        LIBRARY DESTINATION "prototyp4/"
        ARCHIVE DESTINATION "prototyp4/"
        RUNTIME DESTINATION "prototyp4/"
        PUBLIC_HEADER DESTINATION "prototyp4/"
)
