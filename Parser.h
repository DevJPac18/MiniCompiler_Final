#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "Token.h"
#include "AST.h"

using namespace std;

/*
 * Gramática reconocida (descendente recursiva):
 *
 *   Programa    -> Instruccion { Instruccion }
 *   Instruccion -> Declaracion | Asignacion
 *   Declaracion -> Tipo ID = Expresion ;
 *   Asignacion  -> ID = Expresion ;
 *   Tipo        -> int | float | string
 *   Expresion   -> Termino { ( + | - ) Termino }
 *   Termino     -> Factor { ( * | / ) Factor }
 *   Factor      -> Valor | ( Expresion )
 *   Valor       -> NUM | REAL | STRING | ID
 *
 * STRING representa un literal delimitado por comillas dobles.
 * La regla mínima del enunciado (Declaracion -> Tipo ID = Valor ;) está incluida,
 * porque una Expresion puede ser un único Valor.
 */
class Parser
{
private:
    vector<Token> tokens;
    int posicion;

    vector<string> errores;
    vector<Instruccion> instrucciones;

    // Devuelve true si ya no quedan tokens
    bool esFinal();

    // Devuelve el token que estamos analizando
    Token actual();

    // Comprueba si el token actual es del tipo esperado
    bool coincidir(TipoToken tipo);

    // Devuelve true si el token actual es int, float o string
    bool esTipo();

    // No terminales de nuestra gramática
    bool instruccion(Instruccion &inst);
    bool tipo(string &tipoLeido);
    bool declaracion(Instruccion &inst);
    bool asignacion(Instruccion &inst);
    shared_ptr<Nodo> expresion();
    shared_ptr<Nodo> termino();
    shared_ptr<Nodo> factor();
    shared_ptr<Nodo> valor();

    void error(string esperado);

public:
    Parser(vector<Token> tokens);

    // Inicia el análisis sintáctico. Devuelve true si no hubo errores.
    bool analizar();

    vector<string> obtenerErrores();

    // Instrucciones sintácticamente correctas (entrada del análisis semántico)
    vector<Instruccion> obtenerInstrucciones();
};

#endif
