#ifndef STM32_LCD_I2C_H
#define STM32_LCD_I2C_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

/**
 ******************************************************************************
 * @file           : LCD_i2c.h
 * @brief          : HAL library to work with LCD display connected by i2c
 ******************************************************************************
 * @attention
 * Copyright 2021 Konstantin Toporov
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom 
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ********************* Description *************************
 * This library lets work with LCD 1604, 2004 connected to i2c bus.
 * I2C bus must be enabled.
 * Since this library use delays in microsecods, initialization of 
 * the delay library must be done before using this library.
 * 
 * If you want, you can redefine your own functions to 
 * delay micros and milliseconds.
 * 
 * This library use DWT_Delay.h by Khaled Magdy description: 
 * https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/
 * 
 * This library is based on https://habr.com/ru/post/501006/ and
 * https://github.com/FamNuven/STM32_1602-2004
 */

#include "stm32f1xx_hal.h"
#include "DWT_Delay.h"

#ifndef DELAY_MS
#define DELAY_MS(x) DWT_Delay_ms(x)
#endif

#ifndef DELAY_US
#define DELAY_US(x) DWT_Delay_us(x)
#endif

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define BIT_EN 0b00000100  // Enable bit
#define BIT_RW 0b00000010  // Read/Write bit
#define BIT_RS 0b00000001  // Register select bit

/**
 * @brief  LCD_I2C working struct
 * @note   Everything is fully private and should not be touched by user
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t Addr;
    uint8_t displaycontrol;
    uint8_t displaymode;
    uint8_t backlight;
    uint8_t numlines;
    uint8_t cols;
    uint8_t rows;
} LCD_I2C_t;



/* To enable this function define USE_I2C_SCAN 
that's why the body of the function is placed here */
#ifdef USE_I2C_SCAN
#include <string.h>
#include <stdio.h>

/**
 * This function scans the I2C bus and prints found devices to specified UART
 * @param *hi2c1 - handle to i2c
 * @param *huart - handle to uart
 */
void I2C_Scan (I2C_HandleTypeDef *hi2c1, UART_HandleTypeDef *huart)
{
	// создание переменной, содержащей статус
	HAL_StatusTypeDef res;                          
	// сообщение о начале процедуры
	char info[] = "Scanning I2C bus...\r\n";
	// отправка сообщения по UART	
	HAL_UART_Transmit(huart, (uint8_t *)info, strlen(info), HAL_MAX_DELAY);
	
	char msg[10];
	// перебор всех возможных адресов
	for(uint16_t i = 0; i < 128; i++)              
	{
		// проверяем, готово ли устройство по адресу i для связи
		res = HAL_I2C_IsDeviceReady(hi2c1, i << 1, 1, HAL_MAX_DELAY);                   
	    // если да, то
		if(res == HAL_OK)                              
	    {
	    	// запись адреса i, на который откликнулись, в строку в виде 
			// 16тиричного значения:
	    	sprintf(msg, "0x%02X\r\n", i);
	    	// отправка номера откликнувшегося адреса
			HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
	    }
	}
	char finished[] = "Finished\r\n";
	// отправка сообщения по UART	
	HAL_UART_Transmit(huart, (uint8_t *)finished, strlen(finished), HAL_MAX_DELAY);
}
#endif

//Прототипы
///Инициализация дисплея, его очистка
/**
 * @brief Display init
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par addr - address of the display. It could be get in I2C_Scan
 * @par cols - width of the display
 * @par rows - height of the display
 */
void LCD_Init(LCD_I2C_t *lcd_i2c, I2C_HandleTypeDef *, uint8_t addr, uint8_t cols, uint8_t rows);

/**
 * Set cursor to the left top corner.
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 */
void LCD_Home(LCD_I2C_t *lcd_i2c);

/**
 * Clear the display and set cursor to the left top corner. 
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 */
void LCD_Clear(LCD_I2C_t *lcd_i2c);

/**
 * Turn the backlight on and off
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par on - 1 - turn on, 0 - turn off
 */
void LCD_Backlight(LCD_I2C_t *lcd_i2c, uint8_t on);

/**
 * Turn blinking cursor on and off
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par on - 1 - turn on, 0 - turn off
 */ 
void LCD_Blink(LCD_I2C_t *lcd_i2c, uint8_t on);

/**
 * Turn underline cursor on and off
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par on - 1 - turn on, 0 - turn off
 */
void LCD_Cursor(LCD_I2C_t *lcd_i2c, uint8_t on);

/**
 * Set cursor position
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par col - column
 * @par row - row
 */
void LCD_SetCursor(LCD_I2C_t *lcd_i2c, uint8_t col, uint8_t row);

/**
 * Display string
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par str - pointer to string
 */
void LCD_SendString(LCD_I2C_t *lcd_i2c, const char *str);

///don't call it directly
//uint8_t LCD_send(uint8_t data, uint8_t flags);
//uint8_t LCD_command(uint8_t data);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* End of STM32_LCD_I2C_H */