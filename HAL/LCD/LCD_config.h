/*
 * LCD_config.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef HAL_LCD_CONFIG_H_
#define HAL_LCD_CONFIG_H_

/* LCD Data Bus: PA0 -> PA7 */
#define LCD_DATA_PORT   DIO_PORTA

/* LCD Control Pins */
#define LCD_CTR_PORT    DIO_PORTB
#define LCD_RS          DIO_PIN0
#define LCD_RW          DIO_PIN1
#define LCD_E           DIO_PIN2

#endif /* HAL_LCD_CONFIG_H_ */
