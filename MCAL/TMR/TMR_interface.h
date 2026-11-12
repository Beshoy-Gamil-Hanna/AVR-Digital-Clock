/*
 * TMR_interface.h
 *
 * Created on: Aug 29, 2026
 * Author: Beshoy
 */

#ifndef MCAL_TMR_TMR_INTERFACE_H_
#define MCAL_TMR_TMR_INTERFACE_H_

#include "../../LIB/std_types.h"
#include "../DIO/DIO_interface.h"
#include "TMR_config.h"

/* ================= Timer Interrupts ================= */
typedef enum
{
    TMR0_INT_OVF,
    TMR0_INT_OC,

    TMR1_INT_OVF,
    TMR1_INT_OCA,
    TMR1_INT_OCB,
    TMR1_INT_ICU,

    TMR2_INT_OVF,
    TMR2_INT_OC

} TMR_Int_t;


/* ================= Timer1 Channels ================= */
#define TMR1_CHANNEL_A        0
#define TMR1_CHANNEL_B        1


/* ================= Timer1 Pins ================= */
#define TMR1_OC1A_PORT        DIO_PORTD
#define TMR1_OC1A_PIN         DIO_PIN5

#define TMR1_OC1B_PORT        DIO_PORTD
#define TMR1_OC1B_PIN         DIO_PIN4

#define TMR1_ICP1_PORT        DIO_PORTD
#define TMR1_ICP1_PIN         DIO_PIN6


/* ================= Timer0 Pins ================= */
#define TMR0_OC0_PORT         DIO_PORTB
#define TMR0_OC0_PIN          DIO_PIN3


/* ================= PWM Output Mode ================= */
#define TMR_PWM_NON_INVERTING    1
#define TMR_PWM_INVERTING        2


/* ================= General Functions ================= */
void TMR_vidSetCallback(
    TMR_Int_t Copy_u8Int,
    void (*Copy_pvCallback)(void)
);

u8 TMR_u8GetCompareMatchValue(
    u32 Copy_u32TimeMs,
    f32 Copy_f32TickTime
);


/* ================= Timer0 Functions ================= */
void TMR0_vidInit(void);
void TMR0_vidIntEnable(TMR_Int_t Copy_u8Int);
void TMR0_vidIntDisable(TMR_Int_t Copy_u8Int);
void TMR0_vidSetPreload(u8 Copy_u8Preload);
void TMR0_vidSetCompareMatchValue(u8 Copy_u8CompareMatchValue);
void TMR0_vidPWMMode(u8 Copy_u8OutputMode);
void TMR0_vidSetDutyCycle(u8 Copy_u8DutyCycle);

void TMR0_vidStartTimer(
    u32 Copy_u32TimeMS,
    void (*Copy_pvCallback)(void),
    u8 Copy_u8Mode
);

void TMR0_vidGenerateOC0Frequency(u32 Copy_u32Frequency);
void TMR0_vidStop(void);


/* ================= Timer1 Functions ================= */
void TMR1_vidInit(void);
void TMR1_vidIntEnable(TMR_Int_t Copy_u8Int);
void TMR1_vidIntDisable(TMR_Int_t Copy_u8Int);
void TMR1_vidSetTopValue(u16 Copy_u16TopValue);

void TMR1_vidSetCompareMatchValue(
    u8 Copy_u8Channel,
    u16 Copy_u16CompareMatchValue
);

void TMR1_vidPWMMode(
    u8 Copy_u8Channel,
    u8 Copy_u8OutputMode
);

#endif /* MCAL_TMR_TMR_INTERFACE_H_ */
