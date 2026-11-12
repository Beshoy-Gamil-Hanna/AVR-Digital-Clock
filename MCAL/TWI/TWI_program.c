
/*
 * TWI_program.c
 *
 * Created on: Sep 13, 2026
 * Author: Beshoy
 */

#include <stddef.h>

#include "../../LIB/bit_math.h"

#include "TWI_config.h"
#include "TWI_private.h"
#include "TWI_interface.h"


#define TWI_TIMEOUT    100000UL


/*
 * This implementation uses Polling.
 * Slave interrupt mode requires a separate ISR implementation.
 */

#if (TWI_MODE == TWI_SLAVE_MODE) && \
    (TWI_INTERRUPT == TWI_INTERRUPT_ENABLE)

#error "TWI interrupt mode is not implemented. Use TWI_INTERRUPT_DISABLE."

#endif


/* ================= Private Functions ================= */

static u8 TWI_u8WaitForTWINT(void)
{
    u32 Local_u32Timeout = TWI_TIMEOUT;

    while((GET_BIT(TWI_TWCR, TWI_TWCR_TWINT) == 0) &&
          (Local_u32Timeout > 0))
    {
        Local_u32Timeout--;
    }

    if(Local_u32Timeout == 0)
    {
        return TWI_TIMEOUT_ERROR;
    }

    return TWI_NO_ERROR;
}


/* ================= Master Functions ================= */

#if TWI_MODE == TWI_MASTER_MODE


void TWI_vidInitMaster(void)
{
    /* Configure Bit Rate */

    TWI_TWBR = TWI_BIT_RATE;

    /* Configure Prescaler */

    TWI_TWSR &= ~(TWI_PRESCALER_MASK);
    TWI_TWSR |= TWI_PRESCALER;

    /* Enable TWI */

    TWI_TWCR = (1 << TWI_TWCR_TWEN);
}


/* ================= START Condition ================= */

u8 TWI_u8SendStartCondition(void)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    /* Send START Condition */

    TWI_TWCR = (1 << TWI_TWCR_TWINT) |
               (1 << TWI_TWCR_TWSTA) |
               (1 << TWI_TWCR_TWEN);

    ErrorState = TWI_u8WaitForTWINT();

    if(ErrorState == TWI_NO_ERROR)
    {
        Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

        if(Local_u8Status == TWI_START_ACK)
        {
            ErrorState = TWI_NO_ERROR;
        }
        else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
        {
            ErrorState = TWI_BUS_ERROR;
        }
        else
        {
            ErrorState = TWI_START_ERROR;
        }
    }

    return ErrorState;
}


/* ================= Repeated START Condition ================= */

u8 TWI_u8SendRepeatedStart(void)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    /* Send Repeated START Condition */

    TWI_TWCR = (1 << TWI_TWCR_TWINT) |
               (1 << TWI_TWCR_TWSTA) |
               (1 << TWI_TWCR_TWEN);

    ErrorState = TWI_u8WaitForTWINT();

    if(ErrorState == TWI_NO_ERROR)
    {
        Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

        if(Local_u8Status == TWI_REP_START_ACK)
        {
            ErrorState = TWI_NO_ERROR;
        }
        else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
        {
            ErrorState = TWI_BUS_ERROR;
        }
        else
        {
            ErrorState = TWI_REP_START_ERROR;
        }
    }

    return ErrorState;
}


/* ================= STOP Condition ================= */

u8 TWI_u8SendStopCondition(void)
{
    u8 ErrorState = TWI_NO_ERROR;
    u32 Local_u32Timeout = TWI_TIMEOUT;

    /* Send STOP Condition */

    TWI_TWCR = (1 << TWI_TWCR_TWINT) |
               (1 << TWI_TWCR_TWSTO) |
               (1 << TWI_TWCR_TWEN);

    /*
     * TWSTO is cleared automatically by hardware
     * after the STOP condition has been transmitted.
     */

    while((GET_BIT(TWI_TWCR, TWI_TWCR_TWSTO) == 1) &&
          (Local_u32Timeout > 0))
    {
        Local_u32Timeout--;
    }

    if(Local_u32Timeout == 0)
    {
        ErrorState = TWI_TIMEOUT_ERROR;
    }

    return ErrorState;
}


