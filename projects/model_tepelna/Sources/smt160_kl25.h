/**
 * @file stm160_kl25.c
 * @brief driver for temperature sensor SMT 160-30 for Freedom board FRDM-KL25Z
 * 
 * @note
 * For input we can use pins:
 * PTA12 (TMP1_CH0) - digital pin 3 on Arduino
 * PTA13 (TMP1_CH1) - digital pin 8 on Arduino 
 * 
 *  Created on: Apr 11, 2014
 *      Author: dolinay
 */

#ifndef SMT160_KL25_H_
#define SMT160_KL25_H_

void smt160_init();
uint16_t smt160_get_temp();		


#endif /* SMT160_KL25_H_ */
