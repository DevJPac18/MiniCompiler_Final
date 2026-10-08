@echo off
REM Compila el proyecto y ejecuta todos los casos de prueba.
REM Cada salida se guarda en pruebas\salidas\

cd /d "%~dp0"

g++ -std=c++11 -o compilador.exe main.cpp Lexer.cpp Parser.cpp Semantico.cpp CodigoIntermedio.cpp Optimizador.cpp
if errorlevel 1 (
    echo Error al compilar.
    exit /b 1
)

if not exist pruebas\salidas mkdir pruebas\salidas

for %%f in (pruebas\*.txt) do (
    echo Ejecutando %%~nf ...
    .\compilador.exe %%f > pruebas\salidas\%%~nf_salida.txt
)

echo.
echo Listo. Las salidas estan en pruebas\salidas\
