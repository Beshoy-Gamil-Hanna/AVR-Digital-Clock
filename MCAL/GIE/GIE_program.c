/*
 * GIE_program.c
 *
 *  Created on: Aug 23, 2026
 *      Author: Beshoy
 */


#include "../../LIB/bit_math.h"
#include "../../LIB/std_types.h"
#include "GIE_interface.h"
#include "GIE_private.h"


void GIE_vidEnable(void)
{
	SET_BIT(SREG , SREG_I);
}

void GIE_vidDisable(void)
{
	CLR_BIT(SREG , SREG_I);
}


