var ffi = require('ffi');
var ref = require('ref');
// Lade je nach plattform .dll oder .so
var libpath = "./libprototyp4.so"
if(process.platform === "win32") {
  libpath = "./prototyp4.dll";
}
// Marshalling der Funktionen
var lib = ffi.Library(libpath, {
    'PROTO_New': [ 'void', [ ref.refType('void*') ] ],
    'PROTO_Greet': [ 'void', [ 'void*', 'string' ] ],
    'PROTO_Delete': [ 'void', [ref.refType('void*')] ]
});
// Ausführen des nativen codes
var greeter = ref.alloc('void*');
lib.PROTO_New(greeter);
lib.PROTO_Greet(greeter, "ESD");
lib.PROTO_Delete(greeter);
