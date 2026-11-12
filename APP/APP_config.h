/*
 * APP_config.h
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#ifndef APP_APP_CONFIG_H_
#define APP_APP_CONFIG_H_

/*
 * Timer1 Configuration
 *
 * CPU Frequency   = 8 MHz
 * Timer1 Prescaler = 8
 * Timer1 Frequency = 1 MHz
 * Timer1 Tick Time = 1 us
 *
 * Compare Match Period = 10 ms
 *
 * OCR1A = (10000 / 1) - 1 = 9999
 */

/* Timer1 Compare Match Value */

#define APP_TIMER1_COMPARE_MATCH_VALUE       9999U

/* Number of Compare Matches Per Second */

#define APP_TIMER1_MATCHES_PER_SECOND        100U

#endif /* APP_APP_CONFIG_H_ */
