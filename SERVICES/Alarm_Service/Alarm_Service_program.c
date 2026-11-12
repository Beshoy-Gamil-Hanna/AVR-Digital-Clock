/*
 * Alarm_Service_program.c
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#include <stddef.h>

#include "../../LIB/std_types.h"
#include "../../HAL/DS1307/DS1307_interface.h"
#include "../Buzzer_Service/Buzzer_Service_interface.h"

#include "Alarm_Service_interface.h"


/* ================= Private Variables ================= */

static u8 Alarm_u8Hours = 0;
static u8 Alarm_u8Minutes = 0;
static u8 Alarm_u8Date = 0;
static u8 Alarm_u8Month = 0;
static u8 Alarm_u8Year = 0;

static u8 Alarm_u8Configured = 0;
static u8 Alarm_u8Enabled = 0;
static u8 Alarm_u8Ringing = 0;
static u8 Alarm_u8Triggered = 0;
static u8 Alarm_u8Expired = 0;


/* ================= Private Functions ================= */

static u8 Alarm_u8ValidateDate(u8 Copy_u8Date,
                               u8 Copy_u8Month,
                               u8 Copy_u8Year)
{
    u8 Local_u8DaysInMonth;

    if((Copy_u8Month < 1) || (Copy_u8Month > 12) ||
       (Copy_u8Date < 1) || (Copy_u8Year > 99))
    {
        return ALARM_INVALID_INPUT_ERROR;
    }

    switch(Copy_u8Month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            Local_u8DaysInMonth = 31;
            break;

        case 4:
        case 6:
        case 9:
        case 11:
            Local_u8DaysInMonth = 30;
            break;

        case 2:
            Local_u8DaysInMonth = ((Copy_u8Year % 4U) == 0U) ? 29U : 28U;
            break;

        default:
            return ALARM_INVALID_INPUT_ERROR;
    }

    if(Copy_u8Date > Local_u8DaysInMonth)
    {
        return ALARM_INVALID_INPUT_ERROR;
    }

    return ALARM_NO_ERROR;
}


/* Return 1 when the current RTC date is later than the alarm date. */
static u8 Alarm_u8IsCurrentDateAfterTarget(u8 Copy_u8CurrentDate,
                                           u8 Copy_u8CurrentMonth,
                                           u8 Copy_u8CurrentYear)
{
    if(Copy_u8CurrentYear > Alarm_u8Year)
    {
        return 1;
    }

    if(Copy_u8CurrentYear < Alarm_u8Year)
    {
        return 0;
    }

    if(Copy_u8CurrentMonth > Alarm_u8Month)
    {
        return 1;
    }

    if(Copy_u8CurrentMonth < Alarm_u8Month)
    {
        return 0;
    }

    return (Copy_u8CurrentDate > Alarm_u8Date);
}


/* ================= Public Functions ================= */

void Alarm_vidInit(void)
{
    Alarm_u8Hours = 0;
    Alarm_u8Minutes = 0;
    Alarm_u8Date = 0;
    Alarm_u8Month = 0;
    Alarm_u8Year = 0;

    Alarm_u8Configured = 0;
    Alarm_u8Enabled = 0;
    Alarm_u8Ringing = 0;
    Alarm_u8Triggered = 0;
    Alarm_u8Expired = 0;

    Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_ALARM);
}


/*
 * Configure a one-shot alarm for a calendar date and time.
 * Hours: 0..23, Minutes: 0..59, Date: DD/MM/YY.
 */
u8 Alarm_u8SetDateTime(u8 Copy_u8Hours,
                        u8 Copy_u8Minutes,
                        u8 Copy_u8Date,
                        u8 Copy_u8Month,
                        u8 Copy_u8Year)
{
    if((Copy_u8Hours > 23) || (Copy_u8Minutes > 59))
    {
        return ALARM_INVALID_INPUT_ERROR;
    }

    if(Alarm_u8ValidateDate(Copy_u8Date,
                            Copy_u8Month,
                            Copy_u8Year) != ALARM_NO_ERROR)
    {
        return ALARM_INVALID_INPUT_ERROR;
    }

    if(Alarm_u8Ringing == 1)
    {
        Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_ALARM);
    }

    Alarm_u8Hours = Copy_u8Hours;
    Alarm_u8Minutes = Copy_u8Minutes;
    Alarm_u8Date = Copy_u8Date;
    Alarm_u8Month = Copy_u8Month;
    Alarm_u8Year = Copy_u8Year;

    Alarm_u8Configured = 1;
    Alarm_u8Enabled = 1;
    Alarm_u8Ringing = 0;
    Alarm_u8Triggered = 0;
    Alarm_u8Expired = 0;

    return ALARM_NO_ERROR;
}


