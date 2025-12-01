#ifndef SEISMIC_WAVES_H
#define SEISMIC_WAVES_H

#include <stdint.h>


typedef struct {
    double u;          // Текущее смещение (амплитуда волны)
    double u_prev;     // Смещение на предыдущем шаге (для расчета скорости/ускорения)
    
    double velocity_sqr; // Квадрат скорости волны в данной точке (c^2 * dt^2 / dx^2)
    double damping;      // Коэффициент затухания (поглощение энергии грунтом)
} cellBody;

const int cellSize = sizeof(cellBody);

#define MODEL_TYPE_WAVE 2
#define ARRAY_TOPOLOGY_2D 20

#endif