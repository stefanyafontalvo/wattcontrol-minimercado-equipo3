// =============================================================
// WattControl EA2 - Gestor de Energia para Negocios (El Nino 2026)
// =============================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

#define HORAS_DIA 24

// --- Constantes del Negocio (Panaderia El Rosario) ---
const float POTENCIA_REFRIGERACION = 3.0; // Carga Critica (kW)
const float POTENCIA_HORNO         = 8.0; // Carga Aplazable 1 (kW)
const float POTENCIA_AMASADORA     = 4.5; // Carga Aplazable 2 (kW)

const float POTENCIA_CONTRATADA    = 15.0; // Maxima potencia permitida (kW)
const float PRECIO_CARO            = 600.0; // COP/kWh (Limite hora pico)
const float PRECIO_EMERGENCIA      = 1500.0;// COP/kWh
const float EMBALSES_ALERTA        = 40.0;  // Nivel critico embalses (%)

const int PICO_INICIO              = 18;    // 6 PM
const int PICO_FIN                 = 20;    // 8 PM

const int MIN_HORAS_HORNO          = 6;     // Horas minimas requeridas
const int MIN_HORAS_AMASADORA      = 4;

const int FALLA_CADA               = 20;

// --- CLASE BASE ABSTRACTA: Carga ---
class Carga {
protected:
    string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;

public:
    Carga(string n, float p) : nombre(n), potenciaKW(p), encendida(false), horasUso(0) {}
    virtual ~Carga() {}

    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    void encender() { encendida = true; horasUso++; }
    void apagar() { encendida = false; }
    void reiniciarHoras() { horasUso = 0; encendida = false; }

    float costoHora(float precio) { return encendida ? (potenciaKW * precio) : 0.0; }

    string getNombre() { return nombre; }
    float getPotencia() { return potenciaKW; }
    bool estaEncendida() { return encendida; }
    int getHorasUso() { return horasUso; }
};

// --- HERENCIA 1: Carga Critica ---
class CargaCritica : public Carga {
public:
    CargaCritica(string n, float p) : Carga(n, p) {}
    bool debeEncender(int hora, float precio, float embalses) override {
        return true; // Regla 1: Nunca se apaga
    }
};

// --- HERENCIA 2: Carga Aplazable ---
class CargaAplazable : public Carga {
public:
    CargaAplazable(string n, float p) : Carga(n, p) {}
    bool debeEncender(int hora, float precio, float embalses) override {
        bool horaPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool aplazar = (embalses < EMBALSES_ALERTA) || horaPico || (precio > PRECIO_CARO);
        return !aplazar;
    }
};

// --- COMPOSICION: Medidor ---
class Medidor {
private:
    float kWhAcumulados;
    float costoTotal;

public:
    Medidor() : kWhAcumulados(0), costoTotal(0) {}
    void reiniciar() { kWhAcumulados = 0; costoTotal = 0; }
    void acumular(float kWh, float costo) { kWhAcumulados += kWh; costoTotal += costo; }
    float getKWh() { return kWhAcumulados; }
    float getCosto() { return costoTotal; }
};

// --- ASOCIACION: Mercado ---
class Mercado {
public:
    float precioHora(int hora) {
        int minimo, maximo;
        if (hora <= 6)       { minimo = 398; maximo = 640;  }
        else if (hora <= 14) { minimo = 456; maximo = 970;  }
        else if (hora <= 17) { minimo = 596; maximo = 1040; }
        else if (hora <= 20) { minimo = 705; maximo = 1612; }
        else                 { minimo = 565; maximo = 970;  }
        return minimo + rand() % (maximo - minimo + 1);
    }

    float nivelEmbalses() {
        return 30.0 + (rand() % 31);
    }
};

