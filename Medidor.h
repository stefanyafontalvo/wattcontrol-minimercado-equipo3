#ifndef MEDIDOR_H
#define MEDIDOR_H

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

#endif