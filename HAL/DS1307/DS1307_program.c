
 /*
 * DS1307_program.c
 *
 * Created on: Oct 9, 2026
 * Author: Beshoy
 */

#include "../../LIB/std_types.h"
#include "../../LIB/bit_math.h"

#include "DS1307_config.h"
#include "DS1307_interface.h"
#include "DS1307_private.h"

#include "../../MCAL/TWI/TWI_interface.h"


/* ================= Private Functions ================= */

/*
 * Convert Decimal to BCD
 */

static u8 DS1307_u8DecimalToBCD(u8 Copy_u8Decimal)
{
    return ((Copy_u8Decimal / 10) << 4) |
           (Copy_u8Decimal % 10);
}


/*
 * Convert BCD to Decimal
 */

static u8 DS1307_u8BCDToDecimal(u8 Copy_u8BCD)
{
    return (((Copy_u8BCD >> 4) & 0x0F) * 10) +
           (Copy_u8BCD & 0x0F);
}


/*
 * Check whether a value contains valid BCD digits.
 */

static u8 DS1307_u8IsValidBCD(u8 Copy_u8BCD)
{
    u8 Local_u8UpperDigit;
    u8 Local_u8LowerDigit;

    Local_u8UpperDigit = (Copy_u8BCD >> 4) & 0x0F;
    Local_u8LowerDigit = Copy_u8BCD & 0x0F;

    if((Local_u8UpperDigit > 9) ||
       (Local_u8LowerDigit > 9))
    {
        return 0;
    }

    return 1;
}


/*
 * Validate Time
 *
 * Hours   : 0 -> 23
 * Minutes : 0 -> 59
 * Seconds : 0 -> 59
 */

static u8 DS1307_u8ValidateTime(u8 Copy_u8Hours,
                                u8 Copy_u8Minutes,
                                u8 Copy_u8Seconds)
{
    if(Copy_u8Hours > 23)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    if(Copy_u8Minutes > 59)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    if(Copy_u8Seconds > 59)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    return DS1307_NO_ERROR;
}


/*
 * Validate Date
 *
 * Day   : 1 -> 7
 * Date  : According to month and year
 * Month : 1 -> 12
 * Year  : 0 -> 99
 *
 * Year 00 is treated as 2000 for leap-year validation.
 */

static u8 DS1307_u8ValidateDate(u8 Copy_u8Day,
                                u8 Copy_u8Date,
                                u8 Copy_u8Month,
                                u8 Copy_u8Year)
{
    u8 Local_u8MaxDays;

    if((Copy_u8Day < 1) || (Copy_u8Day > 7))
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    if((Copy_u8Month < 1) || (Copy_u8Month > 12))
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    if((Copy_u8Year > 99) || (Copy_u8Date < 1))
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    switch(Copy_u8Month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:

            Local_u8MaxDays = 31;
            break;

        case 4:
        case 6:
        case 9:
        case 11:

            Local_u8MaxDays = 30;
            break;

        case 2:

            if((Copy_u8Year % 4) == 0)
            {
                Local_u8MaxDays = 29;
            }
            else
            {
                Local_u8MaxDays = 28;
            }

            break;

        default:

            return DS1307_INVALID_INPUT_ERROR;
    }

    if(Copy_u8Date > Local_u8MaxDays)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    return DS1307_NO_ERROR;
}


/*
 * Encode Hours According to DS1307_TIME_FORMAT.
 *
 * The application always supplies hours in 24-hour format:
 * 0 -> 23
 */

