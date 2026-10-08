#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "Semantico.h"
#include "CodigoIntermedio.h"
#include "Optimizador.h"

using namespace std;

/*
 * Flujo del compilador:
 *   Codigo fuente -> Lexer -> Parser -> Analisis semantico -> Codigo intermedio -> Optimizacion
 *
 * Uso:
 *   compilador archivo.txt     (lee el codigo desde un archivo)
 *   compilador archivo.txt --pausa  (igual, pero espera Enter antes de cerrar)
 *   compilador                 (lee el codigo por teclado; terminar con una linea FIN)
 */

string nombreToken(TipoToken tipo)
{
    switch (tipo)
    {
    case TipoToken::LITERAL_ENTERO:
        return "LITERAL_ENTERO";

    case TipoToken::LITERAL_REAL:
        return "LITERAL_REAL";

    case TipoToken::LITERAL_STRING:
        return "LITERAL_STRING";

    case TipoToken::IDENTIFICADOR:
        return "IDENTIFICADOR";

    case TipoToken::PALABRA_CLAVE:
        return "PALABRA_CLAVE";

    case TipoToken::OPERADOR:
        return "OPERADOR";

    case TipoToken::ASIGNACION:
        return "ASIGNACION";

    case TipoToken::DELIMITADOR:
        return "DELIMITADOR";

    case TipoToken::PAREN_IZQ:
        return "PAREN_IZQ";

    case TipoToken::PAREN_DER:
        return "PAREN_DER";

    case TipoToken::DESCONOCIDO:
        return "DESCONOCIDO";
    }
    return "";
}

void titulo(string texto)
{
    cout << "\n===============================================" << endl;
    cout << "  " << texto << endl;
    cout << "===============================================" << endl;
}

string leerCodigo(int argc, char *argv[])
{
    ostringstream codigo;

    if (argc > 1)
    {
        ifstream archivo(argv[1]);
        if (!archivo)
        {
            cout << "No se pudo abrir el archivo: " << argv[1] << endl;
            return "";
        }
        codigo << archivo.rdbuf();
        return codigo.str();
    }

    cout << "Ingrese el codigo (varias lineas). Escriba FIN en una linea para terminar:" << endl;
    string linea;
    while (getline(cin, linea))
    {
        if (linea == "FIN")
        {
            break;
        }
        codigo << linea << "\n";
    }
    return codigo.str();
}

