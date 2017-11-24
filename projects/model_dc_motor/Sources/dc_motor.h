/*
 * dc_motor.h
 * Jednoduchy ovladac pro model DC motorku pripojeny k vyvojovemu kitu utb.
 */

#ifndef SOURCES_DC_MOTOR_H_
#define SOURCES_DC_MOTOR_H_

void DCMOTOR_setDirection(char dir);
void DCMOTOR_Init();
void DCMOTOR_SpinON();
void DCMOTOR_SpinOFF();
int DCMOTOR_LapCheck();

// knihovna pro ovládání motoru:
// Obsahuje funkce nastav smer,inicializaci,toèení(SpinON),
//netoèení(SpinOFF),zmenu smeru a funkci pro signalizaci jedné otáèky

#endif /* SOURCES_DC_MOTOR_H_ */
