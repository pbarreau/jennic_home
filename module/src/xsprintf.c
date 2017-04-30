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
#if !NO_DEBUG_ON
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdarg.h>

#include "jendefs.h"
#include "xsprintf.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PAD_RIGHT       1
#define PAD_ZERO        2
#define PRINT_BUF_LEN   12 /* Should be enough for 32 bit int */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void printchar(char **str, int c);
static int print(char **out, const char *format, va_list arguments);
static int prints(char **out, const char *string, int width, int pad);
static int printi(char **out, int i, int b, int sg, int width, int pad,
    int letbase);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC int32 xsprintf(char *out, const char *format, ...)
{
  va_list arguments;
  va_start(arguments, format);

  return (print(&out, format, arguments));
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC int snprintf(char *str, size_t size, const char *format, ...)
{
  va_list arguments;
  va_start(arguments, format);

  return (print(&str, format, arguments));
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
static void printchar(char **str, int c)
{
  if (str)
  {
    **str = c;
    ++(*str);
  }
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
static int print(char **out, const char *format, va_list arguments)
{
  int width, pad;
  int pc = 0;
  char scr[2];

  for (; *format != 0; ++format)
  {
    if (*format == '%')
    {
      ++format;
      width = pad = 0;

      if (*format == '\0')
        break;

      if (*format == '%')
        goto out;

      if (*format == '-')
      {
        ++format;
        pad = PAD_RIGHT;
      }

      while (*format == '0')
      {
        ++format;
        pad |= PAD_ZERO;
      }
      for (; *format >= '0' && *format <= '9'; ++format)
      {
        width *= 10;
        width += *format - '0';
      }
      if (*format == 's')
      {
        char *s = (char *) va_arg(arguments, uint32);
        pc += prints(out, s ? s : "(null)", width, pad);
        continue;
      }
      if (*format == 'd')
      {
        pc += printi(out, va_arg(arguments, uint32), 10, 1, width, pad, 'a');
        continue;
      }
      if (*format == 'x')
      {
        pc += printi(out, va_arg(arguments, uint32), 16, 0, width, pad, 'a');
        continue;
      }
      if (*format == 'X')
      {
        pc += printi(out, va_arg(arguments, uint32), 16, 0, width, pad, 'A');
        continue;
      }
      if (*format == 'u')
      {
        pc += printi(out, va_arg(arguments, uint32), 10, 0, width, pad, 'a');
        continue;
      }
      if (*format == 'c')
      {
        scr[0] = va_arg(arguments, uint32);
        scr[1] = '\0';
        pc += prints(out, scr, width, pad);
        continue;
      }
    }
    else
    {
      out: printchar(out, *format);
      ++pc;
    }
  }
  if (out)
  {
    **out = '\0';
  }
  return pc;
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
static int printi(char **out, int i, int b, int sg, int width, int pad,
    int letbase)
{
  char print_buf[PRINT_BUF_LEN];
  register char *s;
  register int t, neg = 0, pc = 0;
  register unsigned int u = i;

  if (i == 0)
  {
    print_buf[0] = '0';
    print_buf[1] = '\0';
    return prints(out, print_buf, width, pad);
  }

  if (sg && b == 10 && i < 0)
  {
    neg = 1;
    u = -i;
  }

  s = print_buf + PRINT_BUF_LEN - 1;
  *s = '\0';

  while (u)
  {
    t = u % b;
    if (t >= 10)
      t += letbase - '0' - 10;
    *--s = t + '0';
    u /= b;
  }

  if (neg)
  {
    if (width && (pad & PAD_ZERO))
    {
      printchar(out, '-');
      ++pc;
      --width;
    }
    else
    {
      *--s = '-';
    }
  }
  return pc + prints(out, s, width, pad);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 ****************************************************************************/
static int prints(char **out, const char *string, int width, int pad)
{
  int pc = 0, padchar = ' ';

  if (width > 0)
  {
    int len = 0;
    const char *ptr;

    for (ptr = string; *ptr; ++ptr)
    {
      ++len;
    }

    if (len >= width)
    {
      width = 0;
    }
    else
    {
      width -= len;
    }

    if (pad & PAD_ZERO)
    {
      padchar = '0';
    }
  }
  if (!(pad & PAD_RIGHT))
  {
    for (; width > 0; --width)
    {
      printchar(out, padchar);
      ++pc;
    }
  }
  for (; *string; ++string)
  {
    printchar(out, *string);
    ++pc;
  }
  for (; width > 0; --width)
  {
    printchar(out, padchar);
    ++pc;
  }
  return pc;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
#endif // #if !NO_DEBUG_ON
