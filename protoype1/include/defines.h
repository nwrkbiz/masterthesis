/**
 * @author Daniel Giritzer
 * @file defines.h
 * @brief Defines, die normalerweise von den WinCC OA 
 * build Skripten als Compiler parameter gesetzt werden.
 * @copyright Daniel Giritzer 2021. All Rights Reserved.
 */
#ifndef PROTOTYP_H
#define PROTOTYP_H

#ifndef _WIN32
#define DLLEXP_BCM
#define DLLEXP_BASICS
#define DLLEXP_OABASICS
#define DLLEXP_CONFIGS
#define DLLEXP_DATAPOINT
#define DLLEXP_MESSAGES
#define DLLEXP_MANAGER
#define DLLEXP_OAMANAGER
#define DLLEXP_CTRL
#define IL_DEPRECATED(MSG) __attribute__((__deprecated__(MSG)))
#else
#include <winnt/win32.h>
#pragma warning (disable: 4251)
#endif

#endif // PROTOTYP_H