/* ================= Slave Address With Write ================= */

u8 TWI_u8SendSlaveAddressWithWrite(u8 Copy_u8SlaveAddress)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    if((Copy_u8SlaveAddress < 0x08) ||
       (Copy_u8SlaveAddress > 0x77))
    {
        ErrorState = TWI_SLA_W_ERROR;
    }
    else
    {
        /* SLA + W */

        TWI_TWDR = (Copy_u8SlaveAddress << 1);

        TWI_TWCR = (1 << TWI_TWCR_TWINT) |
                   (1 << TWI_TWCR_TWEN);

        ErrorState = TWI_u8WaitForTWINT();

        if(ErrorState == TWI_NO_ERROR)
        {
            Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

            if(Local_u8Status == TWI_MT_SLA_W_ACK)
            {
                ErrorState = TWI_NO_ERROR;
            }
            else if(Local_u8Status == TWI_MT_ARB_LOST)
            {
                ErrorState = TWI_ARBITRATION_LOST_ERROR;
            }
            else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
            {
                ErrorState = TWI_BUS_ERROR;
            }
            else
            {
                ErrorState = TWI_SLA_W_ERROR;
            }
        }
    }

    return ErrorState;
}


/* ================= Slave Address With Read ================= */

u8 TWI_u8SendSlaveAddressWithRead(u8 Copy_u8SlaveAddress)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    if((Copy_u8SlaveAddress < 0x08) ||
       (Copy_u8SlaveAddress > 0x77))
    {
        ErrorState = TWI_SLA_R_ERROR;
    }
    else
    {
        /* SLA + R */

        TWI_TWDR = (Copy_u8SlaveAddress << 1) | 1;

        TWI_TWCR = (1 << TWI_TWCR_TWINT) |
                   (1 << TWI_TWCR_TWEN);

        ErrorState = TWI_u8WaitForTWINT();

        if(ErrorState == TWI_NO_ERROR)
        {
            Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

            if(Local_u8Status == TWI_MR_SLA_R_ACK)
            {
                ErrorState = TWI_NO_ERROR;
            }
            else if(Local_u8Status == TWI_MT_ARB_LOST)
            {
                ErrorState = TWI_ARBITRATION_LOST_ERROR;
            }
            else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
            {
                ErrorState = TWI_BUS_ERROR;
            }
            else
            {
                ErrorState = TWI_SLA_R_ERROR;
            }
        }
    }

    return ErrorState;
}


/* ================= Master Send Data Byte ================= */

u8 TWI_u8MasterSendDataByte(u8 Copy_u8TxData)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    /* Load Data */

    TWI_TWDR = Copy_u8TxData;

    /* Start Data Transmission */

    TWI_TWCR = (1 << TWI_TWCR_TWINT) |
               (1 << TWI_TWCR_TWEN);

    ErrorState = TWI_u8WaitForTWINT();

    if(ErrorState == TWI_NO_ERROR)
    {
        Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

        if(Local_u8Status == TWI_MT_DATA_ACK)
        {
            ErrorState = TWI_NO_ERROR;
        }
        else if(Local_u8Status == TWI_MT_ARB_LOST)
        {
            ErrorState = TWI_ARBITRATION_LOST_ERROR;
        }
        else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
        {
            ErrorState = TWI_BUS_ERROR;
        }
        else
        {
            ErrorState = TWI_DATA_TRANSMIT_ERROR;
        }
    }

    return ErrorState;
}


/* ================= Master Receive Data Byte ================= */

