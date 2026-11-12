/*
 * DS1307_interface.h
 *
 *  Created on: Oct 9, 2026
 *      Author: Beshoy
 */

#ifndef HAL_DS1307_DS1307_INTERFACE_H_
#define HAL_DS1307_DS1307_INTERFACE_H_

#include "../../LIB/std_types.h"

/* Time and Date Structure */
typedef struct
{
    u8 DS1307_Hours;
    u8 DS1307_Minutes;
    u8 DS1307_Seconds;

    u8 DS1307_Day;
    u8 DS1307_Date;
    u8 DS1307_Month;
    u8 DS1307_Year;

} DS1307_TimeDate_t;

/* Error Status */

typedef enum
{
    DS1307_NO_ERROR = 0,
    DS1307_TWI_ERROR,
    DS1307_INVALID_INPUT_ERROR,
    DS1307_NULL_POINTER_ERROR,
    DS1307_INVALID_DATA_ERROR

} DS1307_ErrorStatus_t;

/* Function Prototypes */
void DS1307_vidInit(void);

u8 DS1307_u8SetTimeDate(DS1307_TimeDate_t Copy_TimeDate);

u8 DS1307_u8GetTimeDate(DS1307_TimeDate_t *Copy_pTimeDate);

u8 DS1307_u8SetTime(u8 Copy_u8Hours,
                    u8 Copy_u8Minutes,
                    u8 Copy_u8Seconds);

u8 DS1307_u8SetDate(u8 Copy_u8Day,
                    u8 Copy_u8Date,
                    u8 Copy_u8Month,
                    u8 Copy_u8Year);

u8 DS1307_u8GetTime(u8 *Copy_pu8Hours,
                    u8 *Copy_pu8Minutes,
                    u8 *Copy_pu8Seconds);

u8 DS1307_u8GetDate(u8 *Copy_pu8Day,
                    u8 *Copy_pu8Date,
                    u8 *Copy_pu8Month,
                    u8 *Copy_pu8Year);

#endif /* HAL_DS1307_DS1307_INTERFACE_H_ */
