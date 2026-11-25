#ifndef CONSTANTES_H
#define CONSTANTES_H

// --- Dimensiones de Tiempo y Arreglos ---
const int DIAS_SEMANA           = 7;
const int HORAS_DIA             = 24;
const int MAX_CARGAS            = 3;

// --- Constantes del Negocio (Minimercado El Rosario - Digito 8) ---
const float POTENCIA_VITRINA    = 3.0;  // Vitrina Refrigerada / Carga Critica (kW)
const float POTENCIA_CONGELACION= 8.0;  // Sistemas de Congelacion / Aplazable 1 (kW)
const float POTENCIA_CLIMA      = 4.5;  // Iluminacion y Climatizacion / Aplazable 2 (kW)

const float POTENCIA_CONTRATADA = 15.0; // Maxima potencia permitida (kW)
const float PRECIO_CARO         = 600.0;// COP/kWh (Umbral aplazamiento)
const float PRECIO_EMERGENCIA   = 1500.0;// COP/kWh (Umbral racionamiento)
const float EMBALSES_ALERTA     = 40.0; // Nivel critico embalses (%)
const float EMBALSES_SEMANA     = 78.0; // Nivel real sept 2026

// --- Franjas Horarias y Validaciones ---
const int PICO_INICIO           = 18;   // 6 PM
const int PICO_FIN              = 20;   // 8 PM
const int HORA_MIN              = 0;
const int HORA_MAX              = 23;
const float EMBALSE_MIN         = 0.0;
const float EMBALSE_MAX         = 100.0;

// --- Parametros de Simulacion ---
const int MIN_HORAS_CONGELACION = 6;    // Horas minimas diarias requeridas
const int MIN_HORAS_CLIMA       = 4;
const int FALLA_CADA            = 20;
const int EMBALSE_BASE_RAND     = 30;
const int EMBALSE_RANGO_RAND    = 31;
const int SEMANAS_MES           = 4;    // Proyeccion mensual (x 4)
const int LONGITUD_MIN_TEXTO    = 3;    // Validacion string

#endif