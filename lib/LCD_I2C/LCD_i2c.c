#include "LCD_i2c.h"


//Prototypes
uint8_t LCD_command(LCD_I2C_t *lcd_i2c, uint8_t command);
uint8_t LCD_send(LCD_I2C_t *lcd_i2c, uint8_t data, uint8_t flags);

void LCD_Init(LCD_I2C_t *lcd_i2c, I2C_HandleTypeDef *handle, uint8_t addr, uint8_t cols, uint8_t rows)
{
    lcd_i2c->hi2c = handle;

	lcd_i2c->Addr = (addr << 1); //correct address
  	lcd_i2c->cols = cols;
  	lcd_i2c->rows = rows;
	lcd_i2c->numlines = rows;
  	lcd_i2c->backlight = LCD_NOBACKLIGHT;
	lcd_i2c->displaycontrol = LCD_CURSOROFF|LCD_BLINKOFF;
	
	DELAY_MS(50);  // wait for >40ms
	
	LCD_command(lcd_i2c, LCD_FUNCTIONSET|LCD_8BITMODE);   // 8ми битный интерфейс
  	DELAY_MS(5);
	
	LCD_command(lcd_i2c, LCD_FUNCTIONSET|LCD_8BITMODE);   // 8ми битный интерфейс
  	DELAY_MS(120);
	
	LCD_command(lcd_i2c, LCD_FUNCTIONSET|LCD_4BITMODE);   // 4ми битный интерфейс
	
	LCD_command(lcd_i2c, LCD_FUNCTIONSET|LCD_4BITMODE|LCD_5x8DOTS|LCD_2LINE);

	LCD_command(lcd_i2c, LCD_DISPLAYCONTROL|LCD_DISPLAYOFF|LCD_CURSOROFF|LCD_BLINKOFF);

	LCD_command(lcd_i2c, LCD_CLEARDISPLAY);   // очистка дисплея
	DELAY_MS(2);

	LCD_command(lcd_i2c, LCD_ENTRYMODESET|LCD_ENTRYLEFT|LCD_ENTRYSHIFTDECREMENT);

	LCD_command(lcd_i2c, LCD_DISPLAYCONTROL|LCD_DISPLAYON|lcd_i2c->displaycontrol);

	//https://radiolaba.ru/microcotrollers/podklyuchenie-lcd-1602-po-i2c-interfeysu.html
}

void LCD_Home(LCD_I2C_t *lcd_i2c)
{
	LCD_command(lcd_i2c, LCD_RETURNHOME);   // установка курсора в начале строки
	DELAY_MS(2);
}

void LCD_Clear(LCD_I2C_t *lcd_i2c)
{
	// очистка дисплея
	LCD_command(lcd_i2c, LCD_CLEARDISPLAY);
	DELAY_MS(2);
}

void LCD_Backlight(LCD_I2C_t *lcd_i2c, uint8_t backlight)
{
	if (backlight) lcd_i2c->backlight = LCD_BACKLIGHT;	
	else lcd_i2c->backlight = 0;
	LCD_command(lcd_i2c, 0);
}

void LCD_Blink(LCD_I2C_t *lcd_i2c, uint8_t on)
{
	if (on) {
		lcd_i2c->displaycontrol |= LCD_BLINKON;
	} else {
		lcd_i2c->displaycontrol &= ~LCD_BLINKON;
	}
	LCD_command(lcd_i2c, LCD_DISPLAYCONTROL|LCD_DISPLAYON|lcd_i2c->displaycontrol);
}

void LCD_Cursor(LCD_I2C_t *lcd_i2c, uint8_t cur)
{
	if (cur) {
		lcd_i2c->displaycontrol |= LCD_CURSORON;
	} else {
		lcd_i2c->displaycontrol &= ~LCD_CURSORON;
	}
	LCD_command(lcd_i2c, LCD_DISPLAYCONTROL|LCD_DISPLAYON|lcd_i2c->displaycontrol);
}

void LCD_SetCursor(LCD_I2C_t *lcd_i2c, uint8_t col, uint8_t row)
{
	if ((row+1)>lcd_i2c->rows) return;

	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > (4-1) ) {
		row = 4 - 1;    // we count rows starting w/0
	}
	LCD_command(lcd_i2c, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD_SendString(LCD_I2C_t *lcd_i2c, const char *str)
{
	// *char по сути является строкой
	while(*str && LCD_send(lcd_i2c, (uint8_t)(*str), 1)) {
        // пока строчка не закончится
		// передача первого символа строки
        str++; // сдвиг строки налево на 1 символ
    }
}

/***** Internal communication functions ******/

/**
 * Send command to display
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par command  - command
 * @retval 1 - success, 0 - error divice not responding
 */
uint8_t LCD_command(LCD_I2C_t *lcd_i2c, uint8_t command)
{
	return LCD_send(lcd_i2c, command, 0);
}

/**
 * Send data to display
 * @par *lcd_i2c - Pointer to @ref LCD_I2C_t working lcd_i2c struct
 * @par data - datas to send
 * @par flags - 1 - send datas or 0 - send command
 * @return 1 - success, 0 - error divice not responding
 */
uint8_t LCD_send(LCD_I2C_t *lcd_i2c, uint8_t data, uint8_t flags)
{
	if (lcd_i2c->hi2c == NULL) return 0;

	HAL_StatusTypeDef res;
	
	//Is the device ready by lcd_i2c->Addr?
	//Trials - 10 times
	res = HAL_I2C_IsDeviceReady(lcd_i2c->hi2c, lcd_i2c->Addr, 10, HAL_MAX_DELAY);          
	//Failed, nobody answered
	if(res != HAL_OK) return 0;                                                  
	
	//Split byte to upper and lower parts
	uint8_t up = data & 0xF0; //upper part     
	uint8_t lo = (data << 4) & 0xF0; //lower part
		                                           
	uint8_t data_arr[4];
	// 4-7 bits contains info, bits 0-3 contains configuration
	data_arr[0] = up|flags|lcd_i2c->backlight|BIT_EN;  
	// дублирование сигнала, на выводе Е в этот раз 0
	// send again, this tine EN is zero
	data_arr[1] = up|flags|lcd_i2c->backlight;
	//The same for configuration
	data_arr[2] = lo|flags|lcd_i2c->backlight|BIT_EN;
	data_arr[3] = lo|flags|lcd_i2c->backlight;

	res = HAL_I2C_Master_Transmit(lcd_i2c->hi2c, lcd_i2c->Addr, data_arr, sizeof(data_arr), HAL_MAX_DELAY);
	
	DELAY_MS(5);
	
	if (res == HAL_OK) return 1;
	else return 0;
}