/*
 * Copy_u8Ack:
 * 1 -> Receive data and send ACK.
 * 0 -> Receive data and send NACK.
 */

u8 TWI_u8MasterReceiveDataByte(u8 *Copy_pu8RxData, u8 Copy_u8Ack)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    if(Copy_pu8RxData == NULL)
    {
        ErrorState = TWI_NULL_POINTER_ERROR;
    }
    else if(Copy_u8Ack > 1)
    {
        ErrorState = TWI_DATA_RECEIVE_ERROR;
    }
    else
    {
        /* Configure ACK / NACK */

        if(Copy_u8Ack == 1)
        {
            TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                       (1 << TWI_TWCR_TWINT) |
                       (1 << TWI_TWCR_TWEN);
        }
        else
        {
            TWI_TWCR = (1 << TWI_TWCR_TWINT) |
                       (1 << TWI_TWCR_TWEN);
        }

        ErrorState = TWI_u8WaitForTWINT();

        if(ErrorState == TWI_NO_ERROR)
        {
            Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

            if((Copy_u8Ack == 1) &&
               (Local_u8Status == TWI_MR_DATA_ACK))
            {
                *Copy_pu8RxData = TWI_TWDR;
            }
            else if((Copy_u8Ack == 0) &&
                    (Local_u8Status == TWI_MR_DATA_NACK))
            {
                *Copy_pu8RxData = TWI_TWDR;
            }
            else if(Local_u8Status == TWI_MT_ARB_LOST)
            {
                ErrorState = TWI_ARBITRATION_LOST_ERROR;
            }
            else if(Local_u8Status == TWI_STATUS_BUS_ERROR)
            {
                ErrorState = TWI_BUS_ERROR;
            }
            else
            {
                ErrorState = TWI_DATA_RECEIVE_ERROR;
            }
        }
    }

    return ErrorState;
}


#endif /* TWI_MODE == TWI_MASTER_MODE */


/* ================= Slave Functions ================= */

#if TWI_MODE == TWI_SLAVE_MODE


void TWI_vidInitSlave(void)
{
    /* Configure Own Slave Address */

    TWI_TWAR = (TWI_MY_ADDRESS << 1);

    /* Configure General Call */

#if TWI_GENERAL_CALL == TWI_GENERAL_CALL_ENABLE

    SET_BIT(TWI_TWAR, TWI_TWAR_TWGCE);

#endif

    /*
     * Enable TWI and ACK.
     * Interrupt mode is not implemented in this version.
     */

    TWI_TWCR = (1 << TWI_TWCR_TWEA) |
               (1 << TWI_TWCR_TWEN);
}


/* ================= Slave Listen ================= */

u8 TWI_u8SlaveListen(u8 *Copy_pu8Stats)
{
    u8 ErrorState = TWI_NO_ERROR;

    if(Copy_pu8Stats == NULL)
    {
        ErrorState = TWI_NULL_POINTER_ERROR;
    }
    else
    {
        /*
         * Wait for the current TWI event.
         *
         * Do not clear TWINT here. The caller needs
         * the current status to decide what to do next.
         */

        ErrorState = TWI_u8WaitForTWINT();

        if(ErrorState == TWI_NO_ERROR)
        {
            *Copy_pu8Stats = TWI_TWSR & TWI_STATUS_MASK;
        }
    }

    return ErrorState;
}


/* ================= Slave Send Data Byte ================= */

