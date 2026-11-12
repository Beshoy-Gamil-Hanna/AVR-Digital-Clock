/*
 * TMR_program.c
 *
 *  Created on: Aug 29, 2026
 *      Author: Beshoy
 */

#include "../../LIB/bit_math.h"

#include "../../MCAL/GIE/GIE_interface.h"

#include "TMR_private.h"
#include "TMR_interface.h"
#include "TMR_config.h"

#define TMR0_PRESCALER_VALUE()                         \
(                                                     \
    (TMR0_PRESCALER == TMR_NO_PRESCALER)    ? 1UL :   \
    (TMR0_PRESCALER == TMR_PRESCALER_8)    ? 8UL :   \
    (TMR0_PRESCALER == TMR_PRESCALER_64)   ? 64UL :  \
    (TMR0_PRESCALER == TMR_PRESCALER_256)  ? 256UL : \
    (TMR0_PRESCALER == TMR_PRESCALER_1024) ? 1024UL :\
    1UL                                             \
)

const u32 SYSTEM_CLOCK = 8000000;
#define TOP_VALUE 256
static u32 H_copy_u32TimeMs ;
static void (*H_copy_pvCallback)(void)  ;
static u8 H_copy_u8Mode ;

static void (*TMR0_pvOVFCallback)(void) = NULL;
static void (*TMR0_pvOCCallback)(void)  = NULL;

static void (*TMR1_pvOVFCallback)(void) = NULL;
static void (*TMR1_pvOCACallback)(void) = NULL;
static void (*TMR1_pvOCBCallback)(void) = NULL;
static void (*TMR1_pvICUCallback)(void) = NULL;

static void (*TMR2_pvOVFCallback)(void) = NULL;
static void (*TMR2_pvOCCallback)(void)  = NULL;

/* ================= Timer0 ================= */
void TMR0_vidInit(void)
{
	/*Select timer0 mode*/
	TMR_TCCR0 &= TMR0_MODE_MASK;
	TMR_TCCR0 |= TMR0_MODE;

	/*Select prescaller*/
	TMR_TCCR0 &= TMR_PRESCALER_MASK;
	TMR_TCCR0 |= TMR0_PRESCALER;


}
void TMR0_vidIntEnable(TMR_Int_t Copy_u8Int)
{
	switch(Copy_u8Int)
	{
		case TMR0_INT_OVF:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_TOIE0);
			break;

		case TMR0_INT_OC:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_OCIE0);
			break;

        default:
			break;
	}
}
void TMR0_vidIntDisable(TMR_Int_t Copy_u8Int)
{
	switch(Copy_u8Int)
	{
		case TMR0_INT_OVF:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_TOIE0);
			break;

		case TMR0_INT_OC:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_OCIE0);
			break;

        default:
			break;
	}
}

void TMR0_vidPWMMode(u8 copy_u8OutputMode)
{
    /* Configure Output Mode */
    if (copy_u8OutputMode == TMR_PWM_NON_INVERTING)
    {
        SET_BIT(TMR_TCCR0, TMR_TCCR0_COM01);
        CLR_BIT(TMR_TCCR0, TMR_TCCR0_COM00);
    }
    else if (copy_u8OutputMode == TMR_PWM_INVERTING)
    {
        SET_BIT(TMR_TCCR0, TMR_TCCR0_COM01);
        SET_BIT(TMR_TCCR0, TMR_TCCR0_COM00);
    }
}

void TMR0_vidSetDutyCycle(u8 copy_u8DutyCycle)
{
    TMR_OCR0 = ((u16)copy_u8DutyCycle * 255) / 100;
}

void TMR0_vidSetCompareMatchValue(u8 copy_u8CompareMatchValue)
{
	TMR_OCR0 = copy_u8CompareMatchValue;
}

void TMR0_vidSetPreload(u8 copy_u8Preload)
{
	TMR_TCNT0 = copy_u8Preload;
}

