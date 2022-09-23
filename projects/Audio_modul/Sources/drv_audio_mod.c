/*
 * drv_audio_mod.c
 *
 *  Created on: 16. 11. 2021
 *      Author: student
 */

#include "i2c.h"
#include "drv_audio_mod.h"


//Nastaven� hlasitosti
uint8_t audio_Set_volume (uint8_t value){

  if (((value == 0)||(value > 0)) && (value < 64)){
      IIC_write_byte(AUDIOPROC_ADDRESS, value);
      return 1;
  } else
      return 0;
}



//Nastaven� vstupu
uint8_t audio_Set_input (uint8_t input_num, uint8_t value){

  uint8_t input_gain = 0b01000000 + input_num;

  if (((input_num == 0)||(input_num > 0)) && (input_num < 4) && ((value == 0)||(value > 0)) && (value < 4)){
      //konverze dat
      value = ~value & 0x3;
      value = value << 3;
      input_gain |= value;

      IIC_write_byte(AUDIOPROC_ADDRESS, input_gain);    // odesl�n� dat pomoc� I2C

      return 1;
  } else
      return 0;
}



//Nastaven� hlasitosti reproduktoru
uint8_t audio_Speaker_attenuators (uint8_t speaker_num, uint8_t value) {

  if (((speaker_num == 0)||(speaker_num > 0)) && (speaker_num <= 3) && ((value == 0)||(value > 0)) && (value < 32)){
      //konverze dat
      speaker_num += 4;
      speaker_num <<= 5;
      speaker_num += value;

      IIC_write_byte(AUDIOPROC_ADDRESS, speaker_num);

      return 1;
  } else
      return 0;
}


//Nastaven� bas�
uint8_t audio_Set_bass (uint8_t value){
  if ((((value == 0)||(value > 0)) && (value < 8)) || ((value > 248) && ((value == 255)||(value < 255)))){

      if (((value == 0)||(value > 0)) && (value < 8)){
          //vyhodnocen� kladn�ho value
          value |= 0xF8;
          value ^= 0xFF;

          value += 104;
          IIC_write_byte(AUDIOPROC_ADDRESS, value);
      } else {
          //vyhodnocen� z�porn�ho value
          value -= 1;
          value &= 0x07;

          value += 96;
          IIC_write_byte(AUDIOPROC_ADDRESS, value);
      }
      return 1;
  } else
      return 0;
}


//Nastaven� v��ek
uint8_t audio_Set_treble (uint8_t value){
  if ((((value == 0)||(value > 0)) && (value < 8)) || ((value > 248) && ((value == 255)||(value < 255)))){

      if (((value == 0)||(value > 0)) && (value < 8)){
          //vyhodnocen� kladn�ho value
          value |= 0xF8;
          value ^= 0xFF;

          value += 120;
          IIC_write_byte(AUDIOPROC_ADDRESS, value);
      } else {
          //vyhodnocen� z�porn�ho value
          value -= 1;
          value &= 0x07;

          value += 112;
          IIC_write_byte(AUDIOPROC_ADDRESS, value);
      }
      return 1;
  } else
      return 0;
}


//Inicializace modulu
void audio_Init(void)
{
	i2c_init();
	audio_Set_input(INPUT_STEREO_1, 0);				// V�b�r vstupu Stereo 1, p�edzes�len� vstupu 0 dB
	audio_Speaker_attenuators (ATT_SPEAKER_LF, 0);	// Nastaven� balance p�edn�ho lev�ho a prav�ho reproduktoru
	audio_Speaker_attenuators (ATT_SPEAKER_RF, 0);
	audio_Set_volume(63);							// Pozor, 0 = max. hlasitost, 63 = min. hlasitost
}

