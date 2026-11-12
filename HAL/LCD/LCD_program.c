/*
 * LCD_program.c
 *
 * Created on: Aug 6, 2026
 * Author: Beshoy
 */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stddef.h>
#include <util/delay.h>

#include "../../LIB/std_types.h"
#include "../../MCAL/DIO/DIO_interface.h"

#include "LCD_config.h"
#include "LCD_interface.h"


/* ================= Private Functions ================= */

static void LCD_vidPulseEnable(void)
{
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_E, DIO_PIN_HIGH);
    _delay_us(1);
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_E, DIO_PIN_LOW);
    _delay_us(50);
}


/* ================= Public Functions ================= */

/*
 * Send Command To LCD.
 */
void LCD_vidSendCommand(u8 copy_u8Command)
{
    /* RS = 0: Command */
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RS, DIO_PIN_LOW);

    /* RW = 0: Write */
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RW, DIO_PIN_LOW);

    /* Put Command On Data Bus Before Enable Pulse */
    DIO_u8SetPortValue(LCD_DATA_PORT, copy_u8Command);

    LCD_vidPulseEnable();

    /* Clear Display And Return Home Need More Time */
    if((copy_u8Command == LCD_CLEAR_DISPLAY) || (copy_u8Command == 0x02))
    {
        _delay_ms(2);
    }
}


/*
 * Send Data Character To LCD.
 */
void LCD_vidSendData(u8 copy_u8Data)
{
    /* RS = 1: Data */
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RS, DIO_PIN_HIGH);

    /* RW = 0: Write */
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RW, DIO_PIN_LOW);

    /* Put Data On Data Bus Before Enable Pulse */
    DIO_u8SetPortValue(LCD_DATA_PORT, copy_u8Data);

    LCD_vidPulseEnable();
}


/*
 * Initialize LCD In 8-bit, 2-line, 5x7-dot Mode.
 */
void LCD_vidInit(void)
{
    /* Configure LCD Data Bus */
    DIO_u8SetPortValue(LCD_DATA_PORT, DIO_PORT_LOW);
    DIO_u8SetPortDirection(LCD_DATA_PORT, DIO_PORT_OUTPUT);

    /* Configure LCD Control Pins Without Changing The Buzzer Pin */
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RS, DIO_PIN_LOW);
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_RW, DIO_PIN_LOW);
    DIO_u8SetPinValue(LCD_CTR_PORT, LCD_E, DIO_PIN_LOW);

    DIO_u8SetPinDirection(LCD_CTR_PORT, LCD_RS, DIO_PIN_OUTPUT);
    DIO_u8SetPinDirection(LCD_CTR_PORT, LCD_RW, DIO_PIN_OUTPUT);
    DIO_u8SetPinDirection(LCD_CTR_PORT, LCD_E, DIO_PIN_OUTPUT);

    /* Wait For LCD Power-up */
    _delay_ms(35);

    /* Function Set: 8-bit, 2-line, 5x7 dots */
    LCD_vidSendCommand(
        LCD_FUNCTION_SET |
        LCD_8_BIT_MODE |
        LCD_2_LINES |
        LCD_5x7_DOTS
    );

    /* Display ON, Cursor OFF, Blink OFF */
    LCD_vidSendCommand(
        LCD_DISPLAY_CONTROL |
        LCD_DISPLAY_ON |
        LCD_CURSOR_OFF |
        LCD_BLINK_OFF
    );

    /* Clear Display */
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    /* Entry Mode: Increment Cursor, No Display Shift */
    LCD_vidSendCommand(
        LCD_ENTRY_MODE |
        LCD_ENTRY_INCREMENT |
        LCD_ENTRY_NO_SHIFT
    );
}


/*
 * Write Null-Terminated String.
 */
void LCD_vidWriteString(u8 *copy_pu8String)
{
    u8 Local_u8Index = 0;

    if(copy_pu8String == NULL)
    {
        return;
    }

    while(copy_pu8String[Local_u8Index] != '\0')
    {
        LCD_vidSendData(copy_pu8String[Local_u8Index]);
        Local_u8Index++;
    }
}


/*
 * Set LCD Cursor Position.
 * Rows: 0 -> 1
 * Columns: 0 -> 15
 */
void LCD_SetCursorPosition(u8 copy_u8row, u8 copy_u8col)
{
    if((copy_u8row > 1) || (copy_u8col >= LCD_WIDTH))
    {
        return;
    }

    if(copy_u8row == 0)
    {
        LCD_vidSendCommand(LCD_ROW0_START_ADDRESS + copy_u8col);
    }
    else
    {
        LCD_vidSendCommand(LCD_ROW1_START_ADDRESS + copy_u8col);
    }
}


/*
 * Write String Alternating Between The Two Rows.
 * Kept For Compatibility With The Existing Interface.
 */
void LCD_vidWriteNameSinusoidal(u8 *copy_pu8String, u8 row, u8 col)
{
    u8 Local_u8Index = 0;

    if((copy_pu8String == NULL) || (row > 1) || (col >= LCD_WIDTH))
    {
        return;
    }

    while(copy_pu8String[Local_u8Index] != '\0')
    {
        LCD_SetCursorPosition(row, col);
        LCD_vidSendData(copy_pu8String[Local_u8Index]);

        row = (u8)!row;
        col++;

        if(col >= LCD_WIDTH)
        {
            col = 0;
        }

        Local_u8Index++;
    }
}


/*
 * Move String In A Sinusoidal Pattern.
 * This Function Is Blocking And Is Not Used By The Clock Application.
 */
void LCD_vidMoveNameSinusoidal(u8 *copy_pu8String, u8 row, u8 col)
{
    if(copy_pu8String == NULL)
    {
        return;
    }

    while(LCD_MOVE_TRUE)
    {
        LCD_vidSendCommand(LCD_CLEAR_DISPLAY);
        LCD_vidWriteNameSinusoidal(copy_pu8String, row, col);

        col++;
        if(col >= LCD_WIDTH)
        {
            col = 0;
        }

        row = (u8)!row;
        _delay_ms(600);
    }
}
