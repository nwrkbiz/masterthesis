/**
 * @author Daniel Giritzer
 * @file prototyp4.cpp
 * @brief Einfaches FFI Besipiel.
 * @copyright Daniel Giritzer 2021. All Rights Reserved.
 */
#include "prototyp4.h"

#include <iostream>
#include <string>

class Greeter {
public:
  void Welcome(const std::string& name){
    std::cout  << "Hello, "<< name << std::endl;
  };
};
DLLEXPNOMANGLE void PROTO_New(void*& instance){
  instance = new Greeter();
}
DLLEXPNOMANGLE void PROTO_Greet(void* inst, char* msg){
  Greeter* obj = static_cast<Greeter*>(inst);
  obj->Welcome(msg);
}
DLLEXPNOMANGLE void PROTO_Delete(void*& inst){
  Greeter* obj = static_cast<Greeter*>(inst);
  delete obj; inst = nullptr;
}