u8 TWI_u8SlaveSendDataByte(u8 Copy_u8Data)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    /*
     * Wait for the current event.
     *
     * Expected states before loading the next byte:
     * TWI_ST_SLA_R_ACK
     * TWI_ST_ARB_LOST_SLA_R_ACK
     * TWI_ST_DATA_ACK
     */

    ErrorState = TWI_u8WaitForTWINT();

    if(ErrorState == TWI_NO_ERROR)
    {
        Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

        if((Local_u8Status == TWI_ST_SLA_R_ACK)       ||
           (Local_u8Status == TWI_ST_ARB_LOST_SLA_R_ACK) ||
           (Local_u8Status == TWI_ST_DATA_ACK))
        {
            /* Load Data */

            TWI_TWDR = Copy_u8Data;

            /*
             * Clear TWINT to start transmission.
             * Keep TWEA enabled.
             */

            TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                       (1 << TWI_TWCR_TWINT) |
                       (1 << TWI_TWCR_TWEN);

            ErrorState = TWI_u8WaitForTWINT();

            if(ErrorState == TWI_NO_ERROR)
            {
                Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

                if(Local_u8Status == TWI_ST_DATA_ACK)
                {
                    ErrorState = TWI_NO_ERROR;
                }
                else if(Local_u8Status == TWI_ST_DATA_NACK)
                {
                    /*
                     * The Master did not ACK the transmitted byte.
                     * This commonly indicates the end of its read.
                     *
                     * Release TWINT so the Slave can wait for
                     * another addressing event.
                     */

                    TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                               (1 << TWI_TWCR_TWINT) |
                               (1 << TWI_TWCR_TWEN);

                    ErrorState = TWI_DATA_TRANSMIT_ERROR;
                }
                else if(Local_u8Status == TWI_ST_LAST_DATA)
                {
                    TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                               (1 << TWI_TWCR_TWINT) |
                               (1 << TWI_TWCR_TWEN);

                    ErrorState = TWI_NO_ERROR;
                }
                else
                {
                    ErrorState = TWI_DATA_TRANSMIT_ERROR;
                }
            }
        }
        else
        {
            ErrorState = TWI_DATA_TRANSMIT_ERROR;
        }
    }

    return ErrorState;
}


/* ================= Slave Receive Data Byte ================= */

u8 TWI_u8SlaveReceiveDataByte(u8 *Copy_pu8Data)
{
    u8 ErrorState = TWI_NO_ERROR;
    u8 Local_u8Status;

    if(Copy_pu8Data == NULL)
    {
        ErrorState = TWI_NULL_POINTER_ERROR;
    }
    else
    {
        /*
         * If no event is pending, wait for one.
         * If an event is already pending, process it.
         */

        ErrorState = TWI_u8WaitForTWINT();

        if(ErrorState == TWI_NO_ERROR)
        {
            Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;

            /*
             * If this is an address event, acknowledge it
             * and wait for the first data byte.
             */

            if((Local_u8Status == TWI_SR_SLA_W_ACK)          ||
               (Local_u8Status == TWI_SR_ARB_LOST_SLA_W_ACK) ||
               (Local_u8Status == TWI_SR_GCALL_ACK)          ||
               (Local_u8Status == TWI_SR_ARB_LOST_GCALL_ACK))
            {
                TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                           (1 << TWI_TWCR_TWINT) |
                           (1 << TWI_TWCR_TWEN);

                ErrorState = TWI_u8WaitForTWINT();

                if(ErrorState == TWI_NO_ERROR)
                {
                    Local_u8Status = TWI_TWSR & TWI_STATUS_MASK;
                }
            }

            /*
             * Receive the byte when the data status is valid.
             */

            if(ErrorState == TWI_NO_ERROR)
            {
                if((Local_u8Status == TWI_SR_DATA_ACK) ||
                   (Local_u8Status == TWI_SR_GCALL_DATA_ACK))
                {
                    *Copy_pu8Data = TWI_TWDR;

                    /*
                     * Release the current event and allow
                     * reception of the next byte.
                     */

                    TWI_TWCR = (1 << TWI_TWCR_TWEA)  |
                               (1 << TWI_TWCR_TWINT) |
                               (1 << TWI_TWCR_TWEN);
                }
                else
                {
                    ErrorState = TWI_DATA_RECEIVE_ERROR;
                }
            }
        }
    }

    return ErrorState;
}


#endif /* TWI_MODE == TWI_SLAVE_MODE */
