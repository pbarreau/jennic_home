/****************************************************************************
 * $Rev::                   $: Revision of last commit
 * $Author::                $: Author of last commit
 * $Date::                  $: Date of last commit
 * $HeadURL:                $
 ****************************************************************************
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2010. All rights reserved
 ****************************************************************************/
#ifndef  INTR_H_INCLUDED
#define  INTR_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Interrupt control macros. */
#define ENABLE_INTERRUPTS();                                        \
{                                                                   \
    register uint32 ru32CtrlReg;                                    \
    asm volatile ("l.mfspr %0, r0, 17;" :"=r"(ru32CtrlReg) : );     \
    ru32CtrlReg |= 6;                                               \
    asm volatile ("l.mtspr r0, %0, 17;" : :"r"(ru32CtrlReg));       \
}

#define DISABLE_INTERRUPTS();                                       \
{                                                                   \
    register uint32 ru32CtrlReg;                                    \
    asm volatile ("l.mfspr %0, r0, 17;" :"=r"(ru32CtrlReg) : );     \
    ru32CtrlReg &= 0xfffffff9;                                      \
    asm volatile ("l.mtspr r0, %0, 17;" : :"r"(ru32CtrlReg));       \
}

#define MICRO_RESTORE_INTERRUPTS(u32Store);                         \
{                                                                   \
    asm volatile ("l.mtspr r0, %0, 17;" : :"r"(u32Store));          \
}

#define MICRO_DISABLE_AND_SAVE_INTERRUPTS(u32Store);                \
{                                                                   \
    register uint32 ruCtrlReg;                                      \
    asm volatile ("l.mfspr %0, r0, 17;" :"=r"(u32Store) : );        \
    ruCtrlReg = u32Store & 0xfffffffb;                              \
    asm volatile ("l.mtspr r0, %0, 17;" : :"r"(ruCtrlReg));         \
}

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

#endif  /* INTR_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
