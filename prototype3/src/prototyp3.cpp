/**
 * @author Daniel Giritzer
 * @file prototyp3.cpp
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

#include <v8.h>
#include <libplatform/libplatform.h>

// Hilfsfunktion, gibt V8 Exceptions aus
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch)
{
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exceptionV8(try_catch->Exception());
  std::string exception = *exceptionV8;

  v8::Local<v8::Message> message = try_catch->Message();
  
  // Gebe nur Exception aus, falls keine weiteren Infos vorhanden
  if (message.IsEmpty()) {
    std::cout << exception << std::endl;
  }
  else {
    // Dateiname
    v8::String::Utf8Value filenameV8(message->GetScriptOrigin().ResourceName());
    std::string fileName = *filenameV8;

    // Zeilennummer
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    int linenum = message->GetLineNumber(context).FromJust();

    // Zeileninhalt
    v8::String::Utf8Value sourcelineV8(message->GetSourceLine(context).ToLocalChecked());
    std::string sourceLine = *sourcelineV8;

    // Schreibe Dateinamen, Zeilennummer und Zeileninhalt
    std::cout << fileName << ":" << linenum << std::endl;
    std::cout << sourceLine << std::endl;

    // Schreibe Stacktrace, falls vorhanden
    v8::Local<v8::Value> stackTraceV8;
    if (try_catch->StackTrace(context).ToLocal(&stackTraceV8) &&
        stackTraceV8->IsString() &&
        v8::Local<v8::String>::Cast(stackTraceV8)->Length() > 0) {

      v8::String::Utf8Value stackTraceV8String(stackTraceV8);
      std::string stackTrace = *stackTraceV8String;
      std::cout << stackTrace << std::endl;
    }
  }
}

// Diese Klasse erhält callbacks vom WinCC OA Framework
class DpObserver : public HotLinkWaitForAnswer {
public:
  DpObserver(v8::Isolate* iso,
    const v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>& fun,
    v8::Global<v8::Context>& ctx) : 
    m_Isolate(iso), m_Function(fun),
    m_Context(std::move(ctx)) {};
  // Diese Funktion wird bei jeder Wertänderung aufgerufen
  void hotLinkCallBack(DpHLGroup &group) override {
    std::cout << "- hotLinkCallBack(DpHLGroup), Execute JS Function -" << std::endl;
    v8::Locker lock(m_Isolate); v8::Isolate::Scope isolate_scope(m_Isolate);
    v8::HandleScope handle_scope(m_Isolate);
    v8::Local<v8::Context> context = m_Context.Get(m_Isolate);
    v8::Context::Scope context_scope(context);
    // Marshalling von WinCC OA Datentypen nach V8 Javascript Datentypen
    const int argc = 2;
    v8::Local<v8::Value> argv[argc];
    v8::Local<v8::Array> dpeArr = v8::Array::New(m_Isolate);
    v8::Local<v8::Array> varArr = v8::Array::New(m_Isolate);
    uint32_t num = 0;
    for (DpVCItem* item = group.getFirstItem(); item; item = group.getNextItem()) {
      Variable* value = item->getValuePtr(); // Neuer Wert
      CharString dpe = item->getDpIdentifier().toString(); // Name des DPElement
      dpeArr->Set(num, v8::String::NewFromUtf8(m_Isolate, dpe));
      if(value->isA(VariableType::BIT_VAR) == VariableType::BIT_VAR){
        varArr->Set(num, v8::Boolean::New(m_Isolate, static_cast<BitVar*>(value)->getValue()));
      } else if (value->isA(VariableType::FLOAT_VAR) == VariableType::FLOAT_VAR) {
        varArr->Set(num, v8::Number::New(m_Isolate, static_cast<FloatVar*>(value)->getValue()));
      } else {
        varArr->Set(num, v8::Null(m_Isolate));
      }
      num += 1;
    }
    argv[0] = dpeArr; argv[1] = varArr;
    v8::TryCatch tryCatch(m_Isolate);
    v8::Local<v8::Function> f = v8::Local<v8::Function>::New(m_Isolate, m_Function);
    // Falls der Javascript Functionsscope (in js) über .bind gesetzt wurde verwende
    // diesen, ansonsten verwende das global Objekt des Ausführungskontext
    v8::Local<v8::Object> fScope = f->GetBoundFunction().As<v8::Object>();
    if(fScope == v8::Undefined(m_Isolate)) fScope = context->Global();
    // rufe JS Funktion auf, übergebe dpes als param1 und werte als param2
    f->Call(Scope, argc, argv);
    if (tryCatch.HasCaught()) ReportException(m_Isolate, &tryCatch);
  }
  void hotLinkCallBack(DpMsgAnswer &answer) override {}  // Stub, nicht verwendet
private:
  v8::Isolate* m_Isolate;
  v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> m_Function;
  v8::Global<v8::Context> m_Context;
};

class Agent : public Manager {
public:
    Agent() : 

    // Rufe CTor der Elternklasse auf,
    // Jeder Manager muss eindeutig
    // über den ManagerIdentifier identifizierbar sein
    Manager(ManagerIdentifier(
            ManagerType::API_MAN,
            Resources::getManNum())) // verwendet geparsten '-num' Kommandozeilenparameter
    
    {
        // stelle Verbindung zum System her
        if(!connectToData( // Frage Datenbankkonfiguration an, 
                           // benötigt zum übersetzten von 
                           // Datenpunktelementen -> DpIdentifier
            StartDpInitSysMsg::TYPE_CONTAINER |
            StartDpInitSysMsg::DP_IDENTIFICATION)) {
            exit(EXIT_FAILURE);
        }
        while (getManagerState() == STATE_INIT) {
            long sec = 1, usec = 0;
            dispatch(sec, usec);
        }
        if(!connectToEvent()) {
            exit(EXIT_FAILURE);
        }
    }
};

// DpConnect für Javascript
void DpConnect(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Isolate::Scope iscope(isolate);
  v8::HandleScope hscope(isolate);
  if (args.Length() == 2 && args[0]->IsString() && args[1]->IsFunction()) {
    v8::String::Utf8Value param1(args[0]->ToString());
    CharString str(*param1); // Marshalling V8 -> WinCC OA
    DpIdentifier dpe;
    Agent::getId(str, dpe);
    // JS Callback-Funktion als Persistente Variable (->Schutz vor Garbage Collector)
    v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> callback;
    callback.Reset(isolate, args[1].As<v8::Function>());
    // Ausführungscontext als Globales V8 Objekt, wird benötigt 
    // um diesen context zu einem späteren Zeitpunkt wieder verwenden
    // zu können.
    v8::Global<v8::Context> context;
    context.Reset(isolate, isolate->GetCurrentContext());
    // registriere erhalt von Wertänderungen, bei änderungen soll
    // die Javascript Funktion aufgerufen werden.
    bool ret = Agent::dpConnect(dpe, new DpObserver(isolate, callback, context), true);
    args.GetReturnValue().Set(v8::Boolean::New(isolate, ret)); // Javascript Rückgabewert
  }
  else { // Werfe exception
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Invalid parameters, Expected: string dpe, function callback")));
  }
}

// Einfach Testfunktion, soll via Javascript ausführbar sein
void PrintParams(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Isolate::Scope iscope(isolate);
  v8::HandleScope hscope(isolate);

  if (args.Length() > 0) {
    for (int i = 0; i < args.Length(); i++) {
      v8::String::Utf8Value str(args[i]);
      std::cout << *str << std::endl;
    }
    args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
  }
  else { // Werfe exception
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Invalid parameters")));
  }
}

int main(int argc, char *argv[]) {

    // Lade übergebene datei (letzter parameter)
    std::ifstream inFile(argv[argc-1]);
    if (!inFile.good()) {
      std::cerr << "Unable to open file " << argv[argc-1];
      return EXIT_FAILURE;
    }
    std::stringstream source;
    source << inFile.rdbuf();
    std::string srcStr = source.str();

    // Parse Konfigurationsdatei und Kommandozeilenparameter (WinCC OA)
    Resources::init(argc, argv);
    Agent* agt = new Agent(); // erstelle neuen Agent 

    // Initialisiere V8 Engine
    v8::V8::InitializePlatform(v8::platform::CreateDefaultPlatform());
    v8::V8::Initialize();

    // Erstelle ein neues Isolate, benutze den Standartallocator für Arrays
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);

    // Eigener Scope in dem das Isolate verwendet wird
    { 
      v8::Locker locker(isolate); // Threadschutz/locking
      v8::Isolate::Scope isolate_scope(isolate); // Scope gehört isolate
      v8::HandleScope handle_scope(isolate); // Handles in diesem Scope gehören isolate


      // Erstelle ein leeres global Objekttemplate für einen neuen Ausführungskontext
      v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

      // Füge eine funktion mit C/C++ code zu diesem Objekttemplate hinzu
global->Set(v8::String::NewFromUtf8(isolate, "DpConnect", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(isolate, DpConnect));
      global->Set(v8::String::NewFromUtf8(isolate, "PrintParams", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(isolate, PrintParams));
      // Erstelle einen Azsführungskontext, verwende das zuvor erstellte global Objekttemplate für das
      // global Objekt.
      v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);

      // Scope gehört context
      v8::Context::Scope context_scope(context);

      // Wandle Skript string in v8 Quellcode um
      const v8::Local<v8::String> raw_script(v8::String::NewFromUtf8(isolate, srcStr.c_str()));
      v8::ScriptCompiler::Source v8src(raw_script);

      // Kompiliere Skript für context
      v8::TryCatch tryCatchCompile(isolate); // tryCath objekt, fängt exception
      v8::Local<v8::Script> script = v8::ScriptCompiler::Compile(isolate, &v8src, v8::ScriptCompiler::kProduceCodeCache);

      if (tryCatchCompile.HasCaught()) { // Gebe Compilefehler aus
        ReportException(isolate, &tryCatchCompile);
        return EXIT_FAILURE;
      }
      else { // Führe Skript aus
        v8::TryCatch tryCatchRun(isolate);
        v8::Local<v8::Value> result;
        bool ret = script->Run(context).ToLocal(&result);
        if (tryCatchRun.HasCaught() || !ret){ // Gebe Laufzeitfehler aus
          ReportException(isolate, &tryCatchRun);
          return EXIT_FAILURE;
        }
      }

    } // Verlasse Scope -> isolate ist nun nicht mehr in Verwendung

    // Hauptschleife, kümmert sich um die asynchrone Kommunikation
    while (true) {
        long sec = 1, usec = 0;
        Agent::dispatch(sec, usec);
    }
    delete agt; agt = nullptr;

    // Gebe V8 Resourcen frei
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();

    return EXIT_SUCCESS;
}