static u8 DS1307_u8EncodeHours(u8 Copy_u8Hours,
                               u8 *Copy_pu8HoursRegister)
{
    u8 Local_u8Hours12;

    if(Copy_pu8HoursRegister == NULL)
    {
        return DS1307_NULL_POINTER_ERROR;
    }

#if DS1307_TIME_FORMAT == DS1307_24_HOUR_FORMAT

    *Copy_pu8HoursRegister =
        DS1307_u8DecimalToBCD(Copy_u8Hours);

#elif DS1307_TIME_FORMAT == DS1307_12_HOUR_FORMAT

    /*
     * Convert 24-hour format to 12-hour format.
     *
     * 00:xx -> 12 AM
     * 12:xx -> 12 PM
     * 13:xx -> 01 PM
     */

    Local_u8Hours12 = Copy_u8Hours % 12;

    if(Local_u8Hours12 == 0)
    {
        Local_u8Hours12 = 12;
    }

    *Copy_pu8HoursRegister =
        DS1307_u8DecimalToBCD(Local_u8Hours12) |
        (1 << DS1307_HOURS_12_24);

    if(Copy_u8Hours >= 12)
    {
        *Copy_pu8HoursRegister |=
            (1 << DS1307_HOURS_PM);
    }

#else

#error "Invalid DS1307_TIME_FORMAT configuration"

#endif

    return DS1307_NO_ERROR;
}


/*
 * Decode Time Registers.
 *
 * Register Array:
 * [0] Seconds
 * [1] Minutes
 * [2] Hours
 *
 * The returned hours are always in 24-hour format.
 */

static u8 DS1307_u8DecodeTime(u8 *Copy_pu8Registers,
                              u8 *Copy_pu8Hours,
                              u8 *Copy_pu8Minutes,
                              u8 *Copy_pu8Seconds)
{
    u8 Local_u8Seconds;
    u8 Local_u8Minutes;
    u8 Local_u8Hours;
    u8 Local_u8Hours12;

    if((Copy_pu8Registers == NULL) ||
       (Copy_pu8Hours == NULL) ||
       (Copy_pu8Minutes == NULL) ||
       (Copy_pu8Seconds == NULL))
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    /* Remove CH bit from Seconds Register */

    Local_u8Seconds =
        Copy_pu8Registers[0] &
        ~(1 << DS1307_SECONDS_CH);

    /* Remove reserved/control bits */

    Local_u8Minutes = Copy_pu8Registers[1] & 0x7F;

    /* Validate Seconds and Minutes */

    if((DS1307_u8IsValidBCD(Local_u8Seconds) == 0) ||
       (DS1307_u8IsValidBCD(Local_u8Minutes) == 0))
    {
        return DS1307_INVALID_DATA_ERROR;
    }

    Local_u8Seconds =
        DS1307_u8BCDToDecimal(Local_u8Seconds);

    Local_u8Minutes =
        DS1307_u8BCDToDecimal(Local_u8Minutes);

    if((Local_u8Seconds > 59) ||
       (Local_u8Minutes > 59))
    {
        return DS1307_INVALID_DATA_ERROR;
    }

    /* Check Hours Format Bit */

    if(GET_BIT(Copy_pu8Registers[2], DS1307_HOURS_12_24) == 1)
    {
        /*
         * 12-hour format:
         * Bits 4:0 = BCD hours
         * Bit 5    = PM
         * Bit 6    = 12/24 format
         */

        Local_u8Hours =
            Copy_pu8Registers[2] & 0x1F;

        if(DS1307_u8IsValidBCD(Local_u8Hours) == 0)
        {
            return DS1307_INVALID_DATA_ERROR;
        }

        Local_u8Hours12 =
            DS1307_u8BCDToDecimal(Local_u8Hours);

        if((Local_u8Hours12 < 1) ||
           (Local_u8Hours12 > 12))
        {
            return DS1307_INVALID_DATA_ERROR;
        }

        /*
         * Convert to 24-hour format.
         *
         * 12 AM -> 00
         * 12 PM -> 12
         * 01 PM -> 13
         */

        Local_u8Hours = Local_u8Hours12 % 12;

        if(GET_BIT(Copy_pu8Registers[2], DS1307_HOURS_PM) == 1)
        {
            Local_u8Hours += 12;
        }
    }
    else
    {
        /* 24-hour format */

        Local_u8Hours =
            Copy_pu8Registers[2] & 0x3F;

        if(DS1307_u8IsValidBCD(Local_u8Hours) == 0)
        {
            return DS1307_INVALID_DATA_ERROR;
        }

        Local_u8Hours =
            DS1307_u8BCDToDecimal(Local_u8Hours);

        if(Local_u8Hours > 23)
        {
            return DS1307_INVALID_DATA_ERROR;
        }
    }

    /* Return Decoded Values */

    *Copy_pu8Hours   = Local_u8Hours;
    *Copy_pu8Minutes = Local_u8Minutes;
    *Copy_pu8Seconds = Local_u8Seconds;

    return DS1307_NO_ERROR;
}


