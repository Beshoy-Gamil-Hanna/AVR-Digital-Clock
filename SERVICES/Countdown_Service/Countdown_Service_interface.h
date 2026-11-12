/*
 * Countdown_Service_interface.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef SERVICES_COUNTDOWN_SERVICE_COUNTDOWN_SERVICE_INTERFACE_H_
#define SERVICES_COUNTDOWN_SERVICE_COUNTDOWN_SERVICE_INTERFACE_H_

#include "../../LIB/std_types.h"

/* Error Status */
typedef enum
{
    COUNTDOWN_NO_ERROR = 0,
    COUNTDOWN_INVALID_INPUT_ERROR,
    COUNTDOWN_NULL_POINTER_ERROR

} Countdown_ErrorStatus_t;

/* Function Prototypes */
void Countdown_vidInit(void);

u8 Countdown_u8SetTime(
    u8 Copy_u8Hours,
    u8 Copy_u8Minutes,
    u8 Copy_u8Seconds
);

void Countdown_vidStart(void);
void Countdown_vidPause(void);
void Countdown_vidReset(void);
void Countdown_vidStopRinging(void);
void Countdown_vidTask(void);

u8 Countdown_u8GetRemainingTime(
    u8 *Copy_pu8Hours,
    u8 *Copy_pu8Minutes,
    u8 *Copy_pu8Seconds
);

u8 Countdown_u8IsConfigured(void);
u8 Countdown_u8IsRunning(void);
u8 Countdown_u8IsFinished(void);
u8 Countdown_u8IsRinging(void);

#endif /* SERVICES_COUNTDOWN_SERVICE_COUNTDOWN_SERVICE_INTERFACE_H_ */
