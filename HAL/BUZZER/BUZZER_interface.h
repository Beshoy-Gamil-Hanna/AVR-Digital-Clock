/*
 /*
 * BUZZER_interface.h
 *
 * Created on: Sep 10, 2026
 * Author: Beshoy
 */

#ifndef HAL_BUZZER_BUZZER_INTERFACE_H_
#define HAL_BUZZER_BUZZER_INTERFACE_H_


#include "../../LIB/std_types.h"


/* ================= Octave 4 ================= */

#define NOTE_C4     262U
#define NOTE_CS4    277U
#define NOTE_D4     294U
#define NOTE_DS4    311U
#define NOTE_E4     330U
#define NOTE_F4     349U
#define NOTE_FS4    370U
#define NOTE_G4     392U
#define NOTE_GS4    415U
#define NOTE_A4     440U
#define NOTE_AS4    466U
#define NOTE_B4     494U


/* ================= Octave 5 ================= */

#define NOTE_C5     523U
#define NOTE_CS5    554U
#define NOTE_D5     587U
#define NOTE_DS5    622U
#define NOTE_E5     659U
#define NOTE_F5     698U
#define NOTE_FS5    740U
#define NOTE_G5     784U
#define NOTE_GS5    831U
#define NOTE_A5     880U
#define NOTE_AS5    932U
#define NOTE_B5     988U


/* ================= Octave 6 ================= */

#define NOTE_C6     1047U
#define NOTE_CS6    1109U
#define NOTE_D6     1175U
#define NOTE_DS6    1245U
#define NOTE_E6     1319U
#define NOTE_F6     1397U
#define NOTE_FS6    1480U
#define NOTE_G6     1568U
#define NOTE_GS6    1661U
#define NOTE_A6     1760U
#define NOTE_AS6    1865U
#define NOTE_B6     1976U


/* ================= Special Note ================= */

#define NOTE_REST   0U


/* ================= Function Prototypes ================= */

void BUZZER_vidPlay(u32 Copy_u32Frequency);

void BUZZER_vidStop(void);


#endif /* HAL_BUZZER_BUZZER_INTERFACE_H_ */
