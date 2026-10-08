#ifndef OPTIMIZADOR_H
#define OPTIMIZADOR_H

#include <vector>
#include <string>
#include "CodigoIntermedio.h"

using namespace std;

/*
 * Optimizaciones sencillas sobre el código de tres direcciones:
 *
 *  1. Plegado de constantes: si los dos operandos son números, se calcula en compilación.
 *       t1 = 2 + 3          ->  t1 = 5
 *  2. Simplificación algebraica: x + 0, 0 + x, x - 0, x * 1, 1 * x, x / 1  ->  x
 *  3. Propagación de temporales: si un temporal solo guarda un valor (t1 = 5),
 *     se reemplaza en donde se usa y se elimina su instrucción.
 *       t1 = 5 ; resultado = t1   ->  resultado = 5
 *  4. Eliminación de la copia final: un temporal que solo se copia a la variable
 *     se sustituye por la propia variable.
 *       t2 = t1 * c ; resultado = t2   ->  resultado = t1 * c
 */
class Optimizador
{
private:
    vector<string> cambios; // descripción de cada mejora aplicada (evidencia)

    bool esNumero(string s);
    bool calcular(string a, string op, string b, string &resultado);
    bool simplificar(Cuadrupla &c);

public:
    vector<Cuadrupla> optimizar(vector<Cuadrupla> codigo);

    vector<string> obtenerCambios();
};

#endif
