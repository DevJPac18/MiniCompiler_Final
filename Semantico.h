#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <vector>
#include <string>
#include "AST.h"

using namespace std;

// Entrada de la tabla de símbolos
struct Simbolo
{
    string nombre;
    string tipo;    // int | float | string

    int linea;      // dónde fue declarada
    int columna;
};

/*
 * Reglas semánticas implementadas:
 *  1. Una variable no puede declararse dos veces.
 *  2. Un identificador usado como valor debe estar declarado previamente.
 *  3. A una variable solo se le puede asignar si fue declarada.
 *  4. Compatibilidad de tipos:
 *       int    <- int
 *       float  <- int | float   (int se convierte a float)
 *       string <- string
 *     Los operadores + - * / solo se aplican a int y float.
 *     int (op) int = int ; si algún operando es float, el resultado es float.
 */
class Semantico
{
private:
    vector<Simbolo> tablaSimbolos;
    vector<string> errores;
    vector<Instruccion> instruccionesValidas;

    // Devuelve el índice del símbolo en la tabla, o -1 si no existe
    int buscar(string nombre);

    void registrar(string nombre, string tipo, int linea, int columna);

    // Calcula el tipo de una expresión: int, float, string o "error"
    string tipoExpresion(shared_ptr<Nodo> nodo);

    bool sonCompatibles(string tipoDestino, string tipoOrigen);

    void error(int linea, int columna, string mensaje);

public:
    void analizar(vector<Instruccion> instrucciones);

    vector<Simbolo> obtenerTablaSimbolos();
    vector<string> obtenerErrores();

    // Instrucciones que pasaron el análisis semántico (entrada del generador de código)
    vector<Instruccion> obtenerInstruccionesValidas();
};

#endif
