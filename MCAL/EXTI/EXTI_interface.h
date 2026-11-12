/*
 * EXTI_interface.h
 *
 *  Created on: Aug 23, 2026
 *      Author: Beshoy
 */

#ifndef MCAL_EXTI_EXTI_INTERFACE_H_
#define MCAL_EXTI_EXTI_INTERFACE_H_

#define EXTI_INT0_PORT    DIO_PORTD
#define EXTI_INT0_PIN     DIO_PIN2

#define EXTI_INT1_PORT    DIO_PORTD
#define EXTI_INT1_PIN     DIO_PIN3

#define EXTI_INT2_PORT    DIO_PORTB
#define EXTI_INT2_PIN     DIO_PIN2

typedef enum
{
    EXTI_LOW_LEVEL,
    EXTI_ANY_CHANGE,
    EXTI_FALLING_EDGE,
    EXTI_RISING_EDGE
} EXTI_Sense_t;

typedef enum
{
    EXTI_INT0,
    EXTI_INT1,
    EXTI_INT2
} EXTI_Int_t;

void EXTI_vidInt0Init(EXTI_Sense_t Copy_u8Sense);
void EXTI_vidInt1Init(EXTI_Sense_t Copy_u8Sense);
void EXTI_vidInt2Init(EXTI_Sense_t Copy_u8Sense);
void EXTI_vidSetCallback(EXTI_Int_t Copy_u8Int,void (*Copy_pvCallback)(void));
void EXTI_vidDisable(EXTI_Int_t Copy_u8Int);

#endif /* MCAL_EXTI_EXTI_INTERFACE_H_ */