/*
 * Decode Date Registers.
 *
 * Register Array:
 * [0] Day
 * [1] Date
 * [2] Month
 * [3] Year
 */

static u8 DS1307_u8DecodeDate(u8 *Copy_pu8Registers,
                              u8 *Copy_pu8Day,
                              u8 *Copy_pu8Date,
                              u8 *Copy_pu8Month,
                              u8 *Copy_pu8Year)
{
    u8 Local_u8Day;
    u8 Local_u8Date;
    u8 Local_u8Month;
    u8 Local_u8Year;

    if((Copy_pu8Registers == NULL) ||
       (Copy_pu8Day == NULL) ||
       (Copy_pu8Date == NULL) ||
       (Copy_pu8Month == NULL) ||
       (Copy_pu8Year == NULL))
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    /*
     * Day of week is stored as a binary value, not BCD.
     */

    Local_u8Day = Copy_pu8Registers[0] & 0x07;

    Local_u8Date = Copy_pu8Registers[1] & 0x3F;
    Local_u8Month = Copy_pu8Registers[2] & 0x1F;
    Local_u8Year = Copy_pu8Registers[3];

    /* Validate BCD Values */

    if((DS1307_u8IsValidBCD(Local_u8Date) == 0) ||
       (DS1307_u8IsValidBCD(Local_u8Month) == 0) ||
       (DS1307_u8IsValidBCD(Local_u8Year) == 0))
    {
        return DS1307_INVALID_DATA_ERROR;
    }

    Local_u8Date =
        DS1307_u8BCDToDecimal(Local_u8Date);

    Local_u8Month =
        DS1307_u8BCDToDecimal(Local_u8Month);

    Local_u8Year =
        DS1307_u8BCDToDecimal(Local_u8Year);

    /* Validate Decoded Values */

    if(DS1307_u8ValidateDate(Local_u8Day,
                            Local_u8Date,
                            Local_u8Month,
                            Local_u8Year) != DS1307_NO_ERROR)
    {
        return DS1307_INVALID_DATA_ERROR;
    }

    /* Return Decoded Values */

    *Copy_pu8Day   = Local_u8Day;
    *Copy_pu8Date  = Local_u8Date;
    *Copy_pu8Month = Local_u8Month;
    *Copy_pu8Year  = Local_u8Year;

    return DS1307_NO_ERROR;
}


/* ================= TWI Communication Functions ================= */

/*
 * Write consecutive DS1307 registers.
 *
 * The first transmitted byte is the register address.
 * The remaining bytes are written consecutively.
 */

static u8 DS1307_u8WriteRegisters(u8 Copy_u8StartRegister,
                                  const u8 *Copy_pu8Data,
                                  u8 Copy_u8Length)
{
    u8 Local_u8Status;
    u8 Local_u8Error;
    u8 Local_u8Index;

    if(Copy_pu8Data == NULL)
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    if(Copy_u8Length == 0)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    /* Send START */

    Local_u8Status = TWI_u8SendStartCondition();

    if(Local_u8Status != TWI_NO_ERROR)
    {
        return DS1307_TWI_ERROR;
    }

    /* Send DS1307 Address With Write */

    Local_u8Status =
        TWI_u8SendSlaveAddressWithWrite(DS1307_ADDRESS);

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Send Starting Register Address */

    Local_u8Status =
        TWI_u8MasterSendDataByte(Copy_u8StartRegister);

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Send Data Bytes */

    for(Local_u8Index = 0;
        Local_u8Index < Copy_u8Length;
        Local_u8Index++)
    {
        Local_u8Status =
            TWI_u8MasterSendDataByte(Copy_pu8Data[Local_u8Index]);

        if(Local_u8Status != TWI_NO_ERROR)
        {
            Local_u8Error = DS1307_TWI_ERROR;
            goto StopCondition;
        }
    }

    Local_u8Error = DS1307_NO_ERROR;

StopCondition:

    Local_u8Status = TWI_u8SendStopCondition();

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
    }

    return Local_u8Error;
}


