var text = "Hello, World!";
var bool = false;

bool = PrintParams(text, bool);
PrintParams(text, bool);

for(var i = 0; i < 2; i++)
    PrintParams(text, bool, i);

// wirft exception
PrintParams();