int main(int argc, char *argv[])
{
    string codigo = leerCodigo(argc, argv);

    // ---------------- Codigo fuente ----------------
    titulo("CODIGO FUENTE");
    {
        istringstream lineas(codigo);
        string linea;
        int numero = 1;
        while (getline(lineas, linea))
        {
            cout << setw(3) << numero++ << " | " << linea << endl;
        }
    }

    // ---------------- 1. Analisis lexico ----------------
    titulo("1. ANALISIS LEXICO");

    Lexer lexer(codigo);
    vector<Token> tokens = lexer.analizar();

    cout << "--------TOKENS ENCONTRADOS---------\n";
    for (Token token : tokens)
    {
        cout
            << "<"
            << nombreToken(token.tipo)
            << ", "
            << token.lexema
            << ">  (linea "
            << token.linea
            << ", col "
            << token.columna
            << ")"
            << endl;
    }

    vector<ErrorLexico> errores = lexer.obtenerErrores();
    cout << "\n-------- ERRORES LEXICOS --------\n";

    if (errores.empty())
    {
        cout << "No se encontraron errores lexicos." << endl;
    }
    else
    {
        for (ErrorLexico error : errores)
        {
            cout << "Lexema: " << error.lexema << endl;
            cout << "Linea: " << error.linea << endl;
            cout << "Columna: " << error.columna << endl;
            cout << "Mensaje: " << error.mensaje << endl;
            cout << "----------------------" << endl;
        }
        cout << "Total: " << errores.size()
             << " error(es) lexico(s). El analisis continuo hasta el final del codigo." << endl;
    }

    // ---------------- 2. Analisis sintactico ----------------
    titulo("2. ANALISIS SINTACTICO");

    Parser parser(tokens);
    parser.analizar();

    vector<string> erroresSintacticos = parser.obtenerErrores();
    vector<Instruccion> instrucciones = parser.obtenerInstrucciones();

    if (erroresSintacticos.empty())
    {
        cout << "No se encontraron errores sintacticos." << endl;
    }
    else
    {
        for (string error : erroresSintacticos)
        {
            cout << error << endl;
        }
    }
    cout << "\nInstrucciones sintacticamente correctas: " << instrucciones.size() << endl;

    // ---------------- 3. Analisis semantico ----------------
    titulo("3. ANALISIS SEMANTICO");

    Semantico semantico;
    semantico.analizar(instrucciones);

    vector<string> erroresSemanticos = semantico.obtenerErrores();

    if (erroresSemanticos.empty())
    {
        cout << "No se encontraron errores semanticos." << endl;
    }
    else
    {
        for (string error : erroresSemanticos)
        {
            cout << error << endl;
        }
    }

    cout << "\n-------- TABLA DE SIMBOLOS --------\n";
    vector<Simbolo> tabla = semantico.obtenerTablaSimbolos();

    if (tabla.empty())
    {
        cout << "(vacia)" << endl;
    }
    else
    {
        cout << left << setw(15) << "Nombre" << setw(10) << "Tipo" << "Declarada en" << endl;
        cout << "-----------------------------------------" << endl;
        for (Simbolo s : tabla)
        {
            cout << left << setw(15) << s.nombre << setw(10) << s.tipo
                 << "linea " << s.linea << ", col " << s.columna << endl;
        }
        cout << right;
    }

    // ---------------- 4. Codigo intermedio ----------------
    titulo("4. CODIGO INTERMEDIO (TRES DIRECCIONES)");

    vector<Instruccion> validas = semantico.obtenerInstruccionesValidas();

    GeneradorCodigo generador;
    generador.generar(validas);

    vector<BloqueCodigo> bloques = generador.obtenerBloques();

    if (bloques.empty())
    {
        cout << "No hay instrucciones validas para generar codigo." << endl;
    }

    for (BloqueCodigo bloque : bloques)
    {
        cout << "[linea " << bloque.destino.linea << "] "
             << bloque.destino.lexema << " = ..." << endl;
        cout << "   Postfija        : " << bloque.postfija << endl;
        cout << "   Tres direcciones:" << endl;
        for (Cuadrupla c : bloque.codigo)
        {
            cout << "      " << c.texto() << endl;
        }
        cout << endl;
    }

    // ---------------- 5. Optimizacion ----------------
    titulo("5. OPTIMIZACION");

    vector<Cuadrupla> antes = generador.obtenerCodigo();

    Optimizador optimizador;
    vector<Cuadrupla> despues = optimizador.optimizar(antes);

    cout << "Antes (" << antes.size() << " instrucciones):" << endl;
    for (Cuadrupla c : antes)
    {
        cout << "   " << c.texto() << endl;
    }

    cout << "\nDespues (" << despues.size() << " instrucciones):" << endl;
    for (Cuadrupla c : despues)
    {
        cout << "   " << c.texto() << endl;
    }

    cout << "\nMejoras aplicadas:" << endl;
    vector<string> cambios = optimizador.obtenerCambios();
    if (cambios.empty())
    {
        cout << "   (ninguna, el codigo ya era optimo)" << endl;
    }
    for (string cambio : cambios)
    {
        cout << "   - " << cambio << endl;
    }

    // ---------------- Resumen ----------------
    titulo("RESUMEN");
    cout << "Errores lexicos     : " << errores.size() << endl;
    cout << "Errores sintacticos : " << erroresSintacticos.size() << endl;
    cout << "Errores semanticos  : " << erroresSemanticos.size() << endl;

    int totalErrores = errores.size() + erroresSintacticos.size() + erroresSemanticos.size();
    if (totalErrores == 0)
    {
        cout << "\nResultado: Programa compilado correctamente." << endl;
    }
    else
    {
        cout << "\nResultado: Se encontraron " << totalErrores
             << " error(es). Se genero codigo solo para las instrucciones validas." << endl;
    }

    // Evita que la consola se cierre al terminar (modo teclado o con --pausa)
    bool pausar = argc <= 1 || (argc > 2 && string(argv[2]) == "--pausa");
    if (pausar)
    {
        cout << "\nPresione Enter para salir...";
        cin.clear();
        string espera;
        getline(cin, espera);
    }

    return 0;
}
