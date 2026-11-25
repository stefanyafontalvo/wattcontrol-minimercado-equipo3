#ifndef MERCADO_H
#define MERCADO_H

#include <cstdlib>
#include <string>
#include "Constantes.h"

using namespace std;

class Mercado {
private:
    // Precios reales de bolsa XM (21 al 27 de septiembre de 2026) - COP/kWh
    const float PRECIOS[DIAS_SEMANA][HORAS_DIA] = {
        {398,398,461,502,502,511,511,502,511,502,511,565,565,565,596,596,753,753,1093,1093,884,753,596,565}, // Lun 21
        {415,415,421,499,499,538,538,510,499,499,538,573,538,573,661,712,1040,712,1611,837,860,712,661,661}, // Mar 22
        {463,465,465,465,465,467,465,467,465,465,465,465,465,465,504,648,656,656,1612,1273,940,656,648,580}, // Mie 23
        {569,569,569,569,569,569,569,569,569,495,569,569,569,569,712,712,712,712,1601,1601,928,712,712,712}, // Jue 24
        {526,526,526,526,526,537,537,642,537,537,537,642,537,970,970,970,970,970,1603,1603,970,970,970,833}, // Vie 25
        {584,584,598,603,603,640,640,603,598,598,598,705,705,705,705,705,705,705,1602,751,705,705,640,598}, // Sab 26
        {456,570,518,609,609,609,570,456,456,456,456,456,456,456,609,609,641,644,890,890,890,817,644,620}  // Dom 27
    };

    const string NOMBRES_DIAS[DIAS_SEMANA] = {
        "Lunes 21", "Martes 22", "Miercoles 23", "Jueves 24", "Viernes 25", "Sabado 26", "Domingo 27"
    };

public:
    float precio(int dia, int hora) {
        if (dia >= 0 && dia < DIAS_SEMANA && hora >= 0 && hora < HORAS_DIA) {
            return PRECIOS[dia][hora];
        }
        return 0.0;
    }

    string getNombreDia(int dia) {
        if (dia >= 0 && dia < DIAS_SEMANA) return NOMBRES_DIAS[dia];
        return "Desconocido";
    }

    float precioHoraAleatorio(int hora) {
        int minimo, maximo;
        if (hora <= 6)       { minimo = 398; maximo = 640;  }
        else if (hora <= 14) { minimo = 456; maximo = 970;  }
        else if (hora <= 17) { minimo = 596; maximo = 1040; }
        else if (hora <= 20) { minimo = 705; maximo = 1612; }
        else                 { minimo = 565; maximo = 970;  }
        return minimo + rand() % (maximo - minimo + 1);
    }

    float nivelEmbalsesAleatorio() {
        return EMBALSE_BASE_RAND + (rand() % EMBALSE_RANGO_RAND);
    }
};

#endif