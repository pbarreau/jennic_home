// -----------------------------------
// Projet	: PBPJ1
//	Fic		: led.c
//  Cree	: 18 août 2012
//	Par		: Administrateur
// -----------------------------------

#include <jendefs.h>
//#include <JPI_JN514x.h>
#include <JPI.h>
//#include <Printf.h>

//#include "led.h"
#include "m_config.h"
#include "e_config.h"

#define CLIG_T		30
#define	 CLIG_E		(2 * CLIG_T)
#define CLIG_PAUSE	50

//PUBLIC bool_t bStartFlashing = FALSE;
//PUBLIC uint32 mLedId =0;
PUBLIC sLed au8Led_clav[C_CLAV_NB_IHM_LED]= {
    {FALSE,E_FLASH_FIN,C_CLAV_PIO_INFO_1},
    {FALSE,E_FLASH_FIN,C_CLAV_PIO_INFO_2},
    {FALSE,E_FLASH_FIN,C_CLAV_PIO_INFO_3}
};

PUBLIC  void IHM_ClignoteLed(void)
{
  PRIVATE uint8 uTick = 0;
  PRIVATE uint8 periode = 0;
  uint8 i;


  if(!periode){
    for(i=0;i<sizeof(au8Led_clav)/sizeof(sLed);i++){
      if(au8Led_clav[i].actif == TRUE){
        // Set LED according to status
        if(au8Led_clav[i].mode == 0 || au8Led_clav[i].mode == 0xFF)
          vLedSet(au8Led_clav[i].mode,au8Led_clav[i].pio);
        else
          vLedSet(au8Led_clav[i].mode & (uTick),au8Led_clav[i].pio);
      }
    }
    uTick++;    // Ticker increment
  }

  periode ++;
  periode%=C_CLAV_LED_PERIOD;
}


PUBLIC void PBAR_ClignoterLedNFois(uint32 gpio,uint8 n)
{
  PRIVATE uint8 compteur = 0;
  PRIVATE bool_t echange = FALSE;
  PRIVATE bool_t ecartStart = FALSE;
  PRIVATE uint8 crenau = 0;

  if(n)
  {
    if(echange == FALSE && ecartStart == FALSE){
      /* turn LED on */
      vAHI_DioSetOutput(gpio,0);
    }
    else
    {
      /* turn LED off */
      vAHI_DioSetOutput(0,gpio);
    }
    compteur++;
    if(compteur%CLIG_T == 0 && ecartStart == FALSE){
      crenau++;
      //vPrintf("C=%d\n",compteur);
      echange = !echange;
    }

    if(crenau == 2*n){
      ecartStart = TRUE;
      crenau = 0;
      compteur = 0;
    }

    if(ecartStart && (compteur > CLIG_PAUSE)){
      ecartStart = FALSE;
      compteur = 0;
    }
  }
}
