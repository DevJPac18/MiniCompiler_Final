#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include "Token.h"

using namespace std;

// Nodo del árbol de una expresión.
// Las hojas son valores (NUM, REAL, STRING, ID) y los nodos internos son operadores.
struct Nodo
{
    Token token;            // token que representa el nodo (valor u operador)
    shared_ptr<Nodo> izq;   // operando izquierdo (solo operadores)
    shared_ptr<Nodo> der;   // operando derecho (solo operadores)

    bool esOperador() const
    {
        return token.tipo == TipoToken::OPERADOR;
    }
};

// Instrucción reconocida por el Parser:
//   Declaracion -> Tipo ID = Expresion ;
//   Asignacion  -> ID = Expresion ;
struct Instruccion
{
    bool esDeclaracion;     // true: "int x = ...;"   false: "x = ...;"
    string tipo;            // int | float | string (solo en declaraciones)
    Token id;               // variable que recibe el valor
    shared_ptr<Nodo> expr;  // expresión del lado derecho
};

#endif
