#include "Lexer.h"
#include <cctype>

Lexer::Lexer(string _codigo)
{
    codigo = _codigo;
    posicion = 0;
    linea = 1;
    columna = 1;
}

bool Lexer::esFinal()
{
    return posicion >= (int)codigo.length();
}

char Lexer::actual()
{
    return codigo[posicion];
}

char Lexer::avanzar()
{
    char caracter = codigo[posicion];
    posicion++;
    // controla salto de linea
    if (caracter == '\n')
    {
        linea++;
        columna = 1;
    }
    else
    {
        columna++;
    }

    return caracter;
}

void Lexer::ignorarEspacios()
{
    while (!esFinal() && isspace((unsigned char)actual()))
    {
        avanzar();
    }
}

Token Lexer::reconocerNumero()
{
    string lexema = "";

    int lineaInicio = linea;
    int columnaInicio = columna;

    int cantidadPuntos = 0;

    while (!esFinal())
    {
        if (isdigit((unsigned char)actual()))
        {
            lexema += avanzar();
        }
        else if (actual() == '.')
        {
            cantidadPuntos++;
            lexema += avanzar();
        }
        else
        {
            break;
        }
    }

    // Un número con más de un punto (ej. 1.2.3) o terminado en punto (ej. 10.)
    // es un número mal formado: se registra el error y se sigue analizando
    if (cantidadPuntos > 1 || lexema[lexema.length() - 1] == '.')
    {
        registrarError(lexema, lineaInicio, columnaInicio, "Numero mal formado");
        return {
            TipoToken::DESCONOCIDO,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    if (cantidadPuntos == 1)
    {
        return {
            TipoToken::LITERAL_REAL,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    return {
        TipoToken::LITERAL_ENTERO,
        lexema,
        lineaInicio,
        columnaInicio};
}

Token Lexer::reconocerIdentificador()
{
    string lexema = "";

    int lineaInicio = linea;
    int columnaInicio = columna;

    while (!esFinal() &&
           (isalnum((unsigned char)actual()) || actual() == '_'))
    {
        lexema += avanzar();
    }

    if (esPalabraClave(lexema))
    {
        return {
            TipoToken::PALABRA_CLAVE,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    return {
        TipoToken::IDENTIFICADOR,
        lexema,
        lineaInicio,
        columnaInicio};
}

Token Lexer::reconocerOperador()
{
    return reconocerSimple(TipoToken::OPERADOR);
}

// Reconoce un token de un solo carácter: =, ;, (, ), +, -, *, /
Token Lexer::reconocerSimple(TipoToken tipo)
{
    int lineaInicio = linea;
    int columnaInicio = columna;

    string lexema = "";
    lexema += avanzar();

    return {
        tipo,
        lexema,
        lineaInicio,
        columnaInicio};
}

// ---------------------------------
// STRING -> " { caracter distinto de " y salto de linea } "
// ---------------------------------
// El lexema se guarda con sus comillas: "10.5" es un string, no un float.
Token Lexer::reconocerString()
{
    int lineaInicio = linea;
    int columnaInicio = columna;

    string lexema = "";
    lexema += avanzar(); // comilla de apertura

    while (!esFinal() && actual() != '"' && actual() != '\n')
    {
        lexema += avanzar();
    }

    // Si no se encontró la comilla de cierre en la misma línea, es un error léxico
    if (esFinal() || actual() != '"')
    {
        registrarError(lexema, lineaInicio, columnaInicio,
                       "String sin cerrar (falta la comilla doble de cierre)");
        return {
            TipoToken::DESCONOCIDO,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    lexema += avanzar(); // comilla de cierre

    return {
        TipoToken::LITERAL_STRING,
        lexema,
        lineaInicio,
        columnaInicio};
}

bool Lexer::esPalabraClave(string palabra)
{
    return palabra == "int" || palabra == "float" || palabra == "string" || palabra == "if" || palabra == "else";
}

// error lexico
vector<ErrorLexico> Lexer::obtenerErrores()
{
    return errores;
}

void Lexer::registrarError(string lexema, int lineaError, int columnaError, string mensaje)
{
    ErrorLexico error;
    error.lexema = lexema;
    error.linea = lineaError;
    error.columna = columnaError;
    error.mensaje = mensaje;
    errores.push_back(error);
}

vector<Token> Lexer::analizar()
{
    vector<Token> tokens;
    while (!esFinal())
    {
        ignorarEspacios();
        if (esFinal())
        {
            break;
        }
        char c = actual();

        // Literal entero o real
        if (isdigit((unsigned char)c))
        {
            tokens.push_back(reconocerNumero());
        }

        // Identificador o palabra clave
        else if (isalpha((unsigned char)c) || c == '_')
        {
            tokens.push_back(reconocerIdentificador());
        }

        // Literal string entre comillas dobles
        else if (c == '"')
        {
            tokens.push_back(reconocerString());
        }

        // Operadores aritméticos
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            tokens.push_back(reconocerOperador());
        }

        // Asignación
        else if (c == '=')
        {
            tokens.push_back(reconocerSimple(TipoToken::ASIGNACION));
        }

        // Delimitador
        else if (c == ';')
        {
            tokens.push_back(reconocerSimple(TipoToken::DELIMITADOR));
        }

        // Paréntesis (para expresiones aritméticas)
        else if (c == '(')
        {
            tokens.push_back(reconocerSimple(TipoToken::PAREN_IZQ));
        }

        else if (c == ')')
        {
            tokens.push_back(reconocerSimple(TipoToken::PAREN_DER));
        }

        // Comilla simple: los strings deben ir obligatoriamente entre comillas dobles
        else if (c == '\'')
        {
            int lineaError = linea;
            int columnaError = columna;

            string lexema = "";
            lexema += avanzar();
            while (!esFinal() && actual() != '\'' && actual() != '\n')
            {
                lexema += avanzar();
            }
            if (!esFinal() && actual() == '\'')
            {
                lexema += avanzar();
            }

            registrarError(lexema, lineaError, columnaError,
                           "Los strings deben escribirse entre comillas dobles");
            tokens.push_back({TipoToken::DESCONOCIDO,
                              lexema,
                              lineaError,
                              columnaError});
        }

        // Carácter desconocido - error lexico
        // Se registra el error y se continúa con el siguiente carácter
        else
        {
            int lineaError = linea;
            int columnaError = columna;

            string lexema = "";
            lexema += avanzar();

            registrarError(lexema, lineaError, columnaError, "Caracter no reconocido");
            tokens.push_back({TipoToken::DESCONOCIDO,
                              lexema,
                              lineaError,
                              columnaError});
        }
    }
    return tokens;
}
