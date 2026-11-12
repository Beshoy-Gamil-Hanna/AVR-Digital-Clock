/*
 * DS1307_private.h
 *
 *  Created on: Oct 9, 2026
 *      Author: Beshoy
 */

#ifndef HAL_DS1307_DS1307_PRIVATE_H_
#define HAL_DS1307_DS1307_PRIVATE_H_

/* DS1307 I2C Address */
#define DS1307_ADDRESS             0x68

/* DS1307 Register Addresses */
#define DS1307_SECONDS_REG         0x00
#define DS1307_MINUTES_REG         0x01
#define DS1307_HOURS_REG           0x02
#define DS1307_DAY_REG             0x03
#define DS1307_DATE_REG            0x04
#define DS1307_MONTH_REG           0x05
#define DS1307_YEAR_REG            0x06
#define DS1307_CONTROL_REG         0x07

/* Seconds Register Bits */
#define DS1307_SECONDS_CH          7

/* Hours Register Bits */
#define DS1307_HOURS_12_24          6
#define DS1307_HOURS_PM             5

/* Control Register Bits */
#define DS1307_CONTROL_OUT          7
#define DS1307_CONTROL_SQWE         4
#define DS1307_CONTROL_RS1          1
#define DS1307_CONTROL_RS0          0

#endif /* HAL_DS1307_DS1307_PRIVATE_H_ */
