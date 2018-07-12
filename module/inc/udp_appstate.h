/*
 * udp_appstate.h
 *
 *  Created on: 11 juil. 2018
 *      Author: Pascal
 */

#ifndef MODULE_INC_UDP_APPSTATE_H_
#define MODULE_INC_UDP_APPSTATE_H_

// dhcpc_configured

#include "uip-conf.h"

#include "timer.h"
#include "pt.h"

struct dhcpc_state {
  struct pt pt;
  char state;
  struct uip_udp_conn *conn;
  struct timer timer;
  u16_t ticks;
  const void *mac_addr;
  int mac_len;

  u8_t serverid[4];

  u16_t lease_time[2];
  u16_t ipaddr[2];
  u16_t netmask[2];
  u16_t dnsaddr[2];
  u16_t default_router[2];
};
typedef struct dhcpc_state uip_udp_appstate_t;

void dhcpc_appcall(void);
#define UIP_UDP_APPCALL dhcpc_appcall

#endif /* MODULE_INC_UDP_APPSTATE_H_ */
