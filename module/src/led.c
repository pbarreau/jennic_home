// -----------------------------------
// Projet	: Protheo_v2
//	Fic		: led.c
//  Cree	: 3 juil. 2013
//	Par		: RapidOs
// -----------------------------------

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "led.h"

PUBLIC sLed au8Led[CST_NB_IHM_LED];

PUBLIC void IHM_ClignoteLed(void)
{
  PRIVATE uint8 uTick = 0;
  PRIVATE uint8 periode = 0;
  uint8 i;

  if (!periode)
  {
    for (i = 0; i < sizeof(au8Led) / sizeof(sLed); i++)
    {
      if (au8Led[i].actif == TRUE)
      {
        // Set LED according to status
        if (au8Led[i].mode == 0 || au8Led[i].mode == 0xFF)
          vLedSet(au8Led[i].mode, au8Led[i].pio);
        else
          vLedSet(au8Led[i].mode & (uTick), au8Led[i].pio);
      }
    }
    uTick++;    // Ticker increment
  }

  periode++;
  periode %= CST_LED_PERIOD;
}
