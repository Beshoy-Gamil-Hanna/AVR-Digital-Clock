/*
 * Stopwatch_Service_interface.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef SERVICES_STOPWATCH_SERVICE_STOPWATCH_SERVICE_INTERFACE_H_
#define SERVICES_STOPWATCH_SERVICE_STOPWATCH_SERVICE_INTERFACE_H_

#include "../../LIB/std_types.h"

/* Error Status */

typedef enum
{
    STOPWATCH_NO_ERROR = 0,
    STOPWATCH_NULL_POINTER_ERROR

} Stopwatch_ErrorStatus_t;

/* Function Prototypes */

void Stopwatch_vidInit(void);

void Stopwatch_vidStart(void);

void Stopwatch_vidStop(void);

void Stopwatch_vidReset(void);

void Stopwatch_vidTask(void);

u8 Stopwatch_u8GetTime(u16 *Copy_pu16Minutes,
                       u8 *Copy_pu8Seconds);

u8 Stopwatch_u8IsRunning(void);

#endif