/*
 * Read consecutive DS1307 registers.
 *
 * Sequence:
 * START
 * SLA + W
 * Register Address
 * Repeated START
 * SLA + R
 * Read Data
 * STOP
 */

static u8 DS1307_u8ReadRegisters(u8 Copy_u8StartRegister,
                                 u8 *Copy_pu8Data,
                                 u8 Copy_u8Length)
{
    u8 Local_u8Status;
    u8 Local_u8Error;
    u8 Local_u8Index;
    u8 Local_u8Ack;

    if(Copy_pu8Data == NULL)
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    if(Copy_u8Length == 0)
    {
        return DS1307_INVALID_INPUT_ERROR;
    }

    /* Send START */

    Local_u8Status = TWI_u8SendStartCondition();

    if(Local_u8Status != TWI_NO_ERROR)
    {
        return DS1307_TWI_ERROR;
    }

    /* Send DS1307 Address With Write */

    Local_u8Status =
        TWI_u8SendSlaveAddressWithWrite(DS1307_ADDRESS);

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Send Register Address */

    Local_u8Status =
        TWI_u8MasterSendDataByte(Copy_u8StartRegister);

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Send Repeated START */

    Local_u8Status = TWI_u8SendRepeatedStart();

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Send DS1307 Address With Read */

    Local_u8Status =
        TWI_u8SendSlaveAddressWithRead(DS1307_ADDRESS);

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
        goto StopCondition;
    }

    /* Receive Data Bytes */

    for(Local_u8Index = 0;
        Local_u8Index < Copy_u8Length;
        Local_u8Index++)
    {
        /*
         * Send ACK for every byte except the last byte.
         * Send NACK for the last byte.
         */

        if(Local_u8Index < (Copy_u8Length - 1))
        {
            Local_u8Ack = 1;
        }
        else
        {
            Local_u8Ack = 0;
        }

        Local_u8Status =
            TWI_u8MasterReceiveDataByte(
                &Copy_pu8Data[Local_u8Index],
                Local_u8Ack
            );

        if(Local_u8Status != TWI_NO_ERROR)
        {
            Local_u8Error = DS1307_TWI_ERROR;
            goto StopCondition;
        }
    }

    Local_u8Error = DS1307_NO_ERROR;

StopCondition:

    Local_u8Status = TWI_u8SendStopCondition();

    if(Local_u8Status != TWI_NO_ERROR)
    {
        Local_u8Error = DS1307_TWI_ERROR;
    }

    return Local_u8Error;
}


/* ================= Public Functions ================= */


/*
 * Initialize DS1307.
 *
 * 1- Clear CH to start the oscillator.
 * 2- Disable the square-wave output by setting
 *    the control register to zero.
 *
 * TWI Master must be initialized before calling this function.
 */

void DS1307_vidInit(void)
{
    u8 Local_u8Seconds;
    u8 Local_u8Control;

    /*
     * Read Seconds Register.
     * Do not overwrite the current seconds value.
     */

    if(DS1307_u8ReadRegisters(DS1307_SECONDS_REG,
                              &Local_u8Seconds,
                              1) == DS1307_NO_ERROR)
    {
        /* Clear Clock Halt */

        Local_u8Seconds &=
            ~(1 << DS1307_SECONDS_CH);

        DS1307_u8WriteRegisters(DS1307_SECONDS_REG,
                                &Local_u8Seconds,
                                1);
    }

    /* Disable Square-Wave Output */

    Local_u8Control = 0x00;

    DS1307_u8WriteRegisters(DS1307_CONTROL_REG,
                            &Local_u8Control,
                            1);
}


