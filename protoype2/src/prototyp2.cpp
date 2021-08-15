/**
 * @author Daniel Giritzer
 * @file prototyp2.cpp
 * @brief Einfaches Beispiel, soll die Funktionsweise
 * der Google V8 API veranschaulichen.
 * @copyright Daniel Giritzer 2021. All Rights Reserved.
 */
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

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

// Einfach Testfunktion, soll via Javascript ausführbar sein
void SayHi(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Isolate::Scope iscope(isolate);
  v8::HandleScope hscope(isolate);

  if (args.Length() == 1 && args[0]->IsString()) {
      v8::String::Utf8Value param1(args[0]->ToString());
      std::string str = std::string(*param1);
      std::cout << "SayHi: " << str << std::endl;
      args.GetReturnValue().Set(v8::Boolean::New(isolate, false));
  }
  else if(args.Length() == 0) { 
    std::cout << "SayHi, without parameters" << std::endl;
    args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
  }
  else { // Werfe exception
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Invalid parameters")));
  }
}

int main(int argc, char *argv[]) {

  // Lade übergebene datei
  std::ifstream inFile(argv[argc-1]);
  if (!inFile.good()) {
    std::cerr << "Unable to open file " << argv[argc-1];
    return EXIT_FAILURE;
  }
  std::stringstream source;
  source << inFile.rdbuf();
  std::string srcStr = source.str();

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
    global->Set(v8::String::NewFromUtf8(isolate, "SayHi", v8::NewStringType::kNormal).ToLocalChecked(), 
                                        v8::FunctionTemplate::New(isolate, SayHi));
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

  // Gebe Resourcen frei
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();

  return EXIT_SUCCESS;
}