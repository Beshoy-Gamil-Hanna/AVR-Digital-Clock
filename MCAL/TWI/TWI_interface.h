/*
 * TWI_interface.h
 *
 * Created on: Sep 13, 2026
 * Author: Beshoy
 */

#ifndef MCAL_TWI_TWI_INTERFACE_H_
#define MCAL_TWI_TWI_INTERFACE_H_


#include "../../LIB/std_types.h"
#include "TWI_config.h"


/* ================= Error Status ================= */

typedef enum
{
    TWI_NO_ERROR = 0,
    TWI_START_ERROR,
    TWI_REP_START_ERROR,
    TWI_SLA_W_ERROR,
    TWI_SLA_R_ERROR,
    TWI_DATA_TRANSMIT_ERROR,
    TWI_DATA_RECEIVE_ERROR,
    TWI_ARBITRATION_LOST_ERROR,
    TWI_STOP_ERROR,
    TWI_BUS_ERROR,
    TWI_NULL_POINTER_ERROR,
    TWI_TIMEOUT_ERROR

} TWI_ErrorStatus_t;


/*
 * TWI Driver Usage:
 *
 * Master Mode:
 * 1- Initialize TWI using TWI_vidInitMaster().
 * 2- Send START using TWI_u8SendStartCondition().
 * 3- Send the Slave Address with Write or Read.
 * 4- Send or receive data.
 * 5- Send STOP using TWI_u8SendStopCondition().
 *
 * Master Receive:
 * Copy_u8Ack = 1 -> Receive byte and send ACK.
 * Copy_u8Ack = 0 -> Receive byte and send NACK.
 *
 * Slave Mode:
 * 1- Initialize TWI using TWI_vidInitSlave().
 * 2- Wait for an event using TWI_u8SlaveListen().
 * 3- Use TWI_u8SlaveReceiveDataByte() to receive data.
 * 4- Use TWI_u8SlaveSendDataByte() to transmit data.
 *
 * The current implementation uses Polling.
 */


/* ================= Master Functions ================= */

#if TWI_MODE == TWI_MASTER_MODE

void TWI_vidInitMaster(void);

u8 TWI_u8SendStartCondition(void);

u8 TWI_u8SendRepeatedStart(void);

u8 TWI_u8SendStopCondition(void);


/* ================= Address Packet Functions ================= */

u8 TWI_u8SendSlaveAddressWithWrite(u8 Copy_u8SlaveAddress);

u8 TWI_u8SendSlaveAddressWithRead(u8 Copy_u8SlaveAddress);


/* ================= Data Packet Functions ================= */

u8 TWI_u8MasterSendDataByte(u8 Copy_u8TxData);

u8 TWI_u8MasterReceiveDataByte(u8 *Copy_pu8RxData, u8 Copy_u8Ack);

#endif


/* ================= Slave Functions ================= */

#if TWI_MODE == TWI_SLAVE_MODE

void TWI_vidInitSlave(void);

u8 TWI_u8SlaveListen(u8 *Copy_pu8Stats);

u8 TWI_u8SlaveSendDataByte(u8 Copy_u8Data);

u8 TWI_u8SlaveReceiveDataByte(u8 *Copy_pu8Data);

#endif


#endif /* MCAL_TWI_TWI_INTERFACE_H_ */
