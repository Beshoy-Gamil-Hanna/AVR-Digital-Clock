/*
 * Countdown_Service_program.c
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#include <stddef.h>

#include "../../LIB/std_types.h"
#include "../Buzzer_Service/Buzzer_Service_interface.h"

#include "Countdown_Service_interface.h"


/* ================= Private Variables ================= */

/* Initially Configured Time */
static u8 Countdown_u8InitialHours = 0;
static u8 Countdown_u8InitialMinutes = 0;
static u8 Countdown_u8InitialSeconds = 0;

/* Remaining Time */
static u8 Countdown_u8Hours = 0;
static u8 Countdown_u8Minutes = 0;
static u8 Countdown_u8Seconds = 0;

/* Countdown States */
static u8 Countdown_u8Configured = 0;
static u8 Countdown_u8Running = 0;
static u8 Countdown_u8Finished = 0;
static u8 Countdown_u8Ringing = 0;


/* ================= Public Functions ================= */

/* Initialize Countdown Service */
void Countdown_vidInit(void)
{
    Countdown_u8InitialHours = 0;
    Countdown_u8InitialMinutes = 0;
    Countdown_u8InitialSeconds = 0;

    Countdown_u8Hours = 0;
    Countdown_u8Minutes = 0;
    Countdown_u8Seconds = 0;

    Countdown_u8Configured = 0;
    Countdown_u8Running = 0;
    Countdown_u8Finished = 0;
    Countdown_u8Ringing = 0;

    Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_COUNTDOWN);
}


/*
 * Set Countdown Time.
 * Hours: 0 -> 255
 * Minutes: 0 -> 59
 * Seconds: 0 -> 59
 */
u8 Countdown_u8SetTime(
    u8 Copy_u8Hours,
    u8 Copy_u8Minutes,
    u8 Copy_u8Seconds
)
{
    if((Copy_u8Minutes > 59) ||
       (Copy_u8Seconds > 59) ||
       ((Copy_u8Hours == 0) &&
        (Copy_u8Minutes == 0) &&
        (Copy_u8Seconds == 0)))
    {
        return COUNTDOWN_INVALID_INPUT_ERROR;
    }

    if(Countdown_u8Ringing == 1)
    {
        Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_COUNTDOWN);
        Countdown_u8Ringing = 0;
    }

    Countdown_u8InitialHours = Copy_u8Hours;
    Countdown_u8InitialMinutes = Copy_u8Minutes;
    Countdown_u8InitialSeconds = Copy_u8Seconds;

    Countdown_u8Hours = Copy_u8Hours;
    Countdown_u8Minutes = Copy_u8Minutes;
    Countdown_u8Seconds = Copy_u8Seconds;

    Countdown_u8Configured = 1;
    Countdown_u8Running = 0;
    Countdown_u8Finished = 0;

    return COUNTDOWN_NO_ERROR;
}


/* Start Or Resume Countdown */
void Countdown_vidStart(void)
{
    if((Countdown_u8Configured == 1) &&
       (Countdown_u8Finished == 0) &&
       ((Countdown_u8Hours > 0) ||
        (Countdown_u8Minutes > 0) ||
        (Countdown_u8Seconds > 0)))
    {
        Countdown_u8Running = 1;
    }
}


/* Pause Countdown And Preserve Remaining Time */
void Countdown_vidPause(void)
{
    Countdown_u8Running = 0;
}


/* Restore Initially Configured Time And Stop Countdown */
void Countdown_vidReset(void)
{
    if(Countdown_u8Configured == 1)
    {
        if(Countdown_u8Ringing == 1)
        {
            Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_COUNTDOWN);
            Countdown_u8Ringing = 0;
        }

        Countdown_u8Hours = Countdown_u8InitialHours;
        Countdown_u8Minutes = Countdown_u8InitialMinutes;
        Countdown_u8Seconds = Countdown_u8InitialSeconds;

        Countdown_u8Running = 0;
        Countdown_u8Finished = 0;
    }
}


/* Stop Countdown Sound Without Changing Other Buzzer Sources */
void Countdown_vidStopRinging(void)
{
    if(Countdown_u8Ringing == 1)
    {
        Buzzer_Service_vidRelease(BUZZER_SERVICE_SOURCE_COUNTDOWN);
        Countdown_u8Ringing = 0;
    }
}


/* Countdown Task: Call Once Per Second */
void Countdown_vidTask(void)
{
    if(Countdown_u8Running == 0)
    {
        return;
    }

    if(Countdown_u8Seconds > 0)
    {
        Countdown_u8Seconds--;
    }
    else if(Countdown_u8Minutes > 0)
    {
        Countdown_u8Minutes--;
        Countdown_u8Seconds = 59;
    }
    else if(Countdown_u8Hours > 0)
    {
        Countdown_u8Hours--;
        Countdown_u8Minutes = 59;
        Countdown_u8Seconds = 59;
    }

    if((Countdown_u8Hours == 0) &&
       (Countdown_u8Minutes == 0) &&
       (Countdown_u8Seconds == 0))
    {
        Countdown_u8Running = 0;
        Countdown_u8Finished = 1;
        Countdown_u8Ringing = 1;

        Buzzer_Service_vidRequest(BUZZER_SERVICE_SOURCE_COUNTDOWN);
    }
}


/* Get Remaining Time */
u8 Countdown_u8GetRemainingTime(
    u8 *Copy_pu8Hours,
    u8 *Copy_pu8Minutes,
    u8 *Copy_pu8Seconds
)
{
    if((Copy_pu8Hours == NULL) ||
       (Copy_pu8Minutes == NULL) ||
       (Copy_pu8Seconds == NULL))
    {
        return COUNTDOWN_NULL_POINTER_ERROR;
    }

    *Copy_pu8Hours = Countdown_u8Hours;
    *Copy_pu8Minutes = Countdown_u8Minutes;
    *Copy_pu8Seconds = Countdown_u8Seconds;

    return COUNTDOWN_NO_ERROR;
}


/* Get Countdown Configuration State */
u8 Countdown_u8IsConfigured(void)
{
    return Countdown_u8Configured;
}


/* Get Countdown Running State */
u8 Countdown_u8IsRunning(void)
{
    return Countdown_u8Running;
}


/* Get Countdown Finished State */
u8 Countdown_u8IsFinished(void)
{
    return Countdown_u8Finished;
}


/* Get Countdown Ringing State */
u8 Countdown_u8IsRinging(void)
{
    return Countdown_u8Ringing;
}