// --- CLASE PRINCIPAL: Negocio ---
class Negocio {
private:
    string nombre;
    Medidor medidor;
    Carga* cargas[3];
    int numCargas;
    Mercado* mercado;
    float costoCon, costoSin;
    int horasSimuladas, fallas;

public:
    Negocio(string n, Mercado* m) : nombre(n), mercado(m), numCargas(0), costoCon(0), costoSin(0), horasSimuladas(0), fallas(0) {}

    void agregarCarga(Carga* c) {
        if (numCargas < 3) cargas[numCargas++] = c;
    }

    void evaluar(int hora, float precio, float embalses) {
        cout << "\n--- Evaluacion individual en la hora " << hora << " ---" << endl;
        for (int i = 0; i < numCargas; i++) {
            bool dec = cargas[i]->debeEncender(hora, precio, embalses);
            cout << "  " << setw(22) << left << cargas[i]->getNombre() << right 
                 << (dec ? " [ENCENDER]" : " [APLAZAR/APAGAR]") << endl;
        }
    }

    void simularDia() {
        medidor.reiniciar();
        costoCon = 0; costoSin = 0; horasSimuladas = 0; fallas = 0;
        for (int i = 0; i < numCargas; i++) cargas[i]->reiniciarHoras();

        cout << fixed << setprecision(0);
        cout << "\n==================== TABLA DE SIMULACION (24 HORAS) ====================" << endl;
        cout << setw(5) << "Hora" << setw(9) << "Precio" << setw(9) << "Embalses";
        for (int i = 0; i < numCargas; i++) cout << setw(15) << cargas[i]->getNombre();
        cout << setw(12) << "Costo Hora" << endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            float precio = mercado->precioHora(h);
            float embalses = mercado->nivelEmbalses();

            float consumoSinHora = 0;
            for (int i = 0; i < numCargas; i++) consumoSinHora += cargas[i]->getPotencia();
            costoSin += consumoSinHora * precio;

            // BREAK: Racionamiento por precio de escasez y embalses bajos
            if (precio > PRECIO_EMERGENCIA && embalses < EMBALSES_ALERTA) {
                cout << setw(5) << h << setw(9) << precio << setw(9) << embalses 
                     << "  --> EMERGENCY BREAK: Racionamiento decretado." << endl;
                break;
            }

            horasSimuladas++;

            // CONTINUE: Falla de lectura de mercado
            if (rand() % FALLA_CADA == 0) {
                for (int i = 0; i < numCargas; i++) {
                    if (i == 0 && cargas[i]->debeEncender(h, precio, embalses)) cargas[i]->encender();
                    else cargas[i]->apagar();
                }
                fallas++;
                cout << setw(5) << h << setw(9) << "--" << setw(9) << embalses 
                     << "  --> CONTINUE: Falla de lectura (Solo Critica activa)" << endl;
                continue;
            }

            // GESTOR Y REGLA 6 (Potencia contratada)
            float potenciaEncendida = 0;
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->debeEncender(h, precio, embalses)) {
                    cargas[i]->encender();
                    potenciaEncendida += cargas[i]->getPotencia();
                } else {
                    cargas[i]->apagar();
                }
            }

            if (potenciaEncendida > POTENCIA_CONTRATADA) {
                cout << "  [ALERTA HORA " << h << "]: Exceso potencia (" << potenciaEncendida << " kW). Se apaga Horno." << endl;
                cargas[1]->apagar();
            }

            float costoHoraGestor = 0, kWhHoraGestor = 0;
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->estaEncendida()) {
                    costoHoraGestor += cargas[i]->costoHora(precio);
                    kWhHoraGestor += cargas[i]->getPotencia();
                }
            }
            medidor.acumular(kWhHoraGestor, costoHoraGestor);
            costoCon += costoHoraGestor;

            cout << setw(5) << h << setw(9) << precio << setw(9) << embalses << "%";
            for (int i = 0; i < numCargas; i++) cout << setw(15) << (cargas[i]->estaEncendida() ? "ON" : "off");
            cout << setw(12) << costoHoraGestor << endl;
        }
        cout << "\nSimulacion concluida." << endl;
    }

    void mostrarResumen() {
        if (horasSimuladas == 0) {
            cout << "\n[!] Debe ejecutar primero la simulacion (Opcion 3)." << endl;
            return;
        }

        cout << "\n================ RESUMEN DE OPERACION Y AHORRO ================" << endl;
        cout << "Negocio: " << nombre << endl;
        cout << "Horas simuladas: " << horasSimuladas << " / 24 | Fallas: " << fallas << endl << endl;

        for (int i = 0; i < numCargas; i++) {
            float kWh = cargas[i]->getHorasUso() * cargas[i]->getPotencia();
            cout << "  * " << setw(22) << left << cargas[i]->getNombre() << right 
                 << cargas[i]->getHorasUso() << "h uso | " << setw(6) << kWh << " kWh" << endl;
        }

        cout << "\nCosto CON Gestor:  $" << costoCon << " COP" << endl;
        cout << "Costo SIN Gestor:  $" << costoSin << " COP" << endl;

        float ahorroPesos = costoSin - costoCon;
        float ahorroPorcentaje = (costoSin > 0) ? (ahorroPesos / costoSin) * 100.0 : 0.0;

        cout << "AHORRO NETO:       $" << ahorroPesos << " COP (" << fixed << setprecision(1) << ahorroPorcentaje << " %)" << endl;

        if (cargas[1]->getHorasUso() < MIN_HORAS_HORNO || cargas[2]->getHorasUso() < MIN_HORAS_AMASADORA) {
            cout << "\n[ALERTA]: PRODUCCION INCOMPLETA. No se alcanzaron las horas minimas requeridas." << endl;
        }
    }
};