/* Set alarm for today's RTC date for compatibility with earlier callers. */
u8 Alarm_u8SetTime(u8 Copy_u8Hours, u8 Copy_u8Minutes)
{
    u8 Local_u8Day;
    u8 Local_u8Date;
    u8 Local_u8Month;
    u8 Local_u8Year;
    u8 Local_u8Status;

    Local_u8Status = DS1307_u8GetDate(
        &Local_u8Day,
        &Local_u8Date,
        &Local_u8Month,
        &Local_u8Year
    );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return ALARM_RTC_ERROR;
    }

    return Alarm_u8SetDateTime(
        Copy_u8Hours,
        Copy_u8Minutes,
        Local_u8Date,
        Local_u8Month,
        Local_u8Year
    );
}


void Alarm_vidEnable(void)
{
    if((Alarm_u8Configured == 1) &&
       (Alarm_u8Triggered == 0) &&
       (Alarm_u8Expired == 0))
    {
        Alarm_u8Enabled = 1;
    }
}


void Alarm_vidDisable(void)
{
    Alarm_u8Enabled = 0;

    if(Alarm_u8Ringing == 1)
    {
        Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_ALARM);
        Alarm_u8Ringing = 0;
    }
}


/* Silence this alarm's request only; other sound sources remain active. */
void Alarm_vidStopRinging(void)
{
    if(Alarm_u8Ringing == 1)
    {
        Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_ALARM);
        Alarm_u8Ringing = 0;
    }
}


/*
 * Call every second from the APP scheduler, regardless of visible page.
 * The date-qualified alarm rings once and is then marked as triggered.
 */
void Alarm_vidTask(void)
{
    u8 Local_u8Hours;
    u8 Local_u8Minutes;
    u8 Local_u8Seconds;
    u8 Local_u8Day;
    u8 Local_u8Date;
    u8 Local_u8Month;
    u8 Local_u8Year;

    if((Alarm_u8Enabled == 0) || (Alarm_u8Configured == 0))
    {
        return;
    }

    if(DS1307_u8GetTime(&Local_u8Hours,
                        &Local_u8Minutes,
                        &Local_u8Seconds) != DS1307_NO_ERROR)
    {
        return;
    }

    if(DS1307_u8GetDate(&Local_u8Day,
                        &Local_u8Date,
                        &Local_u8Month,
                        &Local_u8Year) != DS1307_NO_ERROR)
    {
        return;
    }

    /* If the scheduled date has passed, mark the alarm as missed. */
    if(Alarm_u8IsCurrentDateAfterTarget(Local_u8Date,
                                        Local_u8Month,
                                        Local_u8Year) == 1)
    {
        Alarm_u8Enabled = 0;
        Alarm_u8Expired = 1;
        return;
    }

    if((Local_u8Date == Alarm_u8Date) &&
       (Local_u8Month == Alarm_u8Month) &&
       (Local_u8Year == Alarm_u8Year) &&
       (Local_u8Hours == Alarm_u8Hours) &&
       (Local_u8Minutes == Alarm_u8Minutes))
    {
        if(Alarm_u8Triggered == 0)
        {
            /* One-shot alarm: do not trigger again after dismissal. */
            Alarm_u8Enabled = 0;
            Alarm_u8Triggered = 1;
            Alarm_u8Ringing = 1;

            Buzzer_Service_vidRequest(BUZZER_SERVICE_SOURCE_ALARM);
        }
    }
}


u8 Alarm_u8GetDateTime(u8 *Copy_pu8Hours,
                       u8 *Copy_pu8Minutes,
                       u8 *Copy_pu8Date,
                       u8 *Copy_pu8Month,
                       u8 *Copy_pu8Year)
{
    if((Copy_pu8Hours == NULL) ||
       (Copy_pu8Minutes == NULL) ||
       (Copy_pu8Date == NULL) ||
       (Copy_pu8Month == NULL) ||
       (Copy_pu8Year == NULL))
    {
        return ALARM_NULL_POINTER_ERROR;
    }

    *Copy_pu8Hours = Alarm_u8Hours;
    *Copy_pu8Minutes = Alarm_u8Minutes;
    *Copy_pu8Date = Alarm_u8Date;
    *Copy_pu8Month = Alarm_u8Month;
    *Copy_pu8Year = Alarm_u8Year;

    return ALARM_NO_ERROR;
}


u8 Alarm_u8GetTime(u8 *Copy_pu8Hours, u8 *Copy_pu8Minutes)
{
    if((Copy_pu8Hours == NULL) || (Copy_pu8Minutes == NULL))
    {
        return ALARM_NULL_POINTER_ERROR;
    }

    *Copy_pu8Hours = Alarm_u8Hours;
    *Copy_pu8Minutes = Alarm_u8Minutes;

    return ALARM_NO_ERROR;
}


u8 Alarm_u8IsConfigured(void)
{
    return Alarm_u8Configured;
}


u8 Alarm_u8IsEnabled(void)
{
    return Alarm_u8Enabled;
}


u8 Alarm_u8IsRinging(void)
{
    return Alarm_u8Ringing;
}


u8 Alarm_u8IsTriggered(void)
{
    return Alarm_u8Triggered;
}


u8 Alarm_u8IsExpired(void)
{
    return Alarm_u8Expired;
}
