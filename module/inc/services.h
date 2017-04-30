// -----------------------------------
// Projet	: PBPJ1
//	Fic		: services.h
//  Cree	: 30 juil. 2012
//	Par		: Administrateur
// -----------------------------------

// Definitions communes a : Coordonateur, routeur, end device

#ifndef _PBPJ1_MO_SERVICES_H_
#define _PBPJ1_MO_SERVICES_H_

#if defined __cplusplus
extern "C"
{
#endif

  /****************************************************************************/
  /***        Include Files                                                 ***/
  /****************************************************************************/

  /****************************************************************************/
  /***        Macro Definitions                                             ***/
  /****************************************************************************/
#define SRV_INTER    0x00000001
#define SRV_LUMIR    0x00000002
#define SRV_VOLET    0x00000004
#define SRV_MASK	   0x0000000F

#if 0
#define BOITE_ID_01  0x00000010
#define BOITE_ID_02  0x00000020
#define BOITE_ID_03  0x00000040
#define BOITE_ID_04  0x00000080
#define BOITE_ID_05  0x00000100
#define BOITE_ID_06  0x00000200
#define BOITE_ID_07  0x00000400
#define BOITE_ID_08  0x00000800
#define BOITE_ID_09  0x00001000
#define BOITE_ID_10  0x00002000
#define BOITE_ID_11  0x00004000
#define BOITE_ID_12  0x00008000
#define BOITE_MASK   0x000000F0
#endif

#define BOITE_ID_01  0x00000010
#define BOITE_ID_02  0x00000020
#define BOITE_ID_03  0x00000030
#define BOITE_ID_04  0x00000040
#define BOITE_ID_05  0x00000050
#define BOITE_ID_06  0x00000060
#define BOITE_ID_07  0x00000070
#define BOITE_ID_08  0x00000080
#define BOITE_ID_09  0x00000090
#define BOITE_ID_10  0x000000A0
#define BOITE_ID_11  0x000000B0
#define BOITE_ID_12  0x000000C0
#define BOITE_MASK   0x000000F0
#define BOITE_MAXI	 15

  /****************************************************************************/
  /***        Type Definitions                                              ***/
  /****************************************************************************/

  /****************************************************************************/
  /***        Exported Functions                                            ***/
  /****************************************************************************/

  /****************************************************************************/
  /***        Exported Variables                                            ***/
  /****************************************************************************/

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_MO_SERVICES_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
