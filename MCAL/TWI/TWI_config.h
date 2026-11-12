
/*
 * TWI_config.h
 *
 * Created on: Sep 13, 2026
 * Author: Beshoy
 */

#ifndef MCAL_TWI_TWI_CONFIG_H_
#define MCAL_TWI_TWI_CONFIG_H_


#define TWI_FCPU    8000000UL


/* ================= TWI Mode ================= */

#define TWI_MASTER_MODE             0
#define TWI_SLAVE_MODE              1

/*
 * Options:
 * TWI_MASTER_MODE
 * TWI_SLAVE_MODE
 */
#define TWI_MODE                    TWI_MASTER_MODE


/* ================= TWI Clock Prescaler ================= */

#define TWI_PRESCALER_1             0
#define TWI_PRESCALER_4             1
#define TWI_PRESCALER_16            2
#define TWI_PRESCALER_64            3

/*
 * Options:
 * TWI_PRESCALER_1
 * TWI_PRESCALER_4
 * TWI_PRESCALER_16
 * TWI_PRESCALER_64
 */
#define TWI_PRESCALER               TWI_PRESCALER_1


/* ================= TWI Bit Rate ================= */

/*
 * Valid Range:
 * 0 -> 255
 */
#define TWI_BIT_RATE                32


/* ================= Slave Address ================= */

/*
 * Valid Range:
 * 0x08 -> 0x77
 */
#define TWI_MY_ADDRESS              0x10


/* ================= General Call ================= */

#define TWI_GENERAL_CALL_DISABLE    0
#define TWI_GENERAL_CALL_ENABLE     1

/*
 * Options:
 * TWI_GENERAL_CALL_DISABLE
 * TWI_GENERAL_CALL_ENABLE
 */
#define TWI_GENERAL_CALL            TWI_GENERAL_CALL_DISABLE


/* ================= TWI Interrupt ================= */

#define TWI_INTERRUPT_DISABLE       0
#define TWI_INTERRUPT_ENABLE        1

/*
 * The current driver implementation uses Polling.
 * Keep TWI_INTERRUPT_DISABLE until an ISR is implemented.
 */
#define TWI_INTERRUPT               TWI_INTERRUPT_DISABLE


#endif /* MCAL_TWI_TWI_CONFIG_H_ */
