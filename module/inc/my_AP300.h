/*
 * my_AP300.h
 *
 *  Created on: 11 juil. 2018
 *      Author: Pascal
 */

#ifndef MODULE_INC_MY_AP300_H_
#define MODULE_INC_MY_AP300_H_


#include "uip.h"
#include "httpd.h"

//UIP_UDP_APPCALL

void my_AP300_init(void);
void AP300_appcall(void);

#undef  UIP_APPCALL
#define UIP_APPCALL     AP300_appcall


#endif /* MODULE_INC_MY_AP300_H_ */
