/*
 * TWI_private.h
 *
 * Created on: Sep 13, 2026
 * Author: Beshoy
 */

#ifndef MCAL_TWI_TWI_PRIVATE_H_
#define MCAL_TWI_TWI_PRIVATE_H_


#include "../../LIB/std_types.h"


/* ================= TWI Registers ================= */

#define TWI_TWAR                (*(volatile u8*)0x22)
#define TWI_TWBR                (*(volatile u8*)0x20)
#define TWI_TWSR                (*(volatile u8*)0x21)
#define TWI_TWDR                (*(volatile u8*)0x23)
#define TWI_TWCR                (*(volatile u8*)0x56)
#define TWI_TWAMR               (*(volatile u8*)0x5D)


/* ================= TWAR Bits ================= */

#define TWI_TWAR_TWGCE           0


/* ================= TWCR Bits ================= */

#define TWI_TWCR_TWINT           7
#define TWI_TWCR_TWEA            6
#define TWI_TWCR_TWSTA            5
#define TWI_TWCR_TWSTO            4
#define TWI_TWCR_TWWC             3
#define TWI_TWCR_TWEN             2
#define TWI_TWCR_TWIE             0


/* ================= TWSR Bits ================= */

#define TWI_TWSR_TWPS0            0
#define TWI_TWSR_TWPS1            1


/* ================= TWI Status Mask ================= */

#define TWI_STATUS_MASK           0xF8

#define TWI_STATUS_BUS_ERROR      0x00


/* ================= Master Transmitter ================= */

#define TWI_START_ACK             0x08
#define TWI_REP_START_ACK         0x10

#define TWI_MT_SLA_W_ACK          0x18
#define TWI_MT_SLA_W_NACK         0x20

#define TWI_MT_DATA_ACK           0x28
#define TWI_MT_DATA_NACK          0x30

#define TWI_MT_ARB_LOST           0x38


/* ================= Master Receiver ================= */

#define TWI_MR_SLA_R_ACK          0x40
#define TWI_MR_SLA_R_NACK         0x48

#define TWI_MR_DATA_ACK           0x50
#define TWI_MR_DATA_NACK          0x58


/* ================= Slave Receiver ================= */

#define TWI_SR_SLA_W_ACK          0x60

#define TWI_SR_ARB_LOST_SLA_W_ACK 0x68

#define TWI_SR_GCALL_ACK          0x70

#define TWI_SR_ARB_LOST_GCALL_ACK 0x78

#define TWI_SR_DATA_ACK           0x80
#define TWI_SR_DATA_NACK          0x88

#define TWI_SR_GCALL_DATA_ACK     0x90
#define TWI_SR_GCALL_DATA_NACK    0x98

#define TWI_SR_STOP_OR_REP_START  0xA0


/* ================= Slave Transmitter ================= */

#define TWI_ST_SLA_R_ACK          0xA8

#define TWI_ST_ARB_LOST_SLA_R_ACK 0xB0

#define TWI_ST_DATA_ACK           0xB8
#define TWI_ST_DATA_NACK          0xC0

#define TWI_ST_LAST_DATA          0xC8


/* ================= TWI Prescaler Mask ================= */

#define TWI_PRESCALER_MASK        0x03


/*
 * SCL Frequency:
 *
 * SCL = F_CPU / (16 + 2 * TWBR * Prescaler)
 */


#endif /* MCAL_TWI_TWI_PRIVATE_H_ */
