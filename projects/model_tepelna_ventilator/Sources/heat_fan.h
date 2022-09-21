/*
 * heatfan.h.h
 *
 * Driver for heating plant with fan which connects to UTB learning kit
 * with FRDM kl25z
 */

#ifndef SOURCES_HEATFAN_H_
#define SOURCES_HEATFAN_H_

typedef enum {
	HEATFAN_Heater, HEATFAN_Fan
} HEATFAN_Actuator;

void HEATFAN_Init(void);
int16_t HEATFAN_GetTemperature(void);
uint16_t HEATFAN_GetFanRPM(void);
void HEATFAN_CtrlSignalSel(HEATFAN_Actuator vyber);
void HEATFAN_SetPWMDuty(uint8_t pwm_percents);


#endif /* SOURCES_HEATFAN_H_ */
