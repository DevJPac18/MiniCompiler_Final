#include "Semantico.h"
#include <sstream>

using namespace std;

int Semantico::buscar(string nombre)
{
    for (int i = 0; i < (int)tablaSimbolos.size(); i++)
    {
        if (tablaSimbolos[i].nombre == nombre)
        {
            return i;
        }
    }
    return -1;
}

void Semantico::registrar(string nombre, string tipo, int linea, int columna)
{
    Simbolo simbolo;
    simbolo.nombre = nombre;
    simbolo.tipo = tipo;
    simbolo.linea = linea;
    simbolo.columna = columna;
    tablaSimbolos.push_back(simbolo);
}

void Semantico::error(int linea, int columna, string mensaje)
{
    ostringstream texto;
    texto << "Error semantico en linea " << linea
          << ", columna " << columna
          << ": " << mensaje;
    errores.push_back(texto.str());
}

// ---------------------------------
// Tipo de una expresión
// ---------------------------------

string Semantico::tipoExpresion(shared_ptr<Nodo> nodo)
{
    Token t = nodo->token;

    switch (t.tipo)
    {
    case TipoToken::LITERAL_ENTERO:
        return "int";

    case TipoToken::LITERAL_REAL:
        return "float";

    case TipoToken::LITERAL_STRING:
        return "string";

    case TipoToken::IDENTIFICADOR:
    {
        // Regla 2: el identificador debe estar declarado previamente
        int indice = buscar(t.lexema);
        if (indice == -1)
        {
            error(t.linea, t.columna,
                  "la variable '" + t.lexema + "' no ha sido declarada.");
            return "error";
        }
        return tablaSimbolos[indice].tipo;
    }

    case TipoToken::OPERADOR:
    {
        string izq = tipoExpresion(nodo->izq);
        string der = tipoExpresion(nodo->der);

        if (izq == "error" || der == "error")
        {
            return "error";
        }

        // Los operadores aritméticos no se aplican a string
        if (izq == "string" || der == "string")
        {
            error(t.linea, t.columna,
                  "el operador '" + t.lexema +
                      "' no se puede aplicar a operandos de tipo string.");
            return "error";
        }

        if (izq == "float" || der == "float")
        {
            return "float";
        }
        return "int";
    }

    default:
        return "error";
    }
}

bool Semantico::sonCompatibles(string tipoDestino, string tipoOrigen)
{
    if (tipoDestino == tipoOrigen)
    {
        return true;
    }

    // Conversión implícita permitida: int -> float
    if (tipoDestino == "float" && tipoOrigen == "int")
    {
        return true;
    }

    return false;
}

// ---------------------------------
// Recorre las instrucciones y aplica las reglas
// ---------------------------------

void Semantico::analizar(vector<Instruccion> instrucciones)
{
    for (Instruccion inst : instrucciones)
    {
        int erroresAntes = errores.size();
        string nombre = inst.id.lexema;
        string tipoDestino;

        if (inst.esDeclaracion)
        {
            // Regla 1: declaración duplicada
            int indice = buscar(nombre);
            if (indice != -1)
            {
                ostringstream mensaje;
                mensaje << "la variable '" << nombre
                        << "' ya fue declarada en la linea "
                        << tablaSimbolos[indice].linea << ".";
                error(inst.id.linea, inst.id.columna, mensaje.str());
            }
            tipoDestino = inst.tipo;
        }
        else
        {
            // Regla 3: solo se asigna a variables declaradas
            int indice = buscar(nombre);
            if (indice == -1)
            {
                error(inst.id.linea, inst.id.columna,
                      "no se puede asignar a '" + nombre +
                          "' porque no ha sido declarada.");
                tipoDestino = "error";
            }
            else
            {
                tipoDestino = tablaSimbolos[indice].tipo;
            }
        }

        // Regla 4: compatibilidad de tipos
        string tipoOrigen = tipoExpresion(inst.expr);

        if (tipoDestino != "error" && tipoOrigen != "error" &&
            !sonCompatibles(tipoDestino, tipoOrigen))
        {
            error(inst.id.linea, inst.id.columna,
                  "no se puede asignar un valor de tipo " + tipoOrigen +
                      " a la variable '" + nombre + "' de tipo " + tipoDestino + ".");
        }

        bool esValida = (int)errores.size() == erroresAntes;

        // Se registra el nombre y tipo solo después de una declaración válida
        if (esValida && inst.esDeclaracion)
        {
            registrar(nombre, inst.tipo, inst.id.linea, inst.id.columna);
        }

        if (esValida)
        {
            instruccionesValidas.push_back(inst);
        }
    }
}

vector<Simbolo> Semantico::obtenerTablaSimbolos()
{
    return tablaSimbolos;
}

vector<string> Semantico::obtenerErrores()
{
    return errores;
}

vector<Instruccion> Semantico::obtenerInstruccionesValidas()
{
    return instruccionesValidas;
}
