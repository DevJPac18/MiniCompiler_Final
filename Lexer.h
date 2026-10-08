#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include "Token.h"

using namespace std;

class Lexer
{
private:

    string codigo;
    int posicion;

    bool esFinal();
    char actual();
    char avanzar();

    void ignorarEspacios();

    Token reconocerNumero();
    Token reconocerIdentificador();
    Token reconocerOperador();
    Token reconocerString();
    Token reconocerSimple(TipoToken tipo);

    bool esPalabraClave(string palabra);

    // Control de errores léxicos
    int linea;
    int columna;

    vector<ErrorLexico> errores;

    void registrarError(
        string lexema,
        int linea,
        int columna,
        string mensaje
    );

public:

    Lexer(string codigoFuente);

    vector<Token> analizar();

    vector<ErrorLexico> obtenerErrores();
};

#endif
