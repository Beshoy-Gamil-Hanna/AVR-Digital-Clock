/*
 * EXTI_program.c
 *
 *  Created on: Aug 23, 2026
 *      Author: Beshoy
 */

#include "../../LIB/bit_math.h"
#include "../../LIB/std_types.h"
#include "EXTI_interface.h"
#include "EXTI_private.h"


static void (*EXTI_pvInt0Callback)(void) = NULL;
static void (*EXTI_pvInt1Callback)(void) = NULL;
static void (*EXTI_pvInt2Callback)(void) = NULL;

void EXTI_vidInt0Init(EXTI_Sense_t Copy_u8Sense)
{

	/*Configure sense control bits */
	switch(Copy_u8Sense)
	{
		case EXTI_LOW_LEVEL:
			CLR_BIT(MCUCR, MCUCR_ISC01);
			CLR_BIT(MCUCR, MCUCR_ISC00);
			break;

		case EXTI_ANY_CHANGE:
			CLR_BIT(MCUCR, MCUCR_ISC01);
			SET_BIT(MCUCR, MCUCR_ISC00);
			break;

		case EXTI_FALLING_EDGE:
			SET_BIT(MCUCR, MCUCR_ISC01);
			CLR_BIT(MCUCR, MCUCR_ISC00);
			break;

		case EXTI_RISING_EDGE:
			SET_BIT(MCUCR, MCUCR_ISC01);
			SET_BIT(MCUCR, MCUCR_ISC00);
			break;
		default : return;
	}
	/*Enable INT0*/
	SET_BIT(GICR , GICR_INT0);
}

void EXTI_vidInt1Init(EXTI_Sense_t Copy_u8Sense)
{
	/* Configure sense control bits */
	switch(Copy_u8Sense)
	{
		case EXTI_LOW_LEVEL:
			CLR_BIT(MCUCR, MCUCR_ISC11);
			CLR_BIT(MCUCR, MCUCR_ISC10);
			break;

		case EXTI_ANY_CHANGE:
			CLR_BIT(MCUCR, MCUCR_ISC11);
			SET_BIT(MCUCR, MCUCR_ISC10);
			break;

		case EXTI_FALLING_EDGE:
			SET_BIT(MCUCR, MCUCR_ISC11);
			CLR_BIT(MCUCR, MCUCR_ISC10);
			break;

		case EXTI_RISING_EDGE:
			SET_BIT(MCUCR, MCUCR_ISC11);
			SET_BIT(MCUCR, MCUCR_ISC10);
			break;

		default:
			return;
	}

	/* Enable INT1 */
	SET_BIT(GICR, GICR_INT1);
}

void EXTI_vidInt2Init(EXTI_Sense_t Copy_u8Sense)
{
    /* Configure sense control bits */
    switch(Copy_u8Sense)
    {
        case EXTI_FALLING_EDGE:
            CLR_BIT(MCUCSR, MCUCSR_ISC2);
            break;

        case EXTI_RISING_EDGE:
            SET_BIT(MCUCSR, MCUCSR_ISC2);
            break;

        default:
            return;
    }

    /* Enable INT2 */
    SET_BIT(GICR, GICR_INT2);
}

void EXTI_vidSetCallback(EXTI_Int_t Copy_u8Int,void (*Copy_pvCallback)(void))
{
	switch(Copy_u8Int)
	{
	    case EXTI_INT0:
	        EXTI_pvInt0Callback = Copy_pvCallback;
	        break;

	    case EXTI_INT1:
	        EXTI_pvInt1Callback = Copy_pvCallback;
	        break;

	    case EXTI_INT2:
	        EXTI_pvInt2Callback = Copy_pvCallback;
	        break;

	    default:
	        return;
	}
}
void EXTI_vidDisable(EXTI_Int_t Copy_u8Int)
{
    switch(Copy_u8Int)
    {
        case EXTI_INT0:
            CLR_BIT(GICR, GICR_INT0);
            break;

        case EXTI_INT1:
            CLR_BIT(GICR, GICR_INT1);
            break;

        case EXTI_INT2:
            CLR_BIT(GICR, GICR_INT2);
            break;

        default:
            break;
    }
}

void __vector_1(void)__attribute__((signal));
void __vector_1(void)
{
	if(EXTI_pvInt0Callback != NULL)
	{
		EXTI_pvInt0Callback();
	}
}

void __vector_2(void)__attribute__((signal));
void __vector_2(void)
{
	if(EXTI_pvInt1Callback != NULL)
	{
		EXTI_pvInt1Callback();
	}
}

void __vector_3(void)__attribute__((signal));
void __vector_3(void)
{
	if(EXTI_pvInt2Callback != NULL)
	{
		EXTI_pvInt2Callback();
	}
}