/* ================= Set Time and Date ================= */

u8 DS1307_u8SetTimeDate(DS1307_TimeDate_t Copy_TimeDate)
{
    u8 Local_u8Data[7];
    u8 Local_u8Status;

    /* Validate Time */

    Local_u8Status =
        DS1307_u8ValidateTime(
            Copy_TimeDate.DS1307_Hours,
            Copy_TimeDate.DS1307_Minutes,
            Copy_TimeDate.DS1307_Seconds
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Validate Date */

    Local_u8Status =
        DS1307_u8ValidateDate(
            Copy_TimeDate.DS1307_Day,
            Copy_TimeDate.DS1307_Date,
            Copy_TimeDate.DS1307_Month,
            Copy_TimeDate.DS1307_Year
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Prepare Seconds Register */

    Local_u8Data[0] =
        DS1307_u8DecimalToBCD(
            Copy_TimeDate.DS1307_Seconds
        );

    /*
     * CH remains zero, so the oscillator is enabled.
     */

    /* Prepare Minutes Register */

    Local_u8Data[1] =
        DS1307_u8DecimalToBCD(
            Copy_TimeDate.DS1307_Minutes
        );

    /* Prepare Hours Register */

    Local_u8Status =
        DS1307_u8EncodeHours(
            Copy_TimeDate.DS1307_Hours,
            &Local_u8Data[2]
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /*
     * Day of week is stored as a binary value.
     * The remaining date fields use BCD.
     */

    Local_u8Data[3] = Copy_TimeDate.DS1307_Day;

    Local_u8Data[4] =
        DS1307_u8DecimalToBCD(
            Copy_TimeDate.DS1307_Date
        );

    Local_u8Data[5] =
        DS1307_u8DecimalToBCD(
            Copy_TimeDate.DS1307_Month
        );

    Local_u8Data[6] =
        DS1307_u8DecimalToBCD(
            Copy_TimeDate.DS1307_Year
        );

    /* Write Registers 0x00 -> 0x06 */

    return DS1307_u8WriteRegisters(
        DS1307_SECONDS_REG,
        Local_u8Data,
        7
    );
}


/* ================= Get Time and Date ================= */

u8 DS1307_u8GetTimeDate(DS1307_TimeDate_t *Copy_pTimeDate)
{
    u8 Local_u8Registers[7];
    u8 Local_u8Status;
    DS1307_TimeDate_t Local_TimeDate;

    if(Copy_pTimeDate == NULL)
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    /* Read Registers 0x00 -> 0x06 */

    Local_u8Status =
        DS1307_u8ReadRegisters(
            DS1307_SECONDS_REG,
            Local_u8Registers,
            7
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Decode Time */

    Local_u8Status =
        DS1307_u8DecodeTime(
            Local_u8Registers,
            &Local_TimeDate.DS1307_Hours,
            &Local_TimeDate.DS1307_Minutes,
            &Local_TimeDate.DS1307_Seconds
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Decode Date */

    Local_u8Status =
        DS1307_u8DecodeDate(
            &Local_u8Registers[3],
            &Local_TimeDate.DS1307_Day,
            &Local_TimeDate.DS1307_Date,
            &Local_TimeDate.DS1307_Month,
            &Local_TimeDate.DS1307_Year
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Update Output Structure */

    *Copy_pTimeDate = Local_TimeDate;

    return DS1307_NO_ERROR;
}


/* ================= Set Time ================= */

u8 DS1307_u8SetTime(u8 Copy_u8Hours,
                    u8 Copy_u8Minutes,
                    u8 Copy_u8Seconds)
{
    u8 Local_u8Data[3];
    u8 Local_u8Status;

    /* Validate Input */

    Local_u8Status =
        DS1307_u8ValidateTime(
            Copy_u8Hours,
            Copy_u8Minutes,
            Copy_u8Seconds
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Prepare Seconds */

    Local_u8Data[0] =
        DS1307_u8DecimalToBCD(Copy_u8Seconds);

    /* Prepare Minutes */

    Local_u8Data[1] =
        DS1307_u8DecimalToBCD(Copy_u8Minutes);

    /* Prepare Hours */

    Local_u8Status =
        DS1307_u8EncodeHours(
            Copy_u8Hours,
            &Local_u8Data[2]
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Write Seconds, Minutes and Hours */

    return DS1307_u8WriteRegisters(
        DS1307_SECONDS_REG,
        Local_u8Data,
        3
    );
}


/* ================= Set Date ================= */

u8 DS1307_u8SetDate(u8 Copy_u8Day,
                    u8 Copy_u8Date,
                    u8 Copy_u8Month,
                    u8 Copy_u8Year)
{
    u8 Local_u8Data[4];
    u8 Local_u8Status;

    /* Validate Input */

    Local_u8Status =
        DS1307_u8ValidateDate(
            Copy_u8Day,
            Copy_u8Date,
            Copy_u8Month,
            Copy_u8Year
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Day of Week: Binary */

    Local_u8Data[0] = Copy_u8Day;

    /* Date: BCD */

    Local_u8Data[1] =
        DS1307_u8DecimalToBCD(Copy_u8Date);

    /* Month: BCD */

    Local_u8Data[2] =
        DS1307_u8DecimalToBCD(Copy_u8Month);

    /* Year: BCD */

    Local_u8Data[3] =
        DS1307_u8DecimalToBCD(Copy_u8Year);

    /* Write Day, Date, Month and Year */

    return DS1307_u8WriteRegisters(
        DS1307_DAY_REG,
        Local_u8Data,
        4
    );
}


/* ================= Get Time ================= */

u8 DS1307_u8GetTime(u8 *Copy_pu8Hours,
                    u8 *Copy_pu8Minutes,
                    u8 *Copy_pu8Seconds)
{
    u8 Local_u8Registers[3];
    u8 Local_u8Status;

    if((Copy_pu8Hours == NULL) ||
       (Copy_pu8Minutes == NULL) ||
       (Copy_pu8Seconds == NULL))
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    /* Read Seconds, Minutes and Hours */

    Local_u8Status =
        DS1307_u8ReadRegisters(
            DS1307_SECONDS_REG,
            Local_u8Registers,
            3
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Decode Time */

    return DS1307_u8DecodeTime(
        Local_u8Registers,
        Copy_pu8Hours,
        Copy_pu8Minutes,
        Copy_pu8Seconds
    );
}


/* ================= Get Date ================= */

u8 DS1307_u8GetDate(u8 *Copy_pu8Day,
                    u8 *Copy_pu8Date,
                    u8 *Copy_pu8Month,
                    u8 *Copy_pu8Year)
{
    u8 Local_u8Registers[4];
    u8 Local_u8Status;

    if((Copy_pu8Day == NULL) ||
       (Copy_pu8Date == NULL) ||
       (Copy_pu8Month == NULL) ||
       (Copy_pu8Year == NULL))
    {
        return DS1307_NULL_POINTER_ERROR;
    }

    /* Read Day, Date, Month and Year */

    Local_u8Status =
        DS1307_u8ReadRegisters(
            DS1307_DAY_REG,
            Local_u8Registers,
            4
        );

    if(Local_u8Status != DS1307_NO_ERROR)
    {
        return Local_u8Status;
    }

    /* Decode Date */

    return DS1307_u8DecodeDate(
        Local_u8Registers,
        Copy_pu8Day,
        Copy_pu8Date,
        Copy_pu8Month,
        Copy_pu8Year
    );
}