// --- MENU PRINCIPAL ---
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    srand(time(0));

    Mercado mercado;
    CargaCritica refr("Refrigeracion Base", POTENCIA_REFRIGERACION);
    CargaAplazable horno("Horno Electrico", POTENCIA_HORNO);
    CargaAplazable amasadora("Amasadora Ind.", POTENCIA_AMASADORA);

    Negocio negocio("Panaderia El Rosario", &mercado);
    negocio.agregarCarga(&refr);
    negocio.agregarCarga(&horno);
    negocio.agregarCarga(&amasadora);

    int opcion;
    int hora = 12;
    float precio = 500.0, embalses = 50.0;

    do {
        cout << "\n==============================================" << endl;
        cout << "      WATTCONTROL - GESTOR DE ENERGIA        " << endl;
        cout << "==============================================" << endl;
        cout << "1. Ingresar lectura manual (Hora, Precio, Embalses)" << endl;
        cout << "2. Evaluar decision puntual de cada carga" << endl;
        cout << "3. Simular un dia completo de operacion (24 Horas)" << endl;
        cout << "4. Ver resumen de ahorro e informe" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
        case 1:
            do {
                cout << "Hora actual (0 a 23): "; cin >> hora;
                if (hora < 0 || hora > 23) cout << " Error: Hora (0-23)." << endl;
            } while (hora < 0 || hora > 23);

            do {
                cout << "Precio de bolsa (COP/kWh > 0): "; cin >> precio;
                if (precio <= 0) cout << " Error: Precio > 0." << endl;
            } while (precio <= 0);

            do {
                cout << "Nivel de embalses (0 a 100 %): "; cin >> embalses;
                if (embalses < 0 || embalses > 100) cout << " Error: Embalses (0-100)." << endl;
            } while (embalses < 0 || embalses > 100);
            break;

        case 2:
            negocio.evaluar(hora, precio, embalses);
            break;

        case 3:
            negocio.simularDia();
            break;

        case 4:
            negocio.mostrarResumen();
            break;

        case 0:
            cout << "\nHasta luego!" << endl;
            break;

        default:
            cout << "\n[!] Opcion invalida." << endl;
            break;
        }
    } while (opcion != 0);

    return 0;
}