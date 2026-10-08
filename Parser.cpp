#include "Parser.h"
#include <sstream>

using namespace std;
// Constructor
Parser::Parser(vector<Token> tokens)
{

    this->tokens = tokens;
    posicion = 0;
}

// Verifica si terminamos de analizar
bool Parser::esFinal()
{

    return posicion >= (int)tokens.size();
}

// Obtiene el token actual
Token Parser::actual()
{

    return tokens[posicion];
}

// Comprueba si el token actual
// corresponde al tipo esperado
bool Parser::coincidir(TipoToken tipoEsperado)
{
    if (!esFinal() &&
        actual().tipo == tipoEsperado)
    {

        posicion++;

        return true;
    }

    return false;
}

bool Parser::esTipo()
{
    return !esFinal() &&
           actual().tipo == TipoToken::PALABRA_CLAVE &&
           (actual().lexema == "int" ||
            actual().lexema == "float" ||
            actual().lexema == "string");
}

// ---------------------------------
// Tipo → int | float | string
// ---------------------------------

bool Parser::tipo(string &tipoLeido)
{
    if (!esTipo())
    {
        return false;
    }

    tipoLeido = actual().lexema;
    posicion++;

    return true;
}

// ---------------------------------
// Valor → NUM | REAL | STRING | ID
// ---------------------------------

shared_ptr<Nodo> Parser::valor()
{

    if (esFinal())
    {
        return nullptr;
    }

    TipoToken t = actual().tipo;

    if (t == TipoToken::LITERAL_ENTERO ||
        t == TipoToken::LITERAL_REAL ||
        t == TipoToken::LITERAL_STRING ||
        t == TipoToken::IDENTIFICADOR)
    {
        shared_ptr<Nodo> hoja = make_shared<Nodo>();
        hoja->token = actual();
        posicion++;
        return hoja;
    }

    return nullptr;
}

// ---------------------------------
// Factor → Valor | ( Expresion )
// ---------------------------------

shared_ptr<Nodo> Parser::factor()
{
    if (coincidir(TipoToken::PAREN_IZQ))
    {
        shared_ptr<Nodo> interior = expresion();
        if (!interior)
        {
            return nullptr;
        }

        if (!coincidir(TipoToken::PAREN_DER))
        {
            error("')'");
            return nullptr;
        }

        return interior;
    }

    shared_ptr<Nodo> hoja = valor();
    if (!hoja)
    {
        error("NUM, REAL, STRING, IDENTIFICADOR o '('");
        return nullptr;
    }

    return hoja;
}

// ---------------------------------
// Termino → Factor { ( * | / ) Factor }
// ---------------------------------

shared_ptr<Nodo> Parser::termino()
{
    shared_ptr<Nodo> izquierda = factor();
    if (!izquierda)
    {
        return nullptr;
    }

    while (!esFinal() &&
           actual().tipo == TipoToken::OPERADOR &&
           (actual().lexema == "*" || actual().lexema == "/"))
    {
        shared_ptr<Nodo> op = make_shared<Nodo>();
        op->token = actual();
        posicion++;

        shared_ptr<Nodo> derecha = factor();
        if (!derecha)
        {
            return nullptr;
        }

        op->izq = izquierda;
        op->der = derecha;
        izquierda = op;
    }

    return izquierda;
}

// ---------------------------------
// Expresion → Termino { ( + | - ) Termino }
// ---------------------------------

shared_ptr<Nodo> Parser::expresion()
{
    shared_ptr<Nodo> izquierda = termino();
    if (!izquierda)
    {
        return nullptr;
    }

    while (!esFinal() &&
           actual().tipo == TipoToken::OPERADOR &&
           (actual().lexema == "+" || actual().lexema == "-"))
    {
        shared_ptr<Nodo> op = make_shared<Nodo>();
        op->token = actual();
        posicion++;

        shared_ptr<Nodo> derecha = termino();
        if (!derecha)
        {
            return nullptr;
        }

        op->izq = izquierda;
        op->der = derecha;
        izquierda = op;
    }

    return izquierda;
}

