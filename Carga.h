#ifndef CARGA_H
#define CARGA_H

#include <string>
#include "Constantes.h"

using namespace std;

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

class CargaCritica : public Carga {
public:
    CargaCritica(string n, float p) : Carga(n, p) {}
    bool debeEncender(int hora, float precio, float embalses) override {
        return true; // Regla 1: Siempre encendida
    }
};

class CargaAplazable : public Carga {
public:
    CargaAplazable(string n, float p) : Carga(n, p) {}
    bool debeEncender(int hora, float precio, float embalses) override {
        bool horaPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool aplazar = (embalses < EMBALSES_ALERTA) || horaPico || (precio > PRECIO_CARO);
        return !aplazar;
    }
};

#endif