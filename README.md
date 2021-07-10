## STM32 HAl library for LCD1602, 2004 connected by i2s bus

This library allows to work with LCD display through i2c bus.

It uses the delay library DWT_Delay by Khaled Magdy, but you can define your own.

First of all scan the connected devices:
```
//Delay library
#include "DWT_Delay.h"
//Define this to enable scan function
#define USE_I2C_SCAN 
//Include the lib
#include "LCD_i2c.h"

...

int main(void) {
...

  while (1)
  {
    //Scan for devices and put their addresses to UART
    I2C_Scan(&hi2c1, &huart1);
    HAL_Delay(250);
  }
...
```

After you get known the addresses of the devices you can use the library in the following way:
```
//for formatted output
#include <stdio.h>

//Delay library
#include "DWT_Delay.h"
//Include header file
#include "LCD_i2c.h"

//Variable for lcd
LCD_I2C_t lcd;

int main(void) {
  ...
  DWT_Delay_Init(); //Init the delay library
  
  LCD_Init(&lcd, &hi2c1, 0x27, 20, 4);

  LCD_Backlight(&lcd, 1); //Backlight on
  
  LCD_SendString(&lcd, "Hello World!");
  
  char msg[10] = {0}; //buffer
  uint8_t counter = 0;
  
  while (1) {
    LCD_SetCursor(&lcd, 0, 2);
    sprintf(msg, "%d  ", counter++);
    LCD_SendString(&lcd, msg);
    DWT_Delay_ms(250);
  }
 
 ```
 
 This library use DWT_Delay.h by Khaled Magdy description: https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/
 
 This library is based on https://habr.com/ru/post/501006/ and https://github.com/FamNuven/STM32_1602-2004
 
 The initialization of the display decribed here https://radiolaba.ru/microcotrollers/podklyuchenie-lcd-1602-po-i2c-interfeysu.html
