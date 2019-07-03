//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SHT3x Sample Code (V1.1)
// File      :  main.c (V1.1)
// Author    :  RFU
// Date      :  6-Mai-2015
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  This code shows how to implement the basic commands for the
//              SHT3x sensor chip.
//              Due to compatibility reasons the I2C interface is implemented
//              as "bit-banging" on normal I/O's. This code is written for an
//              easy understanding and is neither optimized for speed nor code
//              size.
//
// Porting to a different microcontroller (uC):
//   - the definitions of basic types may have to be changed  in typedefs.h
//   - adapt the button and led functions for your platform   in main.c
//   - adapt the port functions / definitions for your uC     in i2c_hal.h/.c
//   - adapt the timing of the delay function for your uC     in system.c
//   - adapt the SystemInit()                                 in system.c
//   - change the uC register definition file <stm32f10x.h>   in system.h
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "system.h"
#include "sht3x.h"

//-- Static function prototypes -----------------------------------------------
static void EvalBoardPower_Init(void);
static void Led_Init(void);
static void UserButton_Init(void);
static void LedBlueOn(void);
static void LedBlueOff(void);
static void LedGreenOn(void);
static void LedGreenOff(void);
static u8t ReadUserButton(void);

//-----------------------------------------------------------------------------
int main(void)
{
  etError   error;       // error code
  u32t      serialNumber;// serial number
  regStatus status;      // sensor status
  ft        temperature; // temperature [°C]
  ft        humidity;    // relative humidity [%RH]
  bt        heater;      // heater, false: off, true: on
  
  SystemInit();
  Led_Init();
  UserButton_Init();
  EvalBoardPower_Init();
  
  SHT3X_Init(0x45); // Address: 0x44 = Sensor on EvalBoard connector
                    //          0x45 = Sensor on EvalBoard
  
  // wait 50ms after power on
  DelayMicroSeconds(50000);    
  
  error = SHT3x_ReadSerialNumber(&serialNumber);
  if(error != NO_ERROR){} // do error handling here
  
  // demonstrate a single shot measurement with clock-stretching
  error = SHT3X_GetTempAndHumi(&temperature, &humidity, REPEATAB_HIGH, MODE_CLKSTRETCH, 50);
  if(error != NO_ERROR){} // do error handling here 
  
  // demonstrate a single shot measurement with polling and 50ms timeout
  error = SHT3X_GetTempAndHumi(&temperature, &humidity, REPEATAB_HIGH, MODE_POLLING, 50);
  if(error != NO_ERROR){} // do error handling here 
  
  // loop forever
  while(1)
  {
    error = NO_ERROR;
    
    // loop while no error
    while(error == NO_ERROR)
    {
      // read status register
      error |= SHT3X_ReadStatus(&status.u16);
      if(error != NO_ERROR) break;
      
      // check if the reset bit is set after a reset or power-up
      if(status.bit.ResetDetected)
      {
        //override default temperature and humidity alert limits (red LED)
        error = SHT3X_SetAlertLimits( 70.0f,  50.0f,  // high set:   RH [%], T [°C]
                                      68.0f,  48.0f,  // high clear: RH [%], T [°C]
                                      32.0f,  -2.0f,  // low clear:  RH [%], T [°C]
                                      30.0f,  -4.0f); // low set:    RH [%], T [°C]
		    if(error != NO_ERROR) break;
		
        
        // clear reset and alert flags
        error = SHT3X_ClearAllAlertFlags();
        if(error != NO_ERROR) break;
        
        //start periodic measurement, with high repeatability and 1 measurements per second
        error = SHT3X_StartPeriodicMeasurment(REPEATAB_HIGH, FREQUENCY_1HZ);
        if(error != NO_ERROR) break;
        
        //switch green LED on
        LedGreenOn();
      }
        
      // read measurment buffer
      error = SHT3X_ReadMeasurementBuffer(&temperature, &humidity);
      if(error == NO_ERROR)
      {
        // flash blue LED to signalise new temperature and humidity values
        LedBlueOn();
        DelayMicroSeconds(10000);
        LedBlueOff();
      }
      else if (error == ACK_ERROR)
      {
        // there were no new values in the buffer -> ignore this error
        error = NO_ERROR;
      }
      else break;
      
      // read heater status
      heater = status.bit.HeaterStatus ? TRUE : FALSE;
      
      // if the user button is not pressed ...
      if(ReadUserButton() == 0)
      { 
         // ... and the heater is on
         if(heater)
         {
           // switch off the sensor internal heater
           error |= SHT3X_DisableHeater();
           if(error != NO_ERROR) break;
         }
      }
      else
      // if the user button is pressed ...
      {
         // ... and the heater is off
         if(!heater)
         {
           // switch on the sensor internal heater
           error |= SHT3X_EnableHeater();
           if(error != NO_ERROR) break;
         }
      }
      
      // wait 100ms
      DelayMicroSeconds(100000);
    }
    
    // in case of an error ...
    
    // ... switch green and blue LED off
    LedGreenOff();
    LedBlueOff();
    
    // ... try first a soft reset ...
    error = SHT3X_SoftReset();
    
    // ... if the soft reset fails, do a hard reset
    if(error != NO_ERROR)
    {
      SHT3X_HardReset();
    }
    
    // flash green LED to signalise an error
    LedGreenOn();
    DelayMicroSeconds(10000);
    LedGreenOff();
  }
}

//-----------------------------------------------------------------------------
static void EvalBoardPower_Init(void)    /* -- adapt this code for your platform -- */
{
  RCC->APB2ENR |= 0x00000008;  // I/O port B clock enabled
  
  GPIOB->CRH   &= 0x0FFF0FFF;  // set push-pull output for Vdd & GND pins
  GPIOB->CRH   |= 0x10001000;  //
  
  GPIOB->BSRR = 0x08008000;    // set Vdd to High, set GND to Low
}

//-----------------------------------------------------------------------------
static void Led_Init(void)               /* -- adapt this code for your platform -- */
{
  RCC->APB2ENR |= 0x00000010;  // I/O port C clock enabled
  GPIOC->CRH   &= 0xFFFFFF00;  // set general purpose output mode for LEDs
  GPIOC->CRH   |= 0x00000011;  //
  GPIOC->BSRR   = 0x03000000;  // LEDs off
}

//-----------------------------------------------------------------------------
static void UserButton_Init(void)        /* -- adapt this code for your platform -- */
{
  RCC->APB2ENR |= 0x00000004;  // I/O port A clock enabled
  GPIOA->CRH   &= 0xFFFFFFF0;  // set general purpose input mode for User Button
  GPIOA->CRH   |= 0x00000004;  //
}

//-----------------------------------------------------------------------------
static void LedBlueOn(void)              /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x00000100;
}

//-----------------------------------------------------------------------------
static void LedBlueOff(void)             /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x01000000;
}

//-----------------------------------------------------------------------------
static void LedGreenOn(void)             /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x00000200;
}

//-----------------------------------------------------------------------------
static void LedGreenOff(void)            /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x02000000;
}

//-----------------------------------------------------------------------------
static u8t ReadUserButton(void)          /* -- adapt this code for your platform -- */
{
  return (GPIOA->IDR & 0x00000001);
}
