/*
 * bit_math.h
 *
 *  Created on: Aug 5, 2026
 *      Author: Beshoy
 */

#ifndef LIB_BIT_MATH_H_
#define LIB_BIT_MATH_H_


#define SET_BIT(VAR,BITNO) (VAR) |=  (1 << (BITNO))
#define CLR_BIT(VAR,BITNO) (VAR) &= ~(1 << (BITNO))
#define TOG_BIT(VAR,BITNO) (VAR) ^=  (1 << (BITNO))
#define GET_BIT(VAR,BITNO) (((VAR) >> (BITNO)) & 0x01)

#endif /* LIB_BIT_MATH_H_ */
