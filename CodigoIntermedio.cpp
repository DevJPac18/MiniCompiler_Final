#include "CodigoIntermedio.h"
#include <stack>
#include <sstream>

using namespace std;

GeneradorCodigo::GeneradorCodigo()
{
    contadorTemporales = 0;
}

string GeneradorCodigo::nuevoTemporal()
{
    string nombre;
    do
    {
        contadorTemporales++;
        ostringstream texto;
        texto << "t" << contadorTemporales;
        nombre = texto.str();
    } while (nombresUsados.count(nombre) > 0); // evita chocar con una variable llamada t1, t2...

    return nombre;
}

// ---------------------------------
// Conversión a notación postfija (postorden: izquierda, derecha, raíz)
// ---------------------------------

void GeneradorCodigo::aPostfija(shared_ptr<Nodo> nodo, vector<string> &salida)
{
    if (nodo->esOperador())
    {
        aPostfija(nodo->izq, salida);
        aPostfija(nodo->der, salida);
    }
    salida.push_back(nodo->token.lexema);
}

void GeneradorCodigo::generar(vector<Instruccion> instrucciones)
{
    for (Instruccion inst : instrucciones)
    {
        nombresUsados.insert(inst.id.lexema);
    }

    for (Instruccion inst : instrucciones)
    {
        BloqueCodigo bloque;
        bloque.destino = inst.id;

        // Paso 1: expresión -> postfija
        vector<string> postfija;
        aPostfija(inst.expr, postfija);

        for (int i = 0; i < (int)postfija.size(); i++)
        {
            bloque.postfija += (i > 0 ? " " : "") + postfija[i];
        }

        // Paso 2: postfija -> código de tres direcciones usando una pila
        stack<string> pila;

        for (string elemento : postfija)
        {
            bool esOperador = elemento == "+" || elemento == "-" ||
                              elemento == "*" || elemento == "/";

            if (!esOperador)
            {
                pila.push(elemento);
                continue;
            }

            string derecho = pila.top();
            pila.pop();
            string izquierdo = pila.top();
            pila.pop();

            Cuadrupla c;
            c.resultado = nuevoTemporal();
            c.arg1 = izquierdo;
            c.op = elemento;
            c.arg2 = derecho;
            c.resultadoEsTemporal = true;
            bloque.codigo.push_back(c);

            pila.push(c.resultado);
        }

        // destino = resultado final de la expresión
        Cuadrupla asignacion;
        asignacion.resultado = inst.id.lexema;
        asignacion.arg1 = pila.top();
        asignacion.op = "";
        asignacion.arg2 = "";
        asignacion.resultadoEsTemporal = false;
        bloque.codigo.push_back(asignacion);

        bloques.push_back(bloque);
    }
}

vector<BloqueCodigo> GeneradorCodigo::obtenerBloques()
{
    return bloques;
}

vector<Cuadrupla> GeneradorCodigo::obtenerCodigo()
{
    vector<Cuadrupla> todo;
    for (BloqueCodigo bloque : bloques)
    {
        for (Cuadrupla c : bloque.codigo)
        {
            todo.push_back(c);
        }
    }
    return todo;
}
