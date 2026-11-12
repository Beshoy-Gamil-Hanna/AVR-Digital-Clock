/*
 /*
 * Stopwatch_Service_program.c
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#include "../../LIB/std_types.h"

#include "Stopwatch_Service_interface.h"


/* ================= Private Variables ================= */

static u16 Stopwatch_u16Minutes = 0;
static u8 Stopwatch_u8Seconds = 0;

static u8 Stopwatch_u8Running = 0;


/* ================= Public Functions ================= */


/*
 * Initialize Stopwatch Service
 */

void Stopwatch_vidInit(void)
{
    Stopwatch_u16Minutes = 0;
    Stopwatch_u8Seconds = 0;

    Stopwatch_u8Running = 0;
}


/*
 * Start Stopwatch
 */

void Stopwatch_vidStart(void)
{
    Stopwatch_u8Running = 1;
}


/*
 * Stop Stopwatch
 *
 * Keep Current Time
 */

void Stopwatch_vidStop(void)
{
    Stopwatch_u8Running = 0;
}


/*
 * Reset Stopwatch
 *
 * Clear Time and Stop Counting
 */

void Stopwatch_vidReset(void)
{
    Stopwatch_u16Minutes = 0;
    Stopwatch_u8Seconds = 0;

    Stopwatch_u8Running = 0;
}


/*
 * Stopwatch Task
 *
 * Must Be Called Once Every Second
 */

void Stopwatch_vidTask(void)
{
    if(Stopwatch_u8Running == 1)
    {
        Stopwatch_u8Seconds++;

        if(Stopwatch_u8Seconds >= 60)
        {
            Stopwatch_u8Seconds = 0;
            Stopwatch_u16Minutes++;
        }
    }
}


/*
 * Get Stopwatch Time
 */

u8 Stopwatch_u8GetTime(u16 *Copy_pu16Minutes,
                       u8 *Copy_pu8Seconds)
{
    if((Copy_pu16Minutes == NULL) ||
       (Copy_pu8Seconds == NULL))
    {
        return STOPWATCH_NULL_POINTER_ERROR;
    }

    *Copy_pu16Minutes = Stopwatch_u16Minutes;
    *Copy_pu8Seconds = Stopwatch_u8Seconds;

    return STOPWATCH_NO_ERROR;
}


/*
 * Get Stopwatch Running State
 */

u8 Stopwatch_u8IsRunning(void)
{
    return Stopwatch_u8Running;
}
