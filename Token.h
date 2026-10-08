#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

enum class TipoToken
{
    LITERAL_ENTERO,
    LITERAL_REAL,
    LITERAL_STRING, // "texto" (obligatoriamente entre comillas dobles)
    IDENTIFICADOR,
    DESCONOCIDO,

    PALABRA_CLAVE, // int, float, string
    OPERADOR,      // +, -, *, /
    ASIGNACION,    // =
    DELIMITADOR,   // ;
    PAREN_IZQ,     // (
    PAREN_DER      // )
};

struct Token
{
    TipoToken tipo;
    string lexema;

    int linea;
    int columna;
};

struct ErrorLexico
{
    string lexema;

    int linea;
    int columna;

    string mensaje;
};

#endif
