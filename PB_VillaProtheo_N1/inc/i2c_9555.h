/*
 * i2c_9555.h
 *
 *  Created on: 27 juin 2013
 *      Author: RapidOs
 */

#include "m_config.h"

#ifndef I2C_9555_H_
#define I2C_9555_H_

#define	ADDR_IO_1	0x20	/// Addr de 9555 sur le bus i2c
#define	ADDR_IO_2	0x21	/// Addr de 9555 sur le bus i2c

#define	CMD_INP_0	0x00	/// Registre commande lecture port 0
#define	CMD_INP_1	0x01	/// Registre commande lecture port 1
#define	CMD_OUT_0	0x02	/// Registre commande ecriture port 0
#define	CMD_OUT_1	0x03	/// Registre commande ecriture port 1
#define	CMD_PIP_0	0x04	/// Registre commande inversion de polarite port 0
#define	CMD_PIP_1	0x05	/// Registre commande inversion de polarite port 1
#define	CMD_CFG_0	0x06	/// Registre de configuration du port 0 [b0..b7]
#define	CMD_CFG_1	0x07	/// Registre de configuration du port 1 [b8..b15]

#define	CST_ANTI_REBOND_IT	5 /// antirebond it * 10 ms

typedef enum _bus_speed {
  E_BUS_400_KH = 7, E_BUS_100_KH = 31
} sBusSpeed;

extern PUBLIC bool_t bIt_DIO11;
extern PUBLIC bool_t bIt_DIO12;
extern PUBLIC bool_t bMessureDureePressionDio12;
extern PUBLIC uint16 timer_pression_DIO12;
extern PUBLIC uint16 prvCnf_I_9555;
extern PUBLIC uint16 prvCnf_O_9555;

extern PUBLIC void vPRT_Init_9555(sBusSpeed speed);
extern PUBLIC void vPRT_DioSetDirection(uint32 cnf_in, uint32 cnf_out);
extern PUBLIC void vPRT_DioSetOutput(uint32 cnf_on, uint32 cnf_off);
extern PUBLIC uint32 vPRT_DioReadInput(void);

#if !NO_DEBUG_ON
#if SHOW_TEST_1
extern PUBLIC bool_t PCA_9555_test_01 (void);
extern PUBLIC void vPRT_Dio16SetDirection(uint16 cnf_in, uint16 cnf_out);
#endif //SHOW_TEST_1

#if SHOW_TEST_2
extern PUBLIC bool_t PCA_9555_test_02 (void);
#endif //SHOW_TEST_2

#endif //!NO_DEBUG_ON

#if SHOW_TEST_3
extern PUBLIC bool_t PCA_9555_test_03 (void);
#endif //SHOW_TEST_3

#endif /* I2C_9555_H_ */
