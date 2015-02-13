// -----------------------------------
// Projet	: PBPJ1
//	Fic		: led.c
//  Cree	: 18 août 2012
//	Par		: Administrateur
// -----------------------------------

#include <jendefs.h>
//#include <JPI_JN514x.h>
#include <JPI.h>
#include <Printf.h>

#include "m_config.h"

#define CLIG_T			      30
#define	 CLIG_E			      (2 * CLIG_T)
#define CLIG_PAUSE	   50

PUBLIC bool_t bStartFlashing = FALSE;
PUBLIC uint32 mLedId =0;
PUBLIC sbpLed au8Led[2];

PUBLIC  void PBAR_ClignoteLed_1(void)
{
 PRIVATE uint8 uTick = 0;
 PRIVATE uint8 periode = 0;
 uint8 i;


 if(!periode){
   for(i=0;i<sizeof(au8Led)/sizeof(sbpLed);i++){
     if(au8Led[i].actif == TRUE){

       // Set LED according to status
       if(au8Led[i].mode == 0 || au8Led[i].mode == 0xFF)
        vLedCtrlKpd(au8Led[i].mode,au8Led[i].pio);
       else
        vLedCtrlKpd(au8Led[i].mode & (uTick),au8Led[i].pio);
     }
   }
   uTick++;    // Ticker increment
 }
 periode ++;
 periode%=5;

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
