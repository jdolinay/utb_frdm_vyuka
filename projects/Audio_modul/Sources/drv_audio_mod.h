/*
 * drv_audio_mod.h
 *
 *  Created on: 16. 11. 2021
 *      Author: student
 */

#ifndef SOURCES_DRV_AUDIO_MOD_H_
#define SOURCES_DRV_AUDIO_MOD_H_

#define	AUDIOPROC_ADDRESS (0x88)

#define INPUT_STEREO_1	(0)
#define INPUT_STEREO_2	(1)
#define INPUT_STEREO_3	(2)
#define INPUT_STEREO_4	(3)

#define ATT_SPEAKER_LF	(0)
#define ATT_SPEAKER_RF	(1)
#define ATT_SPEAKER_LR	(2)
#define ATT_SPEAKER_RR	(3)


void audio_Init(void);
uint8_t audio_Set_volume(uint8_t value);
uint8_t audio_Set_input(uint8_t input_num, uint8_t value);
uint8_t audio_Speaker_attenuators(uint8_t speaker_num, uint8_t value);
uint8_t audio_Set_bass(uint8_t value);
uint8_t audio_Set_treble(uint8_t value);


#endif /* SOURCES_DRV_AUDIO_MOD_H_ */
