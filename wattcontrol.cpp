// =============================================================
// WattControl EA3 - Gestor de Energia para Negocios
// Negocio: Minimercado El Rosario (Digito 8)
// Integrante: Stefanya Brighte Fontalvo Padilla
// =============================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Constantes.h"
#include "Carga.h"
#include "Medidor.h"
#include "Mercado.h"
#include "Negocio.h"

using namespace std;

// --- Validacion de texto (string): no vacio, no solo espacios, minimo 3 caracteres ---
string leerTextoValido(const string& mensaje) {
    string entrada;
    bool valido = false;

    do {
        cout << mensaje;
        getline(cin, entrada);

        // Quitar espacios al inicio y al final
        size_t inicio = entrada.find_first_not_of(" \t\r\n");
        if (inicio == string::npos) {
            cout << "  [Error]: El texto no puede estar vacio ni contener solo espacios." << endl;
            continue;
        }
        size_t fin = entrada.find_last_not_of(" \t\r\n");
        entrada = entrada.substr(inicio, fin - inicio + 1);

        if ((int)entrada.length() < LONGITUD_MIN_TEXTO) {
            cout << "  [Error]: Debe ingresar al menos " << LONGITUD_MIN_TEXTO << " caracteres." << endl;
        } else {
            valido = true;
        }
    } while (!valido);

    return entrada;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    srand(time(0));

    cout << "==========================================================" << endl;
    cout << "       WATTCONTROL EA3 - REGISTRO DE OPERACION            " << endl;
    cout << "==========================================================" << endl;

    string nombreNegocio = leerTextoValido("Ingrese el nombre del negocio (ej. Minimercado El Rosario): ");
    string responsable   = leerTextoValido("Ingrese el nombre del responsable: ");

    Mercado mercado;
    CargaCritica refr("Vitrina Refrigerada", POTENCIA_VITRINA);
    CargaAplazable congelacion("Sistemas de Congelacion", POTENCIA_CONGELACION);
    CargaAplazable clima("Iluminacion/Climatizacion", POTENCIA_CLIMA);

    Negocio negocio(nombreNegocio, responsable, &mercado);
    negocio.agregarCarga(&refr);
    negocio.agregarCarga(&congelacion);
    negocio.agregarCarga(&clima);

    int opcion;
    int hora = 12;
    float precio = 500.0, embalses = 50.0;

    do {
        cout << "\n==========================================================" << endl;
        cout << "          WATTCONTROL - GESTOR DE ENERGIA                 " << endl;
        cout << "   Negocio: " << nombreNegocio << " | Resp: " << responsable << endl;
        cout << "==========================================================" << endl;
        cout << "1. Ingresar lectura manual (Hora, Precio, Embalses)" << endl;
        cout << "2. Evaluar decision puntual de cada carga" << endl;
        cout << "3. Simular un dia aleatorio (24 Horas - EA2)" << endl;
        cout << "4. Ver resumen diario e informe (EA2)" << endl;
        cout << "5. Simular semana real XM (21 al 27 sept 2026 - EA3)" << endl;
        cout << "6. Ver reporte semanal, ahorro y proyeccion mensual (EA3)" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(10000, '\n');
            opcion = -1;
        }

        switch (opcion) {
        case 1:
            do {
                cout << "Hora actual (" << HORA_MIN << " a " << HORA_MAX << "): ";
                cin >> hora;
                if (hora < HORA_MIN || hora > HORA_MAX) {
                    cout << "  [Error]: Hora fuera de rango (" << HORA_MIN << "-" << HORA_MAX << ")." << endl;
                }
            } while (hora < HORA_MIN || hora > HORA_MAX);

            do {
                cout << "Precio de bolsa (COP/kWh > 0): ";
                cin >> precio;
                if (precio <= 0) {
                    cout << "  [Error]: El precio debe ser mayor a 0." << endl;
                }
            } while (precio <= 0);

            do {
                cout << "Nivel de embalses (" << EMBALSE_MIN << " a " << EMBALSE_MAX << " %): ";
                cin >> embalses;
                if (embalses < EMBALSE_MIN || embalses > EMBALSE_MAX) {
                    cout << "  [Error]: Embalses fuera de rango (" << EMBALSE_MIN << "-" << EMBALSE_MAX << ")." << endl;
                }
            } while (embalses < EMBALSE_MIN || embalses > EMBALSE_MAX);
            break;

        case 2:
            negocio.evaluar(hora, precio, embalses);
            break;

        case 3:
            negocio.simularDia();
            break;

        case 4:
            negocio.mostrarResumenDiario();
            break;

        case 5:
            negocio.simularSemana();
            break;

        case 6:
            negocio.mostrarReporteSemanal();
            break;

        case 0:
            cout << "\nHasta luego! Gracias por usar WattControl." << endl;
            break;

        default:
            cout << "\n[!] Opcion invalida. Intente nuevamente." << endl;
            break;
        }
    } while (opcion != 0);

    return 0;
}