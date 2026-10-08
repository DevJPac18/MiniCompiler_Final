#ifndef CODIGO_INTERMEDIO_H
#define CODIGO_INTERMEDIO_H

#include <vector>
#include <string>
#include <set>
#include "AST.h"

using namespace std;

// Instrucción de código de tres direcciones:
//   resultado = arg1 op arg2     (operación)
//   resultado = arg1             (copia, cuando op está vacío)
struct Cuadrupla
{
    string resultado;
    string arg1;
    string op;
    string arg2;

    bool resultadoEsTemporal; // true si resultado es t1, t2, ...

    string texto() const
    {
        if (op.empty())
        {
            return resultado + " = " + arg1;
        }
        return resultado + " = " + arg1 + " " + op + " " + arg2;
    }
};

// Código generado para una instrucción del programa fuente
struct BloqueCodigo
{
    Token destino;           // variable que recibe el resultado
    string postfija;         // expresión en notación postfija
    vector<Cuadrupla> codigo;
};

/*
 * Generación de código intermedio en dos pasos:
 *  1. Se convierte la expresión a notación postfija (recorrido postorden del árbol).
 *     Ej.: (a + b) * c  ->  a b + c *
 *  2. Se recorre la postfija con una pila para obtener código de tres direcciones:
 *     - operando: se apila
 *     - operador: se desapilan dos operandos, se genera  tN = x op y  y se apila tN
 *     Al final:  destino = cima de la pila
 */
class GeneradorCodigo
{
private:
    int contadorTemporales;
    set<string> nombresUsados; // variables del programa, para no repetir nombres de temporales

    vector<BloqueCodigo> bloques;

    void aPostfija(shared_ptr<Nodo> nodo, vector<string> &salida);
    string nuevoTemporal();

public:
    GeneradorCodigo();

    void generar(vector<Instruccion> instrucciones);

    vector<BloqueCodigo> obtenerBloques();

    // Todo el código de tres direcciones del programa, en orden
    vector<Cuadrupla> obtenerCodigo();
};

#endif
