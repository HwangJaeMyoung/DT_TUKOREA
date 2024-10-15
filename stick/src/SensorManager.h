#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include "Globals.h"

static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
void initialize_sensors();

#endif