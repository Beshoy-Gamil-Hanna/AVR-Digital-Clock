/*
 * Alarm_Service_interface.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef SERVICES_ALARM_SERVICE_ALARM_SERVICE_INTERFACE_H_
#define SERVICES_ALARM_SERVICE_ALARM_SERVICE_INTERFACE_H_

#include "../../LIB/std_types.h"


/* ================= Error Status ================= */

typedef enum
{
    ALARM_NO_ERROR = 0,
    ALARM_INVALID_INPUT_ERROR,
    ALARM_RTC_ERROR,
    ALARM_NULL_POINTER_ERROR

} Alarm_ErrorStatus_t;


/* ================= Function Prototypes ================= */

void Alarm_vidInit(void);

/* One-shot alarm schedule: date DD/MM/YY and time HH:MM (24-hour input). */
u8 Alarm_u8SetDateTime(
    u8 Copy_u8Hours,
    u8 Copy_u8Minutes,
    u8 Copy_u8Date,
    u8 Copy_u8Month,
    u8 Copy_u8Year
);

/* Backward-compatible helper: set time for the current RTC date. */
u8 Alarm_u8SetTime(u8 Copy_u8Hours, u8 Copy_u8Minutes);

void Alarm_vidEnable(void);
void Alarm_vidDisable(void);
void Alarm_vidStopRinging(void);
void Alarm_vidTask(void);

u8 Alarm_u8GetDateTime(
    u8 *Copy_pu8Hours,
    u8 *Copy_pu8Minutes,
    u8 *Copy_pu8Date,
    u8 *Copy_pu8Month,
    u8 *Copy_pu8Year
);

u8 Alarm_u8GetTime(u8 *Copy_pu8Hours, u8 *Copy_pu8Minutes);
u8 Alarm_u8IsConfigured(void);
u8 Alarm_u8IsEnabled(void);
u8 Alarm_u8IsRinging(void);
u8 Alarm_u8IsTriggered(void);
u8 Alarm_u8IsExpired(void);

#endif /* SERVICES_ALARM_SERVICE_ALARM_SERVICE_INTERFACE_H_ */
