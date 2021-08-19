/**
 * @author Daniel Giritzer
 * @file prototyp.cpp
 * @brief Einfaches Beispiel, soll die Funktionsweise der WinCC OA API veranschaulichen.
 * @copyright Daniel Giritzer 2021. All Rights Reserved.
 */
#include "defines.h"

#include <iostream>
#include <string>

#include <Manager.hxx>
#include <Resources.hxx>
#include <StartDpInitSysMsg.hxx>
#include <HotLinkWaitForAnswer.hxx>
#include <FloatVar.hxx>
#include <BitVar.hxx>

// Diese Klasse erhält callbacks vom WinCC OA Framework
class DpObserver : public HotLinkWaitForAnswer {
public:
  // Diese Funktion wird bei jeder Wertänderung eines
  // Datenpunktelements (nach DpConnect) aufgerufen
  void hotLinkCallBack(DpHLGroup &group) override {
    std::cout << "--- hotLinkCallBack(DpHLGroup) ---" << std::endl;
    TimeVar ts = group.getOriginTime(); // Zeitstempel der Antwort
    // Ändert sich ein Datenpunktelement werden immer alle bei dem 
    // DpConnect registrierten mitübertragen, iteriere über alle Elemente.
    for (DpVCItem* item = group.getFirstItem(); item;
          item = group.getNextItem()) {
      Variable* value = item->getValuePtr(); // Neuer Wert
      CharString dpe = item->getDpIdentifier().toString(); // Name DPElement
      // Erkenne Variablentyp und gebe Werte aus
      std::cout << dpe << std::endl;
      if(value->isA(VariableType::BIT_VAR) == VariableType::BIT_VAR){
        std::cout << static_cast<BitVar*>(value)->getValue() << std::endl;
      } else if (value->isA(VariableType::FLOAT_VAR) == VariableType::FLOAT_VAR) {
        std::cout << static_cast<FloatVar*>(value)->getValue() << std::endl;
      } else {
        std::cout << "Unknown Variable type." << std::endl;
      }
    }
  }
  // Diese Funktion wird einmalig als antwort einer
  // einfachen Abfrage (DpConnect, DpGet, DpSet etc.) aufgerufen. 
  void hotLinkCallBack(DpMsgAnswer &answer) override {
    std::cout << "--- hotLinkCallBack(DpMsgAnswer) ---" << std::endl;
    for (AnswerGroup* grp = answer.getFirstGroup(); grp;
          grp = answer.getNextGroup()) {
      // Iteriere über Liste aller angefragten Datenpunktelementen
      for (AnswerItem* item = grp->getFirstItem(); item;
            item = grp->getNextItem()) {
        TimeVar ts = item->getTime(); // Zeitstempel, als der Wert geschrieben wurde
        Variable* value = item->getValuePtr(); // Derzeitiger Wert
        CharString dpe = item->getDpIdentifier().toString(); // Name DPElement
        // Erkenne Variablentyp und gebe Werte aus
        std::cout << dpe << std::endl;
        if(value->isA(VariableType::BIT_VAR) == VariableType::BIT_VAR){
          std::cout << static_cast<BitVar*>(value)->getValue() << std::endl;
        } else if (value->isA(VariableType::FLOAT_VAR) == VariableType::FLOAT_VAR) {
          std::cout << static_cast<FloatVar*>(value)->getValue() << std::endl;
        } else {
          std::cout << "Unknown Variable type." << std::endl;
        }
      }
    }
  }
};

class Agent : public Manager {
public:
  Agent() : 
  // Rufe CTor der Elternklasse auf,
  // Jeder Manager muss eindeutig
  // über den ManagerIdentifier identifizierbar sein
  Manager(ManagerIdentifier(
          ManagerType::API_MAN,
          Resources::getManNum())) {// verwendet geparsten '-num' Kommandozeilenparameter   
    // stelle Verbindung zum System her
    if(!connectToData(// Frage Datenbankkonfiguration an, 
                      // benötigt zum übersetzten von 
                      // Datenpunktelementen -> DpIdentifier
      StartDpInitSysMsg::TYPE_CONTAINER |
      StartDpInitSysMsg::DP_IDENTIFICATION)) {
      exit(EXIT_FAILURE);
    }
    while (getManagerState() == STATE_INIT) {
      long sec = 1, usec = 0;
      dispatch(sec, usec); // erhalte Daten
    }
    if(!connectToEvent()) {
      exit(EXIT_FAILURE);
    }
  }
};

int main(int argc, char *argv[]) {
  // Parst Konfigurationsdatei und Kommandozeilenparameter
  Resources::init(argc, argv);
  Agent* agt = new Agent();

  // Erstelle eine Liste von Datenpunktelementen
  // die verwendet werden sollen. 
  DpIdentList dpes; DpIdentifier dpe1, dpe2;
  Agent::getId("System1:ExampleDP_DDE.f1:_online.._value", dpe1);
  Agent::getId("System1:ExampleDP_DDE.b1:_online.._value", dpe2);
  dpes.append(dpe1); dpes.append(dpe2);

  // Beispiel API Aufrufe, erhalte Werte
  Agent::dpGet(dpes, new DpObserver(), true);
  // registriere erhalt von Wertänderungen
  Agent::dpConnect(dpes, new DpObserver(), true);

  // setze Werte
  DpIdValueList dpesAndValues;
  Agent::getId("System1:ExampleDP_DDE.f1:_original.._value", dpe1);
  Agent::getId("System1:ExampleDP_DDE.b1:_original.._value", dpe2);
  dpesAndValues.appendItem(dpe1, new FloatVar(47.11));
  dpesAndValues.appendItem(dpe2, new BitVar(false));
  Agent::dpSet(dpesAndValues, new DpObserver(), true);

  // Hauptschleife, kümmert sich um die asynchrone Kommunikation.
  while (true) {
      long sec = 1, usec = 0;
      Agent::dispatch(sec, usec);
  }
  delete agt; agt = nullptr;
  return EXIT_SUCCESS;
}