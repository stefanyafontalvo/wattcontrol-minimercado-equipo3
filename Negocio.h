#ifndef NEGOCIO_H
#define NEGOCIO_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include "Constantes.h"
#include "Carga.h"
#include "Medidor.h"
#include "Mercado.h"

using namespace std;

class Negocio {
private:
    string nombre;
    string responsable;
    Medidor medidor;                                // Composicion
    Carga* cargas[MAX_CARGAS];                      // Agregacion
    int numCargas;
    Mercado* mercado;                               // Asociacion

    // Variables simulacion diaria (EA2)
    float costoCon, costoSin;
    int horasSimuladas, fallas;

    // Variables y matriz simulacion semanal (EA3)
    float costos[DIAS_SEMANA][HORAS_DIA];
    float costosSinGestor[DIAS_SEMANA][HORAS_DIA];
    int diasCumpleCongelacion;
    int diasCumpleClima;
    bool semanaSimulada;

public:
    Negocio(string n, string resp, Mercado* m)
        : nombre(n), responsable(resp), mercado(m), numCargas(0),
          costoCon(0), costoSin(0), horasSimuladas(0), fallas(0),
          diasCumpleCongelacion(0), diasCumpleClima(0), semanaSimulada(false) {
        for (int d = 0; d < DIAS_SEMANA; d++) {
            for (int h = 0; h < HORAS_DIA; h++) {
                costos[d][h] = 0.0;
                costosSinGestor[d][h] = 0.0;
            }
        }
    }

    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas++] = c;
        }
    }

    // --- FUNCION SOBRECARGADA 1: Costo de una hora ---
    float costo(float kW, float precio) {
        return kW * precio;
    }

    // --- FUNCION SOBRECARGADA 2: Suma el costo de un arreglo de horas ---
    float costo(float costosDia[], int n) {
        float total = 0.0;
        for (int i = 0; i < n; i++) {
            total += costosDia[i];
        }
        return total;
    }

    // --- FUNCION PROPIA 1: Costo total de la semana ---
    float costoSemana(float matriz[DIAS_SEMANA][HORAS_DIA]) {
        float total = 0.0;
        for (int d = 0; d < DIAS_SEMANA; d++) {
            total += costo(matriz[d], HORAS_DIA); // Usa la sobrecarga
        }
        return total;
    }

    // --- FUNCION PROPIA 2: Promedio diario de gasto ---
    float promedio(float matriz[DIAS_SEMANA][HORAS_DIA]) {
        return costoSemana(matriz) / DIAS_SEMANA;
    }

    // --- FUNCION PROPIA 3: Indice del dia mas caro de la semana ---
    int diaMasCaro(float matriz[DIAS_SEMANA][HORAS_DIA]) {
        int indiceMayor = 0;
        float mayorCosto = costo(matriz[0], HORAS_DIA);
        for (int d = 1; d < DIAS_SEMANA; d++) {
            float costoActual = costo(matriz[d], HORAS_DIA);
            if (costoActual > mayorCosto) {
                mayorCosto = costoActual;
                indiceMayor = d;
            }
        }
        return indiceMayor;
    }

    // --- FUNCION PROPIA 4: Hora mas cara de toda la semana ---
    int horaMasCara(int& diaEncontrado, float& precioMaximo) {
        diaEncontrado = 0;
        int horaMayor = 0;
        precioMaximo = mercado->precio(0, 0);

        for (int d = 0; d < DIAS_SEMANA; d++) {
            for (int h = 0; h < HORAS_DIA; h++) {
                float p = mercado->precio(d, h);
                if (p > precioMaximo) {
                    precioMaximo = p;
                    diaEncontrado = d;
                    horaMayor = h;
                }
            }
        }
        return horaMayor;
    }

    // Evaluacion puntual (EA2)
    void evaluar(int hora, float precio, float embalses) {
        cout << "\n--- Evaluacion individual en la hora " << hora << " ---" << endl;
        for (int i = 0; i < numCargas; i++) {
            bool dec = cargas[i]->debeEncender(hora, precio, embalses);
            cout << "  " << setw(28) << left << cargas[i]->getNombre() << right
                 << (dec ? " [ENCENDER]" : " [APLAZAR/APAGAR]") << endl;
        }
    }

    // Simulacion de 1 dia aleatorio (EA2)
    void simularDia() {
        medidor.reiniciar();
        costoCon = 0; costoSin = 0; horasSimuladas = 0; fallas = 0;
        for (int i = 0; i < numCargas; i++) cargas[i]->reiniciarHoras();

        cout << fixed << setprecision(0);
        cout << "\n==================== TABLA DE SIMULACION (24 HORAS) ====================" << endl;
        cout << setw(5) << "Hora" << setw(9) << "Precio" << setw(9) << "Embalses";
        for (int i = 0; i < numCargas; i++) cout << setw(26) << cargas[i]->getNombre();
        cout << setw(12) << "Costo Hora" << endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            float precio = mercado->precioHoraAleatorio(h);
            float embalses = mercado->nivelEmbalsesAleatorio();

            float consumoSinHora = 0;
            for (int i = 0; i < numCargas; i++) consumoSinHora += cargas[i]->getPotencia();
            costoSin += costo(consumoSinHora, precio);

            if (precio > PRECIO_EMERGENCIA && embalses < EMBALSES_ALERTA) {
                cout << setw(5) << h << setw(9) << precio << setw(9) << embalses
                     << "  --> EMERGENCY BREAK: Racionamiento decretado." << endl;
                break;
            }

            horasSimuladas++;

            if (rand() % FALLA_CADA == 0) {
                fallas++;
                float costoFalla = 0, kWhFalla = 0;
                for (int i = 0; i < numCargas; i++) {
                    if (dynamic_cast<CargaCritica*>(cargas[i])) {
                        cargas[i]->encender();
                        costoFalla += cargas[i]->costoHora(precio);
                        kWhFalla += cargas[i]->getPotencia();
                    } else {
                        cargas[i]->apagar();
                    }
                }
                medidor.acumular(kWhFalla, costoFalla);
                costoCon += costoFalla;
                cout << setw(5) << h << setw(9) << "--" << setw(9) << embalses << "%"
                     << "  --> CONTINUE: Falla de lectura (Solo Critica activa)" << endl;
                continue;
            }

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
                CargaAplazable* mayorCarga = nullptr;
                for (int i = 0; i < numCargas; i++) {
                    CargaAplazable* ca = dynamic_cast<CargaAplazable*>(cargas[i]);
                    if (ca && ca->estaEncendida()) {
                        if (!mayorCarga || ca->getPotencia() > mayorCarga->getPotencia()) {
                            mayorCarga = ca;
                        }
                    }
                }
                if (mayorCarga) {
                    cout << "  [ALERTA HORA " << h << "]: Exceso potencia (" << potenciaEncendida
                         << " kW > " << POTENCIA_CONTRATADA << " kW). Se apaga " << mayorCarga->getNombre() << "." << endl;
                    mayorCarga->apagar();
                }
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
            for (int i = 0; i < numCargas; i++) cout << setw(26) << (cargas[i]->estaEncendida() ? "ON" : "off");
            cout << setw(12) << costoHoraGestor << endl;
        }
        cout << "\nSimulacion diaria concluida." << endl;
    }

    void mostrarResumenDiario() {
        if (horasSimuladas == 0) {
            cout << "\n[!] Debe ejecutar primero la simulacion diaria (Opcion 3)." << endl;
            return;
        }
        cout << "\n================ RESUMEN DIARIO DE OPERACION ================" << endl;
        cout << "Negocio: " << nombre << " | Responsable: " << responsable << endl;
        cout << "Horas simuladas: " << horasSimuladas << " / " << HORAS_DIA << " | Fallas: " << fallas << endl << endl;

        for (int i = 0; i < numCargas; i++) {
            float kWh = cargas[i]->getHorasUso() * cargas[i]->getPotencia();
            cout << "  * " << setw(28) << left << cargas[i]->getNombre() << right
                 << cargas[i]->getHorasUso() << "h uso | " << setw(6) << kWh << " kWh" << endl;
        }

        cout << "\nCosto CON Gestor:  $" << fixed << setprecision(0) << costoCon << " COP" << endl;
        cout << "Costo SIN Gestor:  $" << costoSin << " COP" << endl;
        float ahorroPesos = costoSin - costoCon;
        float ahorroPorcentaje = (costoSin > 0) ? (ahorroPesos / costoSin) * 100.0 : 0.0;
        cout << "AHORRO NETO:       $" << ahorroPesos << " COP (" << setprecision(1) << ahorroPorcentaje << " %)" << endl;
    }

    // --- NUEVO EA3: Simulacion de la semana real (21 al 27 sept 2026) ---
    void simularSemana() {
        medidor.reiniciar();
        diasCumpleCongelacion = 0;
        diasCumpleClima = 0;

        for (int d = 0; d < DIAS_SEMANA; d++) {
            for (int i = 0; i < numCargas; i++) {
                cargas[i]->reiniciarHoras();
            }

            for (int h = 0; h < HORAS_DIA; h++) {
                float precioActual = mercado->precio(d, h);
                float embalsesActual = EMBALSES_SEMANA;

                // Costo SIN gestor (todas las cargas encendidas)
                float kWSin = 0.0;
                for (int i = 0; i < numCargas; i++) {
                    kWSin += cargas[i]->getPotencia();
                }
                costosSinGestor[d][h] = costo(kWSin, precioActual);

                // Evaluacion polimorfica CON gestor
                float potenciaEncendida = 0.0;
                for (int i = 0; i < numCargas; i++) {
                    if (cargas[i]->debeEncender(h, precioActual, embalsesActual)) {
                        cargas[i]->encender();
                        potenciaEncendida += cargas[i]->getPotencia();
                    } else {
                        cargas[i]->apagar();
                    }
                }

                // Regla 6: Control de potencia contratada (15.0 kW)
                if (potenciaEncendida > POTENCIA_CONTRATADA) {
                    CargaAplazable* mayorCarga = nullptr;
                    for (int i = 0; i < numCargas; i++) {
                        CargaAplazable* ca = dynamic_cast<CargaAplazable*>(cargas[i]);
                        if (ca && ca->estaEncendida()) {
                            if (!mayorCarga || ca->getPotencia() > mayorCarga->getPotencia()) {
                                mayorCarga = ca;
                            }
                        }
                    }
                    if (mayorCarga) {
                        mayorCarga->apagar();
                    }
                }

                // Calcular y guardar costo en matriz costos[7][24]
                float kWCon = 0.0;
                for (int i = 0; i < numCargas; i++) {
                    if (cargas[i]->estaEncendida()) {
                        kWCon += cargas[i]->getPotencia();
                    }
                }
                costos[d][h] = costo(kWCon, precioActual);
                medidor.acumular(kWCon, costos[d][h]);
            }

            // Verificar cumplimiento de horas minimas diarias de cargas aplazables
            if (numCargas > 1 && cargas[1]->getHorasUso() >= MIN_HORAS_CONGELACION) {
                diasCumpleCongelacion++;
            }
            if (numCargas > 2 && cargas[2]->getHorasUso() >= MIN_HORAS_CLIMA) {
                diasCumpleClima++;
            }
        }

        semanaSimulada = true;
        cout << "\n[OK] Simulacion de la semana real (21 al 27 sept 2026) completada." << endl;
    }

    // --- NUEVO EA3: Reporte semanal completo ---
    void mostrarReporteSemanal() {
        if (!semanaSimulada) {
            cout << "\n[!] Primero ejecute la simulacion semanal (Opcion 5)." << endl;
            return;
        }

        cout << fixed << setprecision(0);
        cout << "\n==========================================================================" << endl;
        cout << "          REPORTE SEMANAL WATTCONTROL (21 AL 27 SEPT 2026)                " << endl;
        cout << "==========================================================================" << endl;
        cout << "Negocio: " << nombre << " | Responsable: " << responsable << endl;
        cout << "--------------------------------------------------------------------------" << endl;
        cout << setw(16) << left << "Dia"
             << setw(18) << right << "Sin Gestor (COP)"
             << setw(18) << right << "Con Gestor (COP)"
             << setw(18) << right << "Ahorro (COP)" << endl;
        cout << "--------------------------------------------------------------------------" << endl;

        for (int d = 0; d < DIAS_SEMANA; d++) {
            float diaSin = costo(costosSinGestor[d], HORAS_DIA);
            float diaCon = costo(costos[d], HORAS_DIA);
            float ahorroDia = diaSin - diaCon;

            cout << setw(16) << left << mercado->getNombreDia(d)
                 << setw(18) << right << diaSin
                 << setw(18) << right << diaCon
                 << setw(18) << right << ahorroDia << endl;
        }
        cout << "--------------------------------------------------------------------------" << endl;

        float totalSin = costoSemana(costosSinGestor);
        float totalCon = costoSemana(costos);
        float ahorroSemanal = totalSin - totalCon;
        float ahorroMensual = ahorroSemanal * SEMANAS_MES;
        float porcentajeAhorro = (totalSin > 0) ? (ahorroSemanal / totalSin) * 100.0 : 0.0;

        int dCaro = diaMasCaro(costos);
        int diaHoraCara = 0;
        float precioPico = 0.0;
        int hCara = horaMasCara(diaHoraCara, precioPico);

        cout << "\n--- INDICADORES CLAVE DE LA SEMANA ---" << endl;
        cout << "  * Dia mas caro (Con Gestor):   " << mercado->getNombreDia(dCaro)
             << " ($" << costo(costos[dCaro], HORAS_DIA) << " COP)" << endl;
        cout << "  * Hora mas cara de la semana:  " << mercado->getNombreDia(diaHoraCara)
             << " a las " << hCara << ":00 h ($" << precioPico << " COP/kWh)" << endl;
        cout << "  * Promedio diario con gestor:  $" << promedio(costos) << " COP/dia" << endl;

        cout << "\n--- RESUMEN FINANCIERO Y PROYECCION ---" << endl;
        cout << "  * Costo semanal SIN gestor:    $" << totalSin << " COP" << endl;
        cout << "  * Costo semanal CON gestor:    $" << totalCon << " COP" << endl;
        cout << "  * AHORRO SEMANAL NETO:         $" << ahorroSemanal << " COP ("
             << setprecision(1) << porcentajeAhorro << " %)" << endl;
        cout << setprecision(0);
        cout << "  * PROYECCION AHORRO MENSUAL:   $" << ahorroMensual << " COP (x 4 semanas)" << endl;

        cout << "\n--- CUMPLIMIENTO DE HORAS MINIMAS (CARGAS APLAZABLES) ---" << endl;
        if (numCargas > 1) {
            cout << "  * " << cargas[1]->getNombre() << " (min " << MIN_HORAS_CONGELACION << "h/dia): "
                 << diasCumpleCongelacion << " de " << DIAS_SEMANA << " dias cumplidos." << endl;
        }
        if (numCargas > 2) {
            cout << "  * " << cargas[2]->getNombre() << " (min " << MIN_HORAS_CLIMA << "h/dia): "
                 << diasCumpleClima << " de " << DIAS_SEMANA << " dias cumplidos." << endl;
        }
        cout << "==========================================================================" << endl;
    }
};

#endif