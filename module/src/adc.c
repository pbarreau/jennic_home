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
/** @file
 * ADC Interface
 *
 * @defgroup grp_ADC ADC Interface Utilities
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi.h>

#include "adc.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Conversion factor for ADC based battery voltage measurement. Within the
   JN5139 the supply voltage (VDD1) is divided down by a factor of 0.666
   before being fed into the ADC. When measuring the supply voltage the ADC
   output is a 12-bit value which represents the range 0 - 2.4V (2 * Vfef)
   i.e. 1 bit = 585.93uV of input voltage. Therefore for 1mV change in VDD1
   there is a 666.66uV change in ADC input voltage which corresponds to a
   666.66uV/585.83uV = 1.14 bit change in ADC value. */
#define ADC_BITS_PER_BATT_mV    1.14F

/* Conversion factor for ADC based temperature measurement. The temperature
   sensor within the JN5139 has a typical gain of 1.6mV per degree C. When
   measuring the temperature the ADC output is a 12-bit value which represents
   the range 0 - 1.2V (Vfef) i.e. 1 bit = 292.97uV of input voltage. Therefore
   1 degree C change = 1.6mV / 292.97uV = 5.46 bit change in ADC value. */
#define ADC_BITS_PER_TEMP_C     5.46F

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/* Store the ADC count that we should expect at 25C. We intialise this with
   a defualt value but this can be updated if we choose to calibrate the temp
   sensor */
PRIVATE uint16 u16TempSensor25COffset = ADC_DEFAULT_25C_OFFSET;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/**
 * Initialise ADC
 *
 */
PUBLIC void vADC_Init(void)
{
    /* Configure analogue peripherals */
    vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE,
                     E_AHI_AP_INT_ENABLE,
                     E_AHI_AP_SAMPLE_2,
                     E_AHI_AP_CLOCKDIV_500KHZ,
                     E_AHI_AP_INTREF);

    /* Wait for regulator to come up */
    while(!bAHI_APRegulatorEnabled());
}

/**
 * Starts an ADC conversion on the requested input. The range used is
 * changed depeding upon the input to be read.
 *
 * @param 8-bit Input number (i.e. temp sensor or battery voltage monitor).
 *
 */
PUBLIC void vADC_StartConversion(uint8 u8Input)
{
    /* Enable a one-shot ADC conversion for the selected input */
    if (u8Input == E_AHI_ADC_SRC_TEMP)
    {
        /* Use 0-Vref range for temp sensor as we know it will not
           exceed this */
        vAHI_AdcEnable(E_AHI_ADC_CONVERT_DISABLE,
                       E_AHI_AP_INPUT_RANGE_1,
                       u8Input);
    }
    else
    {
        /* Use 0-2*Vref range for battery measurement as the input
           may exceed Vref */
        vAHI_AdcEnable(E_AHI_ADC_CONVERT_DISABLE,
                       E_AHI_AP_INPUT_RANGE_2,
                       u8Input);
    }
    /* Start the ADC conversion */
    vAHI_AdcStartSample();
}

/**
 * Reads supply voltage (VDD1) via the ADC and converts the result to
 * milli-volts.
 *
 * @return 16-bit Measured battery voltage in milli-volts
 *
 */
PUBLIC uint16 u16ADC_ReadBattVolt(void)
{
    /* Read ADC value */
    uint16 u16AdcValue = u16AHI_AdcRead();

    /* Convert to battery voltage */
    float fBattVoltage =  (float)u16AdcValue / ADC_BITS_PER_BATT_mV;

    return((uint16)fBattVoltage);
}

/**
 * Reads temperature sensor via ADC and converts result to degrees C based
 * upon a fixed sensor gain (ADC_BITS_PER_TEMP_C) and an output at a known
 * temperature (u16TempSensor25COffset).
 *
 * @return 8-bit Measured temperature in degrees C (-127C to +127C)
 *
 */
PUBLIC int8 i8ADC_ReadTemperature(void)
{
    /* Read ADC value */
    uint16 u16AdcValue = u16AHI_AdcRead();

    /* Convert to temperature in degrees C */
    float fTemperatureC = 25 - (((int16)(u16AdcValue - u16TempSensor25COffset)) / ADC_BITS_PER_TEMP_C);

    return ((int8)fTemperatureC);
}

/**
 * Reads current ADC value from internal temperature sensor. If device is
 * held at a known temperature (25C in this application) the value returned
 * by this function can be stored as an offset and used to perform a 1-point
 * calibration of the temperature sensor.
 *
 * @return 16-bit ADC output from temperature sensor (12LSB only)
 *
 */
PUBLIC uint16 u16ADC_MeasureTempSensorOffset(void)
{
    vAHI_AdcEnable(E_AHI_ADC_CONVERT_DISABLE,
                   E_AHI_AP_INPUT_RANGE_1,
                   E_AHI_ADC_SRC_TEMP);

    /* Start the ADC conversion */
    vAHI_AdcStartSample();

    /* Wait for ADC to complete */
    while (bAHI_AdcPoll());

    /* Read ADC value */
    return (u16AHI_AdcRead());
}

/**
 * Sets ADC output from temp sensor that is to be expected at 25C.
 *
 * @param 16-bit Value corresponding to the ADC output from the temperature
 *        sensor at 25C. This allows us to perform a simple 1-point calibration
 *        of the internal temperature sensor.
 *
 */
PUBLIC void vADC_SetTempSensor25COffset(uint16 u16OffSet)
{
    u16TempSensor25COffset = u16OffSet;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
