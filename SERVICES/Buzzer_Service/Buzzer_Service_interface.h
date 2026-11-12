/*
 * Buzzer_Service_interface.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef SERVICES_BUZZER_SERVICE_BUZZER_SERVICE_INTERFACE_H_
#define SERVICES_BUZZER_SERVICE_BUZZER_SERVICE_INTERFACE_H_

#include "../../LIB/std_types.h"


/* ================= Sound Request Sources ================= */

#define BUZZER_SERVICE_SOURCE_ALARM       0x01U
#define BUZZER_SERVICE_SOURCE_COUNTDOWN   0x02U


/* ================= Ringtone IDs ================= */

#define BUZZER_RINGTONE_NOKIA             0U
#define BUZZER_RINGTONE_JINGLE_BELLS      1U
#define BUZZER_RINGTONE_WESTMINSTER       2U
#define BUZZER_RINGTONE_COUNT             3U


/* ================= Function Prototypes ================= */

void Buzzer_Service_vidInit(void);

void Buzzer_Service_vidRequest(u8 Copy_u8Source);

void Buzzer_Service_vidRelease(u8 Copy_u8Source);

u8 Buzzer_Service_u8IsActive(u8 Copy_u8Source);

u8 Buzzer_Service_u8SetRingtone(u8 Copy_u8Source, u8 Copy_u8Ringtone);

u8 Buzzer_Service_u8GetRingtone(u8 Copy_u8Source);

/* Called by the Timer1 ISR every 10 ms. */
void Buzzer_Service_vidTimerTick10ms(void);

/* Called continuously from APP_vidTask(). */
void Buzzer_Service_vidTask(void);

#endif /* SERVICES_BUZZER_SERVICE_BUZZER_SERVICE_INTERFACE_H_ */
