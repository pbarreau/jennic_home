/*
 * bit.h
 *
 *  Created on: 1 juil. 2013
 *      Author: RapidOs
 */

#ifndef BIT_H_
#define BIT_H_

/* Attention : not portable if ulong is > 32-bit */
#define BITS_NB_MAX 32

/* Attention, type 'ulong' */
#define BIT(bit) (1UL<<(bit))

#define bit31 BIT(31)           /* 0x80000000UL */
#define bit30 BIT(30)           /* 0x40000000UL */
#define bit29 BIT(29)           /* 0x20000000UL */
#define bit28 BIT(28)           /* 0x10000000UL */
#define bit27 BIT(27)           /* 0x08000000UL */
#define bit26 BIT(26)           /* 0x04000000UL */
#define bit25 BIT(25)           /* 0x02000000UL */
#define bit24 BIT(24)           /* 0x01000000UL */
#define bit23 BIT(23)           /* 0x00800000UL */
#define bit22 BIT(22)           /* 0x00400000UL */
#define bit21 BIT(21)           /* 0x00200000UL */
#define bit20 BIT(20)           /* 0x00100000UL */
#define bit19 BIT(19)           /* 0x00080000UL */
#define bit18 BIT(18)           /* 0x00040000UL */
#define bit17 BIT(17)           /* 0x00020000UL */
#define bit16 BIT(16)           /* 0x00010000UL */

#define bit15 BIT(15)           /* 0x00008000U  */
#define bit14 BIT(14)           /* 0x00004000   */
#define bit13 BIT(13)           /* 0x00002000   */
#define bit12 BIT(12)           /* 0x00001000   */
#define bit11 BIT(11)           /* 0x00000800   */
#define bit10 BIT(10)           /* 0x00000400   */
#define bit9  BIT(9 )           /* 0x00000200   */
#define bit8  BIT(8 )           /* 0x00000100   */
#define bit7  BIT(7 )           /* 0x00000080   */
#define bit6  BIT(6 )           /* 0x00000040   */
#define bit5  BIT(5 )           /* 0x00000020   */
#define bit4  BIT(4 )           /* 0x00000010   */
#define bit3  BIT(3 )           /* 0x00000008   */
#define bit2  BIT(2 )           /* 0x00000004   */
#define bit1  BIT(1 )           /* 0x00000002   */
#define bit0  BIT(0 )           /* 0x00000001   */

// Manipulation de Bit
#define Bit(bit) 						(1UL << (bit))
#define BitNset(arg,bit) 				((arg) |= BIT(bit))
#define BitNclr(arg,bit) 				((arg) &= ~BIT(bit))
#define IsBitSet(arg,bit)				(((arg>>bit)&1)?1:0)

#ifdef HOWTO
    int setBit(int x, unsigned char position)
 {
  int mask = 1 << position;
  return x | mask;
 }

 int clearBit(int x, unsigned char position)
 {
  int mask = 1 << position;
  return x & ~mask;
 }

 int modifyBit(int x, unsigned char position, bool newState)
 {
  nt mask = 1 << position;
  int state = int(newState); // relies on true = 1 and false = 0
  return (x & ~mask) | (-state & mask);
 }

 int flipBit(int x, unsigned char position)
 {
  int mask = 1 << position;
  return x ^ mask;
 }

 bool isBitSet(int x, unsigned char position)
 {
  x >>= position;
  return (x & 1) != 0;
 }
#endif

#endif /* BIT_H_ */