static void TMR0_vidStartTimerHelper(void)
{
	TMR0_vidStartTimer(H_copy_u32TimeMs , H_copy_pvCallback ,H_copy_u8Mode);
}

u8 TMR_u8GetCompareMatchValue(u32 copy_u32TimeMs, f32 tickTime)
{
	f32 numberOfTicks =(copy_u32TimeMs / 1000.0) / tickTime;

	u8 most_accurate = 50;
	f32 minimum_error = numberOfTicks;

	for(u16 i = 256; i > 50; i--)
	{
		f32 numberOfInterrupts = numberOfTicks / i;

		u32 roundedInterrupts = (u32)(numberOfInterrupts + 0.5);

		if(roundedInterrupts == 0)
		{
			roundedInterrupts = 1;
		}

		f32 actualTicks = roundedInterrupts * i;

		f32 error = numberOfTicks - actualTicks;

		if(error < 0)
		{
			error = -error;
		}

		if(error < minimum_error)
		{
			minimum_error = error;
			most_accurate = i - 1;
		}

		if(error == 0)
		{
			return i - 1;
		}
	}

	return most_accurate;
}
void TMR0_vidStartTimer(u32 copy_u32TimeMs ,void (*copy_pvCallback)(void), u8 copy_u8Mode)
{
	static u32 counter = 0;
	static u32 NumberoFovf = 0;
	static u32 NumberoFComp = 0;
	static u8 running = 0;
	if(running == 0)
	{
		running = 1;
		counter = 0;
		H_copy_u32TimeMs = copy_u32TimeMs;
		H_copy_pvCallback = copy_pvCallback;
		H_copy_u8Mode = copy_u8Mode;
		TMR0_vidInit();
		f32 timerClock = SYSTEM_CLOCK / (f32)(TMR0_PRESCALER_VALUE());
		f32 tickTime =1.0 / timerClock;
		switch(copy_u8Mode)
		{
			case TMR0_NORMAL_MODE:
			{
				/*Select timer0 mode*/
				TMR_TCCR0 &= TMR0_MODE_MASK;
				TMR_TCCR0 |= TMR0_NORMAL_MODE;

				TMR_vidSetCallback(TMR0_INT_OVF,TMR0_vidStartTimerHelper);
				TMR0_vidIntEnable(TMR0_INT_OVF);
				f32 numberOfTicks =(copy_u32TimeMs / 1000.0)/ tickTime;
				NumberoFovf =(u32)(numberOfTicks / TOP_VALUE);
				if((numberOfTicks -(NumberoFovf * TOP_VALUE)) > 0)
				{
					NumberoFovf++;
				}
				if(NumberoFovf == 0)
				{
					NumberoFovf = 1;
				}
				u32 totalTicks =(u32)(numberOfTicks + 0.5);
				u32 preload =(NumberoFovf * TOP_VALUE)- totalTicks;
				TMR0_vidSetPreload((u8)preload);
				GIE_vidEnable();
				break;
			}
			case TMR0_CTC_MODE:
			{

				/*Select timer0 mode*/
				TMR_TCCR0 &= TMR0_MODE_MASK;
				TMR_TCCR0 |= TMR0_CTC_MODE;

				TMR_vidSetCallback(TMR0_INT_OC,TMR0_vidStartTimerHelper);
				TMR0_vidIntEnable(TMR0_INT_OC);
				u8 CompareMatchValue =TMR_u8GetCompareMatchValue(copy_u32TimeMs,tickTime);
				TMR0_vidSetCompareMatchValue(CompareMatchValue);
				f32 timeOfComp =tickTime *(CompareMatchValue + 1);
				f32 numberOfComp =(copy_u32TimeMs / 1000.0) / timeOfComp;
				NumberoFComp = (u32)(numberOfComp + 0.5);
				if(NumberoFComp == 0)
				{
					NumberoFComp = 1;
				}
				GIE_vidEnable();
				break;
			}
			default:
			{
				running = 0;
				break;
			}
		}
		return;
	}
	counter++;
	if(copy_u8Mode == TMR0_NORMAL_MODE)
	{
		if(counter >= NumberoFovf)
		{
			counter = 0;
			running = 0;
			if(H_copy_pvCallback != NULL)
			{
				H_copy_pvCallback();
			}
		}
	}
	else if(copy_u8Mode == TMR0_CTC_MODE)
	{
		if(counter >= NumberoFComp)
		{
			counter = 0;
			running = 0;
			if(H_copy_pvCallback != NULL)
			{
				H_copy_pvCallback();
			}
		}
	}
}

