/*
* Driver for model of 7-segment display with keypad.
* Ovladac pro modul 7-segmentoveho displeje s klavesnici
* Vyuziva casovac TPM1
*/
#ifndef SOURCES_7SEGKEYPAD_H_
#define SOURCES_7SEGKEYPAD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SEGKEYPAD_InitializeDisplay(void);
void SEGKEYPAD_InitializeKeyboard(void);

void SEGKEYPAD_SelectDisplay(int display);
void SEGKEYPAD_ShowNumber(int number);

int SEGKEYPAD_GetKey(void);

#ifdef __cplusplus
}
#endif



#endif /* SOURCES_7SEGKEYPAD_H_ */
