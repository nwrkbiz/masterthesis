/**
 * @author Daniel Giritzer
 * @file prototyp4.h
 * @brief Einfaches FFI Besipiel.
 * @copyright Daniel Giritzer 2021. All Rights Reserved.
 */
#ifndef PROTOTYP4_H
#define PROTOTYP4_H
#ifdef _WIN32
#ifdef _BUILDING
#define DLLEXPNOMANGLE extern "C" __declspec( dllexport )
#else
#define DLLEXPNOMANGLE extern "C" __declspec( dllimport )
#endif
#else
#define DLLEXPNOMANGLE extern "C"
#endif
// Erzeugt eine neue Instanz
DLLEXPNOMANGLE void PROTO_New(void*& instance);
// Führe Funktion von Instanz aus
DLLEXPNOMANGLE void PROTO_Greet(void* inst, char* msg);
// Lösche Instanz
DLLEXPNOMANGLE void PROTO_Delete(void*& instance);
#endif // PROTOTYP4_H