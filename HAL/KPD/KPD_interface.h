/*
 * KPD_interface.h
 *
 * Created on: Aug 10, 2026
 * Author: Beshoy
 */

#ifndef HAL_KPD_KPD_INTERFACE_H_
#define HAL_KPD_KPD_INTERFACE_H_

#include "../../LIB/std_types.h"

/* Returned When No Key Is Pressed */
#define KPD_NO_PRESSED_KEY    255U

/* Standard 4x4 Keypad Characters */
#define KPD_KEY_A       ((u8)'A')
#define KPD_KEY_B       ((u8)'B')
#define KPD_KEY_C       ((u8)'C')
#define KPD_KEY_D       ((u8)'D')
#define KPD_KEY_STAR    ((u8)'*')
#define KPD_KEY_HASH    ((u8)'#')

/* Function Prototypes */
void KPD_vidInit(void);
u8 KPD_u8GetPressedKey(void);

#endif /* HAL_KPD_KPD_INTERFACE_H_ */
