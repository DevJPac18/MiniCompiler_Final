# Mini compilador – Compiladores

Mini compilador en C++ que recorre las fases del proceso de traducción:

```
Codigo fuente -> Lexer -> Parser -> Analisis semantico -> Codigo intermedio -> Optimizacion
```

## Gramática

```
Programa    -> Instruccion { Instruccion }
Instruccion -> Declaracion | Asignacion
Declaracion -> Tipo ID = Expresion ;
Asignacion  -> ID = Expresion ;
Tipo        -> int | float | string
Expresion   -> Termino { ( + | - ) Termino }
Termino     -> Factor { ( * | / ) Factor }
Factor      -> Valor | ( Expresion )
Valor       -> NUM | REAL | STRING | ID
```

`STRING` es un literal delimitado **obligatoriamente por comillas dobles**: `"10.5"` es un string, no un float.

## Archivos

| Archivo | Fase |
|---|---|
| `Token.h`, `Lexer.h/.cpp` | Análisis léxico: tokens, literales string, varias líneas, lista de errores léxicos |
| `AST.h`, `Parser.h/.cpp` | Análisis sintáctico descendente recursivo con recuperación de errores (modo pánico) |
| `Semantico.h/.cpp` | Tabla de símbolos, duplicados, no declarados y compatibilidad de tipos |
| `CodigoIntermedio.h/.cpp` | Conversión a notación postfija y código de tres direcciones (t1, t2, ...) |
| `Optimizador.h/.cpp` | Plegado de constantes, simplificación algebraica, propagación y eliminación de copias |
| `main.cpp` | Integra todas las fases y muestra los resultados |
| `pruebas/` | Casos de prueba |

## Compilación

Requiere `g++` con soporte para C++11.

```
g++ -std=c++11 -o compilador main.cpp Lexer.cpp Parser.cpp Semantico.cpp CodigoIntermedio.cpp Optimizador.cpp
```

## Ejecución

Desde un archivo:

```
./compilador pruebas/caso8_codigo_intermedio.txt
```

Por teclado (escribir el código en varias líneas y terminar con una línea `FIN`):

```
./compilador
```

### En Visual Studio Code

Abrir la carpeta del proyecto (`Archivo > Abrir carpeta`). La carpeta `.vscode` ya trae la configuración:

- `Ctrl+Shift+B`: compila todo el proyecto en `compilador.exe`.
- `Terminal > Ejecutar tarea...`:
  - **Ejecutar (escribir codigo por teclado)**
  - **Ejecutar caso de prueba** (permite elegir uno de la carpeta `pruebas`)
  - **Ejecutar archivo abierto** (analiza el `.txt` abierto en el editor)
  - **Ejecutar todas las pruebas**
- `F5`: compila y ejecuta en una consola aparte (permite usar puntos de interrupción).

Con la extensión "C/C++ Compile Run", el botón ▶ también funciona (`.vscode/settings.json` le agrega los demás `.cpp`), pero debe pulsarse **con `main.cpp` abierto**.

### Script de pruebas

En Windows se puede ejecutar `ejecutar_pruebas.bat`, que compila el proyecto, corre todos los casos y guarda cada salida en `pruebas/salidas/`.

## Casos de prueba

| Archivo | Qué demuestra |
|---|---|
| `caso1_int_float.txt` | Declaraciones válidas de int y float |
| `caso2_string.txt` | `string nombre = "Ana";` y strings con números (`"10.5"`, `"1"`) |
| `caso3_duplicada.txt` | Variable declarada dos veces |
| `caso4_no_declarada.txt` | Uso de un identificador no declarado |
| `caso5_error_sintactico.txt` | Errores sintácticos y recuperación |
| `caso6_error_lexico.txt` | Error léxico seguido de instrucciones válidas (el análisis continúa) |
| `caso7_varias_lineas.txt` | Código de varias líneas |
| `caso8_codigo_intermedio.txt` | Expresión con postfija y código de tres direcciones |
| `caso9_optimizacion.txt` | Optimización con código antes y después |
| `caso10_tipos.txt` | Validación de compatibilidad de tipos |
