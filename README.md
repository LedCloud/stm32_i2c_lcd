## STM32 HAl library for LCD1602, 2004 connected by i2s bus

This libarry allows to work with LCD display through i2c bus.

It uses the delay library DWT_Delay by Khaled Magdy, you can define your own.

First of all scan the connected devices:
```
//Delay library
#include "DWT_Delay.h"
//Define this to use scan function
#define USE_I2C_SCAN 
//Include the lib
#include "LCD_i2c.h"

int main(void) {
...

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //Scan for devices and put their addresses to UART
    I2C_Scan(&hi2c1, &huart1);
    HAL_Delay(250);
  }
...
```

After you can use the library in the following way:
```
#include <stdio.h>

//#include "DWT_Delay.h"

#define DELAY_MS(x) HAL_Delay(x)
#include "LCD_i2c.h"
