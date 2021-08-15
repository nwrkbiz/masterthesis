# Autor: Daniel Giritzer
# Linux:
# mkdir build
# cd ./build
# cmake .. -DWINCCOA_INSTALL=/opt/WinCC_OA/3.16
# cmake --build .
# Windows:
# mkdir build
# cd ./build
# cmake .. -T v140 -A x64 -DWINCCOA_INSTALL=C:/Siemens/Automation/WinCC_OA/3.16
# cmake --build .
cmake_minimum_required(VERSION 3.5)

project(prototyp VERSION 1.0.0.0)

# Überprüfe WinCC OA Verzeichnis und parse Versionsnummer
set(WINCCOA_INSTALL "" CACHE PATH "Wincc OA installation directory")
if(NOT EXISTS "${WINCCOA_INSTALL}")
    message(FATAL_ERROR "Please set WinCC OA install path using -DWINCCOA_INSTALL")
endif()
get_filename_component(WINCCOA_VERSION "${WINCCOA_INSTALL}" NAME)
message(STATUS "WINCCOA_INSTALL is set to: ${WINCCOA_INSTALL}")
message(STATUS "Detected WinCC OA Version: ${WINCCOA_VERSION}")

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

set(CMAKE_INSTALL_RPATH "${WINCCOA_INSTALL}/bin")

# Linker suchpfad
link_directories(${WINCCOA_INSTALL}/bin)

# Quellcode
add_executable(prototyp src/prototyp.cpp ${CMAKE_CURRENT_BINARY_DIR}/prototyp.rc)


# WinCC OA libs
target_link_libraries(prototyp bcm${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Basics${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Configs${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Datapoint${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Messages${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Manager${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}Ctrl${WINCCOA_LIBRARY_POSTFIX}
                              ${WINCCOA_LIBRARY_PREFIX}ComDrv
                              ${WINCCOA_LIBRARY_PREFIX}V24)

# WinCC OA includes
target_include_directories(prototyp PUBLIC
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


# allgemeine Optionen
set_property(TARGET prototyp PROPERTY CXX_STANDARD 11)
target_include_directories(prototyp PUBLIC 
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
INSTALL(TARGETS prototyp
        LIBRARY DESTINATION "prototyp/"
        ARCHIVE DESTINATION "prototyp/"
        RUNTIME DESTINATION "prototyp/"
        PUBLIC_HEADER DESTINATION "prototyp/"
)
