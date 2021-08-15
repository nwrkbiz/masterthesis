// Beispiel Javascript, ausgeführt von prototyp3
var i = 0;
DpConnect("System1:ExampleDP_DDE.f1:_online.._value", function(dpes, vars){
    PrintParams(dpes, vars, i);
    i++;
})