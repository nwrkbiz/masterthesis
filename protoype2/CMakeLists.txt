# Autor: Daniel Giritzer
# Linux:
# mkdir build
# cd ./build
# cmake .. -DSMF_3P=~/SMF_3P
# cmake --build .
# Windows:
# mkdir build
# cd ./build
# cmake .. -T v140 -A x64 -DSMF_3P=C:/SMF_3P
# cmake --build .
cmake_minimum_required(VERSION 3.5)

project(prototyp2 VERSION 1.0.0.0)

# Überprüfe SMF_3P Verzeichnis
set(SMF_3P "" CACHE PATH "SMF_3P directory")
if(NOT EXISTS "${SMF_3P}/v8")
    message(FATAL_ERROR "Please set SMF_3P path using -DSMF_3P")
endif()
message(STATUS "SMF_3P is set to: ${SMF_3P}")

#### SMF_3P Flags, Linux
if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  # Linker suchpfad
  link_directories(${SMF_3P}/v8/lib/sles_x64-release)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
  set(BASE_V8_LIBS v8_base)
  set(ADDITIONAL_V8_LIBS "pthread")
#### SMF_3P Flags, Windows
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
  # Linker suchpfad
  link_directories(${SMF_3P}/v8/lib/win-x64-release)
  set(BASE_V8_LIBS v8_base_0 
                   v8_base_1)
  set(ADDITIONAL_V8_LIBS WS2_32.lib
                         dbghelp.lib
                         shlwapi.lib
                         winmm.lib
                         kernel32.lib
                         user32.lib
                         gdi32.lib
                         winspool.lib
                         comdlg32.lib
                         advapi32.lib
                         shell32.lib
                         ole32.lib
                         oleaut32.lib
                         uuid.lib
                         odbc32.lib
                         odbccp32.lib)
endif()

# Quellcode
add_executable(prototyp2 src/prototyp2.cpp ${CMAKE_CURRENT_BINARY_DIR}/prototyp.rc)


# SMF_3P/V8 libs
target_link_libraries(prototyp2 inspector
                                ${BASE_V8_LIBS}
                                v8_builtins_setup
                                v8_builtins_generators
                                v8_nosnapshot
                                v8_snapshot
                                v8_libplatform
                                v8_libsampler
                                v8_libbase
                                icui18n
                                icuuc
                                ${ADDITIONAL_V8_LIBS}
                                )


# SMF_3P/V8 includes
target_include_directories(prototyp2 PUBLIC
                          "${SMF_3P}/v8/include"
                          )


# allgemeine Optionen
set_property(TARGET prototyp2 PROPERTY CXX_STANDARD 11)
target_include_directories(prototyp2 PUBLIC 
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
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
endif()

# Installationseinstellungen
INSTALL(TARGETS prototyp2
        LIBRARY DESTINATION "prototyp2/"
        ARCHIVE DESTINATION "prototyp2/"
        RUNTIME DESTINATION "prototyp2/"
        PUBLIC_HEADER DESTINATION "prototyp2/"
)