// ---------------------------------
// Declaracion → Tipo ID = Expresion ;
// ---------------------------------

bool Parser::declaracion(Instruccion &inst)
{
    inst.esDeclaracion = true;

    // Tipo
    if (!tipo(inst.tipo))
    {
        error("int, float o string");
        return false;
    }

    // ID
    if (esFinal() || actual().tipo != TipoToken::IDENTIFICADOR)
    {
        error("IDENTIFICADOR");
        return false;
    }
    inst.id = actual();
    posicion++;

    // =
    if (!coincidir(TipoToken::ASIGNACION))
    {
        error("'='");
        return false;
    }

    // Expresion (puede ser un único Valor)
    inst.expr = expresion();
    if (!inst.expr)
    {
        return false;
    }

    // ;
    if (!coincidir(TipoToken::DELIMITADOR))
    {
        error("';'");
        return false;
    }

    return true;
}

// ---------------------------------
// Asignacion → ID = Expresion ;
// ---------------------------------

bool Parser::asignacion(Instruccion &inst)
{
    inst.esDeclaracion = false;
    inst.tipo = "";

    // ID
    inst.id = actual();
    posicion++;

    // =
    if (!coincidir(TipoToken::ASIGNACION))
    {
        error("'='");
        return false;
    }

    // Expresion
    inst.expr = expresion();
    if (!inst.expr)
    {
        return false;
    }

    // ;
    if (!coincidir(TipoToken::DELIMITADOR))
    {
        error("';'");
        return false;
    }

    return true;
}

// ---------------------------------
// Instruccion → Declaracion | Asignacion
// ---------------------------------

bool Parser::instruccion(Instruccion &inst)
{
    if (esTipo())
    {
        return declaracion(inst);
    }

    if (actual().tipo == TipoToken::IDENTIFICADOR)
    {
        return asignacion(inst);
    }

    error("int, float, string o IDENTIFICADOR al inicio de la instruccion");
    return false;
}

// ---------------------------------
// Iniciar análisis sintáctico
// Programa → Instruccion { Instruccion }
// ---------------------------------

bool Parser::analizar()
{
    if (tokens.empty())
    {
        errores.push_back("Error sintactico: no hay instrucciones para analizar.");
        return false;
    }

    while (!esFinal())
    {
        int inicio = posicion;
        Instruccion inst;

        if (instruccion(inst))
        {
            instrucciones.push_back(inst);
        }
        else
        {
            // Recuperación en modo pánico: se descartan tokens hasta el ';'
            // o hasta el inicio de otra declaración, y se sigue analizando
            while (!esFinal())
            {
                if (actual().tipo == TipoToken::DELIMITADOR)
                {
                    posicion++;
                    break;
                }
                if (esTipo() && posicion > inicio)
                {
                    break;
                }
                posicion++;
            }
        }
    }

    return errores.empty();
}

void Parser::error(string esperado)
{
    ostringstream mensaje;

    // Si falta el ';' se señala la posición del último token de la instrucción,
    // porque el siguiente token suele estar ya en otra línea
    if (esperado == "';'" && posicion > 0)
    {
        Token anterior = tokens[posicion - 1];
        mensaje << "Error sintactico en linea "
                << anterior.linea
                << ", columna "
                << anterior.columna + (int)anterior.lexema.length()
                << ": se esperaba ';' despues de '"
                << anterior.lexema
                << "'.";
    }
    else if (!esFinal())
    {
        mensaje << "Error sintactico en linea "
                << actual().linea
                << ", columna "
                << actual().columna
                << ": se esperaba "
                << esperado
                << ", pero se encontro '"
                << actual().lexema
                << "'.";
    }
    else
    {
        mensaje << "Error sintactico: "
                << "se esperaba "
                << esperado
                << ", pero se llego al final de la entrada.";
    }

    errores.push_back(mensaje.str());
}

vector<string> Parser::obtenerErrores()
{
    return errores;
}

vector<Instruccion> Parser::obtenerInstrucciones()
{
    return instrucciones;
}