void TMR0_vidGenerateOC0Frequency(u32 Copy_u32Frequency)
{
	if(Copy_u32Frequency == 0)
		return;

	u32 Temp_TMR_OCR0 =
		(F_CPU / (2UL * TMR0_PRESCALER_VALUE() * Copy_u32Frequency)) - 1UL;

	/* Set Timer0 to CTC Mode */
	TMR_TCCR0 &= TMR0_MODE_MASK;
	TMR_TCCR0 |= TMR0_CTC_MODE;

	/* Toggle OC0 on Compare Match */
	SET_BIT(TMR_TCCR0, TMR_TCCR0_COM00);
	CLR_BIT(TMR_TCCR0, TMR_TCCR0_COM01);

	/* Set Compare Value */
	TMR_OCR0 = (u8)Temp_TMR_OCR0;
}

void TMR0_vidStop(void)
{
	TMR_TCCR0 &= TMR_PRESCALER_MASK;
}
/* ================= Timer1 ================= */
void TMR1_vidInit(void)
{
	/* Select Timer1 mode */

	TMR_TCCR1A &= TMR1_MODE_MASK_A;
	TMR_TCCR1B &= TMR1_MODE_MASK_B;

	TMR_TCCR1A |= (TMR1_MODE & 0x03);
	TMR_TCCR1B |= ((TMR1_MODE >> 2) << 3);

	/* Select Timer1 Prescaler */
	TMR_TCCR1B &= TMR_PRESCALER_MASK;
	TMR_TCCR1B |= TMR1_PRESCALER;
}
void TMR1_vidPWMMode(u8 copy_u8Channel, u8 copy_u8OutputMode)
{
	if(copy_u8Channel == TMR1_CHANNEL_A)
	{
		if(copy_u8OutputMode == TMR_PWM_NON_INVERTING)
		{
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1A1);
			CLR_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1A0);
		}
		else if(copy_u8OutputMode == TMR_PWM_INVERTING)
		{
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1A1);
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1A0);
		}
	}
	else if(copy_u8Channel == TMR1_CHANNEL_B)
	{
		if(copy_u8OutputMode == TMR_PWM_NON_INVERTING)
		{
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1B1);
			CLR_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1B0);
		}
		else if(copy_u8OutputMode == TMR_PWM_INVERTING)
		{
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1B1);
			SET_BIT(TMR_TCCR1A, TMR_TCCR1A_COM1B0);
		}
	}
}
void TMR1_vidIntEnable(TMR_Int_t Copy_u8Int)
{
	switch(Copy_u8Int)
	{
		case TMR1_INT_OVF:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_TOIE1);
			break;

		case TMR1_INT_OCA:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_OCIE1A);
			break;

		case TMR1_INT_OCB:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_OCIE1B);
			break;

		case TMR1_INT_ICU:
			SET_BIT(TMR_TIMSK, TMR_TIMSK_TICIE1);
			break;

		default:
			break;
	}
}

void TMR1_vidIntDisable(TMR_Int_t Copy_u8Int)
{
	switch(Copy_u8Int)
	{
		case TMR1_INT_OVF:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_TOIE1);
			break;

		case TMR1_INT_OCA:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_OCIE1A);
			break;

		case TMR1_INT_OCB:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_OCIE1B);
			break;

		case TMR1_INT_ICU:
			CLR_BIT(TMR_TIMSK, TMR_TIMSK_TICIE1);
			break;

		default:
			break;
	}
}

