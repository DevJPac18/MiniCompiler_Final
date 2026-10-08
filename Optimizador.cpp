#include "Optimizador.h"
#include <map>
#include <sstream>
#include <iomanip>
#include <cctype>

using namespace std;

// Un número es una secuencia de dígitos con como máximo un punto (los strings llevan comillas)
bool Optimizador::esNumero(string s)
{
    if (s.empty())
    {
        return false;
    }

    int puntos = 0;
    for (char c : s)
    {
        if (c == '.')
        {
            puntos++;
        }
        else if (!isdigit((unsigned char)c))
        {
            return false;
        }
    }
    return puntos <= 1;
}

// Calcula "a op b" cuando ambos son números. Devuelve false si no se puede (división entre 0).
bool Optimizador::calcular(string a, string op, string b, string &resultado)
{
    bool ambosEnteros = a.find('.') == string::npos && b.find('.') == string::npos;
    ostringstream salida;

    if (ambosEnteros)
    {
        long long x, y;
        istringstream(a) >> x;
        istringstream(b) >> y;

        if (op == "/" && y == 0)
        {
            return false;
        }

        long long r = 0;
        if (op == "+") r = x + y;
        if (op == "-") r = x - y;
        if (op == "*") r = x * y;
        if (op == "/") r = x / y; // división entera, igual que en C

        // Un resultado negativo no es un literal válido del lenguaje (no hay menos unario)
        if (r < 0)
        {
            return false;
        }

        salida << r;
    }
    else
    {
        double x, y;
        istringstream(a) >> x;
        istringstream(b) >> y;

        if (op == "/" && y == 0)
        {
            return false;
        }

        double r = 0;
        if (op == "+") r = x + y;
        if (op == "-") r = x - y;
        if (op == "*") r = x * y;
        if (op == "/") r = x / y;

        if (r < 0)
        {
            return false;
        }

        salida << setprecision(10) << r;

        // El resultado sigue siendo float: se conserva el punto decimal (5 -> 5.0)
        string texto = salida.str();
        if (texto.find('.') == string::npos && texto.find('e') == string::npos)
        {
            salida << ".0";
        }
    }

    resultado = salida.str();
    return true;
}

// x + 0, 0 + x, x - 0, x * 1, 1 * x, x / 1  ->  x
// Solo con los enteros 0 y 1 para no cambiar el tipo del resultado (x * 1.0 sería float)
bool Optimizador::simplificar(Cuadrupla &c)
{
    string queda = "";

    if (c.op == "+" && c.arg2 == "0") queda = c.arg1;
    else if (c.op == "+" && c.arg1 == "0") queda = c.arg2;
    else if (c.op == "-" && c.arg2 == "0") queda = c.arg1;
    else if (c.op == "*" && c.arg2 == "1") queda = c.arg1;
    else if (c.op == "*" && c.arg1 == "1") queda = c.arg2;
    else if (c.op == "/" && c.arg2 == "1") queda = c.arg1;

    if (queda.empty())
    {
        return false;
    }

    c.arg1 = queda;
    c.op = "";
    c.arg2 = "";
    return true;
}

vector<Cuadrupla> Optimizador::optimizar(vector<Cuadrupla> codigo)
{
    cambios.clear();

    // ---------- Pasada 1: plegado, simplificación y propagación de temporales ----------
    map<string, string> valorTemporal; // temporal -> valor que lo reemplaza
    vector<Cuadrupla> paso1;

    for (Cuadrupla c : codigo)
    {
        // Propagación: se reemplazan temporales ya conocidos
        if (valorTemporal.count(c.arg1))
        {
            c.arg1 = valorTemporal[c.arg1];
        }
        if (valorTemporal.count(c.arg2))
        {
            c.arg2 = valorTemporal[c.arg2];
        }

        if (!c.op.empty())
        {
            string antes = c.texto();
            string valor;

            if (esNumero(c.arg1) && esNumero(c.arg2) && calcular(c.arg1, c.op, c.arg2, valor))
            {
                c.arg1 = valor;
                c.op = "";
                c.arg2 = "";
                cambios.push_back("Plegado de constantes:     " + antes + "   ->   " + c.texto());
            }
            else if (simplificar(c))
            {
                cambios.push_back("Simplificacion algebraica: " + antes + "   ->   " + c.texto());
            }
        }

        // Un temporal que solo guarda un valor se propaga y se elimina
        if (c.op.empty() && c.resultadoEsTemporal)
        {
            valorTemporal[c.resultado] = c.arg1;
            cambios.push_back("Propagacion de temporal:   " + c.resultado +
                              " se reemplaza por " + c.arg1 + " (se elimina su instruccion)");
            continue;
        }

        paso1.push_back(c);
    }

    // ---------- Pasada 2: eliminación de la copia final ----------
    //   tN = a op b
    //   x  = tN        ->   x = a op b
    vector<Cuadrupla> optimizado;

    for (int i = 0; i < (int)paso1.size(); i++)
    {
        Cuadrupla c = paso1[i];

        if (c.resultadoEsTemporal && i + 1 < (int)paso1.size())
        {
            Cuadrupla siguiente = paso1[i + 1];

            if (siguiente.op.empty() && siguiente.arg1 == c.resultado)
            {
                // El temporal no debe usarse en ninguna otra instrucción
                int usos = 0;
                for (Cuadrupla otra : paso1)
                {
                    if (otra.arg1 == c.resultado) usos++;
                    if (otra.arg2 == c.resultado) usos++;
                }

                if (usos == 1)
                {
                    Cuadrupla unida = c;
                    unida.resultado = siguiente.resultado;
                    unida.resultadoEsTemporal = false;

                    cambios.push_back("Eliminacion de copia:      " + c.texto() + " ; " +
                                      siguiente.texto() + "   ->   " + unida.texto());

                    optimizado.push_back(unida);
                    i++; // la copia ya fue absorbida
                    continue;
                }
            }
        }

        optimizado.push_back(c);
    }

    return optimizado;
}

vector<string> Optimizador::obtenerCambios()
{
    return cambios;
}
