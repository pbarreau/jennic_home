/*
 * my_AP300.c
 *
 *  Created on: 11 juil. 2018
 *      Author: Pascal
 */
#include <jendefs.h>

#include "protheo.h"
#include "my_AP300.h"

void my_AP300_init(void)
{

  httpd_init();
}

void AP300_appcall(void)
{
  switch(uip_conn->lport) {
  case HTONS(80):
    httpd_appcall();
  break;
  case HTONS(1968):
  protheo_appcall();
  break;
  }
}

