/*
 * protheo.h
 *
 *  Created on: 11 juil. 2018
 *      Author: Pascal
 */

#ifndef MODULE_INC_PROTHEO_H_
#define MODULE_INC_PROTHEO_H_

//#include "uip.h"
#include "my_AP300.h"

void protheo_init(void);
void protheo_appcall(void);

void PBAR_aborted(void);
void PBAR_timedout(void);
void PBAR_closed(void);
void PBAR_connected(void);
void PBAR_acked(void);
void PBAR_newdata(void);
void PBAR_senddata(void);
#endif /* MODULE_INC_PROTHEO_H_ */
