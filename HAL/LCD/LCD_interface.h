/*
 * LCD_interface.h
 *
 *  Created on: Aug 6, 2026
 *      Author: Beshoy
 */

#ifndef HAL_LCD_INTERFACE_H_
#define HAL_LCD_INTERFACE_H_

#include "../../LIB/std_types.h"

/* Clear Display */
#define LCD_CLEAR_DISPLAY            0x01

#define LCD_ROW0_START_ADDRESS       0x80
#define LCD_ROW1_START_ADDRESS       0xC0

/* Entry Mode Set */
#define LCD_ENTRY_MODE               0x04
#define LCD_ENTRY_INCREMENT          0x02
#define LCD_ENTRY_DECREMENT          0x00
#define LCD_ENTRY_SHIFT              0x01
#define LCD_ENTRY_NO_SHIFT           0x00


/* Display ON/OFF Control */
#define LCD_DISPLAY_CONTROL          0x08
#define LCD_DISPLAY_ON               0x04
#define LCD_DISPLAY_OFF              0x00
#define LCD_CURSOR_ON                0x02
#define LCD_CURSOR_OFF               0x00
#define LCD_BLINK_ON                 0x01
#define LCD_BLINK_OFF                0x00


/* Cursor / Display Shift */
#define LCD_CURSOR_DISPLAY_SHIFT     0x10
#define LCD_SHIFT_DISPLAY            0x08
#define LCD_SHIFT_CURSOR             0x00
#define LCD_SHIFT_RIGHT              0x04
#define LCD_SHIFT_LEFT               0x00


/* Function Set */
#define LCD_FUNCTION_SET             0x20
#define LCD_8_BIT_MODE               0x10
#define LCD_4_BIT_MODE               0x00
#define LCD_2_LINES                  0x08
#define LCD_1_LINE                   0x00
#define LCD_5x10_DOTS                0x04
#define LCD_5x7_DOTS                 0x00


/* Set CGRAM Address */
#define LCD_SET_CGRAM_ADDRESS        0x40
/* Set DDRAM Address */
#define LCD_SET_DDRAM_ADDRESS        0x80


#define LCD_WIDTH    16

#define LCD_MOVE_TRUE    1
#define LCD_MOVE_FALSE    0

#define LCD_FIRST_ROW       0
#define LCD_FIRST_COLUMN    0

void LCD_vidSendCommand(u8 copy_u8Command);
void LCD_vidSendData(u8 copy_u8Data);
void LCD_vidInit(void);
void LCD_vidWriteString(u8 * copy_pu8String);
void LCD_SetCursorPosition(u8 copy_u8row , u8 copy_u8col);
void LCD_vidWriteNameSinusoidal(u8 *copy_pu8String, u8 row, u8 col);
void LCD_vidMoveNameSinusoidal(u8 *copy_pu8String, u8 row, u8 col);

#endif /* HAL_LCD_INTERFACE_H_ */
