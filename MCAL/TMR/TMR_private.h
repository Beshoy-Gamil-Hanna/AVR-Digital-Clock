/*
 * TMR_private.h
 *
 *  Created on: Aug 29, 2026
 *      Author: Beshoy
 */

#ifndef MCAL_TMR_TMR_PRIVATE_H_
#define MCAL_TMR_TMR_PRIVATE_H_

/* ================= TIMER0 ================= */

/* TCCR0 */
#define TMR_TCCR0       *((volatile u8*)0x53)

#define TMR_TCCR0_FOC0      7
#define TMR_TCCR0_WGM00     6
#define TMR_TCCR0_COM01     5
#define TMR_TCCR0_COM00     4
#define TMR_TCCR0_WGM01     3
#define TMR_TCCR0_CS02      2
#define TMR_TCCR0_CS01      1
#define TMR_TCCR0_CS00      0


/* TCNT0 */
#define TMR_TCNT0       *((volatile u8*)0x52)


/* OCR0 */
#define TMR_OCR0        *((volatile u8*)0x5C)


/* TIMSK */
#define TMR_TIMSK       *((volatile u8*)0x59)

#define TMR_TIMSK_OCIE2     7
#define TMR_TIMSK_TOIE2     6
#define TMR_TIMSK_TICIE1    5
#define TMR_TIMSK_OCIE1A    4
#define TMR_TIMSK_OCIE1B    3
#define TMR_TIMSK_TOIE1     2
#define TMR_TIMSK_OCIE0     1
#define TMR_TIMSK_TOIE0     0


/* TIFR */
#define TMR_TIFR        *((volatile u8*)0x58)

#define TMR_TIFR_OCF2      7
#define TMR_TIFR_TOV2      6
#define TMR_TIFR_ICF1      5
#define TMR_TIFR_OCF1A     4
#define TMR_TIFR_OCF1B     3
#define TMR_TIFR_TOV1      2
#define TMR_TIFR_OCF0      1
#define TMR_TIFR_TOV0      0

/* ================= TIMER1 ================= */

/* TCCR1A */
#define TMR_TCCR1A      *((volatile u8*)0x4F)

#define TMR_TCCR1A_COM1A1    7
#define TMR_TCCR1A_COM1A0    6
#define TMR_TCCR1A_COM1B1    5
#define TMR_TCCR1A_COM1B0    4
#define TMR_TCCR1A_FOC1A     3
#define TMR_TCCR1A_FOC1B     2
#define TMR_TCCR1A_WGM11     1
#define TMR_TCCR1A_WGM10     0


/* TCCR1B */
#define TMR_TCCR1B      *((volatile u8*)0x4E)

#define TMR_TCCR1B_ICNC1     7
#define TMR_TCCR1B_ICES1     6
#define TMR_TCCR1B_WGM13     4
#define TMR_TCCR1B_WGM12     3
#define TMR_TCCR1B_CS12      2
#define TMR_TCCR1B_CS11      1
#define TMR_TCCR1B_CS10      0


/* 16-bit Registers */

/* TCNT1 = TCNT1H + TCNT1L */
#define TMR_TCNT1       *((volatile u16*)0x4C)

/* OCR1A = OCR1AH + OCR1AL */
#define TMR_OCR1A       *((volatile u16*)0x4A)

/* OCR1B = OCR1BH + OCR1BL */
#define TMR_OCR1B       *((volatile u16*)0x48)

/* ICR1 = ICR1H + ICR1L */
#define TMR_ICR1        *((volatile u16*)0x46)


/* ================= TIMER2 ================= */

/* TCCR2 */
#define TMR_TCCR2       *((volatile u8*)0x45)

#define TMR_TCCR2_FOC2      7
#define TMR_TCCR2_WGM20     6
#define TMR_TCCR2_COM21     5
#define TMR_TCCR2_COM20     4
#define TMR_TCCR2_WGM21     3
#define TMR_TCCR2_CS22      2
#define TMR_TCCR2_CS21      1
#define TMR_TCCR2_CS20      0


/* TCNT2 */
#define TMR_TCNT2       *((volatile u8*)0x44)


/* OCR2 */
#define TMR_OCR2        *((volatile u8*)0x43)

/* Timer Prescaler */
#define TMR_NO_CLOCK_SOURCE       0b00000000
#define TMR_NO_PRESCALER          0b00000001
#define TMR_PRESCALER_8           0b00000010
#define TMR_PRESCALER_64          0b00000011
#define TMR_PRESCALER_256         0b00000100
#define TMR_PRESCALER_1024        0b00000101
#define TMR_EXTERNAL_FALLING      0b00000110
#define TMR_EXTERNAL_RISING       0b00000111

#define TMR_PRESCALER_MASK        0b11111000

/*Select Timer0 mode*/
#define TMR0_NORMAL_MODE             0b00000000
#define TMR0_PHASE_CORRECT_PWM_MODE  0b01000000
#define TMR0_CTC_MODE                0b00001000
#define TMR0_FAST_PWM_MODE           0b01001000

#define TMR0_MODE_MASK               0b10110111

/* ================= TIMER1 MODE ================= */

#define TMR1_NORMAL_MODE                              0
#define TMR1_PWM_PHASE_CORRECT_8_BIT_MODE             1
#define TMR1_PWM_PHASE_CORRECT_9_BIT_MODE             2
#define TMR1_PWM_PHASE_CORRECT_10_BIT_MODE            3
#define TMR1_CTC_OCR1A_MODE                           4
#define TMR1_FAST_PWM_8_BIT_MODE                      5
#define TMR1_FAST_PWM_9_BIT_MODE                      6
#define TMR1_FAST_PWM_10_BIT_MODE                     7
#define TMR1_PWM_PHASE_FREQUENCY_CORRECT_ICR1_MODE    8
#define TMR1_PWM_PHASE_FREQUENCY_CORRECT_OCR1A_MODE   9
#define TMR1_PWM_PHASE_CORRECT_ICR1_MODE              10
#define TMR1_PWM_PHASE_CORRECT_OCR1A_MODE             11
#define TMR1_CTC_ICR1_MODE                            12
#define TMR1_RESERVED_MODE                            13
#define TMR1_FAST_PWM_ICR1_MODE                       14
#define TMR1_FAST_PWM_OCR1A_MODE                      15

/* ================= TIMER1 MODE MASK ================= */

#define TMR1_MODE_MASK_A    0xFC
#define TMR1_MODE_MASK_B    0xE7



#endif /* MCAL_TMR_TMR_PRIVATE_H_ */