void TMR1_vidSetTopValue(u16 copy_u16TopValue)
{
	TMR_ICR1 = copy_u16TopValue;
}

void TMR1_vidSetCompareMatchValue(u8 copy_u8Channel, u16 copy_u16CompareMatchValue)
{
	if(copy_u8Channel == TMR1_CHANNEL_A)
	{
		TMR_OCR1A = copy_u16CompareMatchValue;
	}
	else if(copy_u8Channel == TMR1_CHANNEL_B)
	{
		TMR_OCR1B = copy_u16CompareMatchValue;
	}
}

void TMR_vidSetCallback(TMR_Int_t Copy_u8Int, void (*Copy_pvCallback)(void))
{
	if(Copy_pvCallback == NULL)
		return;
	switch(Copy_u8Int)
	{
		case TMR0_INT_OVF:
			TMR0_pvOVFCallback = Copy_pvCallback;
			break;

		case TMR0_INT_OC:
			TMR0_pvOCCallback = Copy_pvCallback;
			break;


		case TMR1_INT_OVF:
			TMR1_pvOVFCallback = Copy_pvCallback;
			break;

		case TMR1_INT_OCA:
			TMR1_pvOCACallback = Copy_pvCallback;
			break;

		case TMR1_INT_OCB:
			TMR1_pvOCBCallback = Copy_pvCallback;
			break;

		case TMR1_INT_ICU:
			TMR1_pvICUCallback = Copy_pvCallback;
			break;


		case TMR2_INT_OVF:
			TMR2_pvOVFCallback = Copy_pvCallback;
			break;

		case TMR2_INT_OC:
			TMR2_pvOCCallback = Copy_pvCallback;
			break;


		default:
			return;
	}
}

/* ================= Timer0 ================= */

/* Timer0 Overflow */
void __vector_11(void) __attribute__((signal));

void __vector_11(void)
{
	if(TMR0_pvOVFCallback != NULL)
	{
		TMR0_pvOVFCallback();
	}
}


/* Timer0 Compare Match */
void __vector_10(void) __attribute__((signal));

void __vector_10(void)
{
	if(TMR0_pvOCCallback != NULL)
	{
		TMR0_pvOCCallback();
	}
}
/* ================= Timer1 ================= */

/* Timer1 Input Capture */
void __vector_6(void) __attribute__((signal));

void __vector_6(void)
{
	if(TMR1_pvICUCallback != NULL)
	{
		TMR1_pvICUCallback();
	}
}


/* Timer1 Compare Match A */
void __vector_7(void) __attribute__((signal));

void __vector_7(void)
{
	if(TMR1_pvOCACallback != NULL)
	{
		TMR1_pvOCACallback();
	}
}


/* Timer1 Compare Match B */
void __vector_8(void) __attribute__((signal));

void __vector_8(void)
{
	if(TMR1_pvOCBCallback != NULL)
	{
		TMR1_pvOCBCallback();
	}
}


/* Timer1 Overflow */
void __vector_9(void) __attribute__((signal));

void __vector_9(void)
{
	if(TMR1_pvOVFCallback != NULL)
	{
		TMR1_pvOVFCallback();
	}
}
/* ================= Timer2 ================= */

/* Timer2 Compare Match */
void __vector_4(void) __attribute__((signal));

void __vector_4(void)
{
	if(TMR2_pvOCCallback != NULL)
	{
		TMR2_pvOCCallback();
	}
}


/* Timer2 Overflow */
void __vector_5(void) __attribute__((signal));

void __vector_5(void)
{
	if(TMR2_pvOVFCallback != NULL)
	{
		TMR2_pvOVFCallback();
	}
}
