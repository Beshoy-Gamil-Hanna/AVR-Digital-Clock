/*
 * KPD_program.c
 *
 * Created on: Aug 10, 2026
 * Author: Beshoy
 */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <util/delay.h>

#include "../../LIB/std_types.h"
#include "../../MCAL/DIO/DIO_interface.h"

#include "KPD_config.h"
#include "KPD_interface.h"


/* ================= Private Variables ================= */

static u8 KPD_u8Initialized = 0;


/* ================= Public Functions ================= */

/*
 * Initialize Keypad Pins.
 * Columns are outputs and rows are inputs with pull-up resistors enabled.
 */
void KPD_vidInit(void)
{
    u8 Local_u8ColArr[4] =
    {
        KPD_COL0,
        KPD_COL1,
        KPD_COL2,
        KPD_COL3
    };

    u8 Local_u8RowArr[4] =
    {
        KPD_ROW0,
        KPD_ROW1,
        KPD_ROW2,
        KPD_ROW3
    };

    u8 Local_u8Index;

    for(Local_u8Index = 0; Local_u8Index < 4; Local_u8Index++)
    {
        DIO_u8SetPinDirection(
            KPD_COL_PORT,
            Local_u8ColArr[Local_u8Index],
            DIO_PIN_OUTPUT
        );

        DIO_u8SetPinValue(
            KPD_COL_PORT,
            Local_u8ColArr[Local_u8Index],
            DIO_PIN_HIGH
        );

        DIO_u8SetPinDirection(
            KPD_ROW_PORT,
            Local_u8RowArr[Local_u8Index],
            DIO_PIN_INPUT
        );

        /* Enable Internal Pull-up */
        DIO_u8SetPinValue(
            KPD_ROW_PORT,
            Local_u8RowArr[Local_u8Index],
            DIO_PIN_HIGH
        );
    }

    KPD_u8Initialized = 1;
}


/*
 * Get Pressed Key.
 *
 * Standard keypad layout:
 * 1 2 3 A
 * 4 5 6 B
 * 7 8 9 C
 * * 0 # D
 *
 * The function returns one key event and waits for its release,
 * preventing one long press from being read repeatedly.
 */
u8 KPD_u8GetPressedKey(void)
{
    u8 Local_u8RowValue;
    u8 Local_u8ColIndex;
    u8 Local_u8RowIndex;

    u8 Local_u8ColArr[4] =
    {
        KPD_COL0,
        KPD_COL1,
        KPD_COL2,
        KPD_COL3
    };

    u8 Local_u8RowArr[4] =
    {
        KPD_ROW0,
        KPD_ROW1,
        KPD_ROW2,
        KPD_ROW3
    };

    static const u8 Local_au8KeyMap[4][4] =
    {
        {'1', '2', '3', KPD_KEY_A},
        {'4', '5', '6', KPD_KEY_B},
        {'7', '8', '9', KPD_KEY_C},
        {KPD_KEY_STAR, '0', KPD_KEY_HASH, KPD_KEY_D}
    };

    if(KPD_u8Initialized == 0)
    {
        KPD_vidInit();
    }

    for(Local_u8ColIndex = 0; Local_u8ColIndex < 4; Local_u8ColIndex++)
    {
        /* Activate Current Column */
        DIO_u8SetPinValue(
            KPD_COL_PORT,
            Local_u8ColArr[Local_u8ColIndex],
            DIO_PIN_LOW
        );

        for(Local_u8RowIndex = 0; Local_u8RowIndex < 4; Local_u8RowIndex++)
        {
            DIO_u8GetPinValue(
                KPD_ROW_PORT,
                Local_u8RowArr[Local_u8RowIndex],
                &Local_u8RowValue
            );

            if(Local_u8RowValue == DIO_PIN_LOW)
            {
                /* Debounce */
                _delay_ms(20);

                DIO_u8GetPinValue(
                    KPD_ROW_PORT,
                    Local_u8RowArr[Local_u8RowIndex],
                    &Local_u8RowValue
                );

                if(Local_u8RowValue == DIO_PIN_LOW)
                {
                    /* Wait Until The Key Is Released */
                    do
                    {
                        DIO_u8GetPinValue(
                            KPD_ROW_PORT,
                            Local_u8RowArr[Local_u8RowIndex],
                            &Local_u8RowValue
                        );
                    }
                    while(Local_u8RowValue == DIO_PIN_LOW);

                    _delay_ms(20);

                    /* Deactivate Current Column */
                    DIO_u8SetPinValue(
                        KPD_COL_PORT,
                        Local_u8ColArr[Local_u8ColIndex],
                        DIO_PIN_HIGH
                    );

                    return Local_au8KeyMap[Local_u8RowIndex][Local_u8ColIndex];
                }
            }
        }

        /* Deactivate Current Column */
        DIO_u8SetPinValue(
            KPD_COL_PORT,
            Local_u8ColArr[Local_u8ColIndex],
            DIO_PIN_HIGH
        );
    }

    return KPD_NO_PRESSED_KEY;
}
