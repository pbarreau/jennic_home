/*
 * protheo.c
 *
 *  Created on: 11 juil. 2018
 *      Author: Pascal
 */

#include "protheo.h"

void protheo_init(void)
{
  uip_listen(HTONS(1968));
  my_AP300_init();
}

// ------------------------------
void protheo_appcall(void)
{
  if(uip_aborted()) {
    PBAR_aborted();
  }
  if(uip_timedout()) {
    PBAR_timedout();
  }
  if(uip_closed()) {
    PBAR_closed();
  }
  if(uip_connected()) {
    PBAR_connected();
  }
  if(uip_acked()) {
    PBAR_acked();
  }
  if(uip_newdata()) {
    PBAR_newdata();
  }
  if(uip_rexmit() ||
      uip_newdata() ||
      uip_acked() ||
      uip_connected() ||
      uip_poll()) {
    PBAR_senddata();
  }
}

// -------------------------

void PBAR_aborted(void)
{

}
void PBAR_timedout(void)
{

}
void PBAR_closed(void)
{

}
void PBAR_connected(void)
{

}
void PBAR_acked(void)
{

}
void PBAR_newdata(void)
{

}
void PBAR_senddata(void)
{

}
