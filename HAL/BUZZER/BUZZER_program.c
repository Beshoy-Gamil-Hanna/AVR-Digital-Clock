/*
 * BUZZER_program.c
 *
 * Created on: Sep 10, 2026
 * Author: Beshoy
 */

#include "../../MCAL/TMR/TMR_interface.h"
#include "../../MCAL/DIO/DIO_interface.h"

#include "BUZZER_config.h"
#include "BUZZER_interface.h"


/* ================= Public Functions ================= */

/*
 * Play A Tone Using OC0 / PB3.
 *
 * Timer0 Must Be Configured For CTC Toggle Mode.
 * The Selected Timer0 Prescaler Must Support The Requested Frequency.
 */
void BUZZER_vidPlay(u32 Copy_u32Frequency)
{
    if(Copy_u32Frequency == 0)
    {
        BUZZER_vidStop();
        return;
    }

    /* Keep The OC0 Pin Disconnected Until Its Direction Is Configured */
    DIO_u8SetPinDirection(
        BUZZER_PORT,
        BUZZER_PIN,
        DIO_PIN_OUTPUT
    );

    /* Configure Timer0 Prescaler And Base Mode */
    TMR0_vidInit();

    /* Configure CTC Toggle Output And Compare Value */
    TMR0_vidGenerateOC0Frequency(Copy_u32Frequency);
}


/*
 * Stop The Tone.
 *
 * Timer0 Is Stopped And The OC0 Pin Is Released.
 */
void BUZZER_vidStop(void)
{
    TMR0_vidStop();

    /* Avoid Leaving The Buzzer Pin Floating High */
    DIO_u8SetPinValue(
        BUZZER_PORT,
        BUZZER_PIN,
        DIO_PIN_LOW
    );

    DIO_u8SetPinDirection(
        BUZZER_PORT,
        BUZZER_PIN,
        DIO_PIN_INPUT
    );
}
