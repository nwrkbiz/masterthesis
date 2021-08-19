# Autor: Daniel Giritzer
# Linux:
# mkdir build
# cd ./build
# cmake .. -DWINCCOA_INSTALL=/opt/WinCC_OA/3.16 -DSMF_3P=~/SMF_3P
# cmake --build .
# Windows:
# mkdir build
# cd ./build
# cmake .. -T v140 -A x64 -DWINCCOA_INSTALL=C:/Siemens/Automation/WinCC_OA/3.16 -DSMF_3P=C:/SMF_3P
# cmake --build .
cmake_minimum_required(VERSION 3.5)

project(prototyp3 VERSION 1.0.0.0)

# Überprüfe WinCC OA Verzeichnis und parse Versionsnummer
set(WINCCOA_INSTALL "" CACHE PATH "Wincc OA installation directory")
if(NOT EXISTS "${WINCCOA_INSTALL}")
    message(FATAL_ERROR "Please set WinCC OA install path using -DWINCCOA_INSTALL")
endif()
get_filename_component(WINCCOA_VERSION "${WINCCOA_INSTALL}" NAME)
message(STATUS "WINCCOA_INSTALL is set to: ${WINCCOA_INSTALL}")
message(STATUS "Detected WinCC OA Version: ${WINCCOA_VERSION}")

# Überprüfe SMF_3P Verzeichnis
set(SMF_3P "" CACHE PATH "SMF_3P directory")
if(NOT EXISTS "${SMF_3P}/v8")
    message(FATAL_ERROR "Please set SMF_3P path using -DSMF_3P")
endif()
message(STATUS "SMF_3P is set to: ${SMF_3P}")

#### WinCC OA Flags, Linux
if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    link_directories(${WINCCOA_INSTALL}/api/lib.linux)
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

    string(REPLACE "." "" VERSION_STR ${WINCCOA_VERSION})

    # bereite Linux Dateinamendekoration vor
    set(WINCCOA_LIBRARY_PREFIX "")
    set(WINCCOA_LIBRARY_POSTFIX "V${VERSION_STR}")

#### WinCC OA Flags, Windows
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    link_directories(${WINCCOA_INSTALL}/api/lib.winnt)

    add_definitions(-DIS_MSWIN__ -DWIN32 -D_BOOL_T_DEFINED -D_SBCS
    -D_CRT_NON_CONFORMING_SWPRINTFS -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE -DNODEF__RPC_H__ -DWINVER=0x0500)

    # bereite Windows Dateinamendekoration vor
    set(WINCCOA_LIBRARY_PREFIX "lib")
    set(WINCCOA_LIBRARY_POSTFIX "")
endif()

#### SMF_3P Flags, Linux
if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  # Linker suchpfad
  link_directories(${SMF_3P}/v8/lib/sles_x64-release)
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

set(CMAKE_INSTALL_RPATH "${WINCCOA_INSTALL}/bin")

# Linker suchpfad
link_directories(${WINCCOA_INSTALL}/bin)

# Quellcode
add_executable(prototyp3 src/prototyp3.cpp ${CMAKE_CURRENT_BINARY_DIR}/prototyp.rc)


# WinCC OA libs
target_link_libraries(prototyp3 bcm${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Basics${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Configs${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Datapoint${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Messages${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Manager${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Ctrl${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}ComDrv
                              ${WINCCOA_LIBRARY_PREFIX}V24)

# WinCC OA includes
target_include_directories(prototyp3 PUBLIC
                          "${WINCCOA_INSTALL}/api/include/OaBasics/Accu"
                          "${WINCCOA_INSTALL}/api/include/OaBasics/Variables"
                          "${WINCCOA_INSTALL}/api/include/OaBasics/Utilities"
                          "${WINCCOA_INSTALL}/api/include/OaBasics/NoPosix"
                          "${WINCCOA_INSTALL}/api/include/OaBasics/Core"
                          "${WINCCOA_INSTALL}/api/include/Basics/Variables"
                          "${WINCCOA_INSTALL}/api/include/Basics/Utilities"
                          "${WINCCOA_INSTALL}/api/include/Basics/NoPosix"
                          "${WINCCOA_INSTALL}/api/include/Basics/DpBasics"
                          "${WINCCOA_INSTALL}/api/include/BCMNew"
                          "${WINCCOA_INSTALL}/api/include/ComDrv"
                          "${WINCCOA_INSTALL}/api/include/Configs"
                          "${WINCCOA_INSTALL}/api/include/Configs/DrvConfigs/DrvCommon"
                          "${WINCCOA_INSTALL}/api/include/Configs/DrvConfigs/ConvSmooth"
                          "${WINCCOA_INSTALL}/api/include/Ctrl"
                          "${WINCCOA_INSTALL}/api/include/Datapoint"
                          "${WINCCOA_INSTALL}/api/include/Manager"
                          "${WINCCOA_INSTALL}/api/include/Messages"
                          "${WINCCOA_INSTALL}/api/include/V24"
                          "${WINCCOA_INSTALL}/api/include/"
                          )

# SMF_3P/V8 libs
target_link_libraries(prototyp3 inspector
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
target_include_directories(prototyp3 PUBLIC
                          "${SMF_3P}/v8/include"
                          )

# allgemeine Optionen
set_property(TARGET prototyp3 PROPERTY CXX_STANDARD 11)
target_include_directories(prototyp3 PUBLIC 
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
INSTALL(TARGETS prototyp3
        LIBRARY DESTINATION "prototyp3/"
        ARCHIVE DESTINATION "prototyp3/"
        RUNTIME DESTINATION "prototyp3/"
        PUBLIC_HEADER DESTINATION "prototyp3/"
)
