
#include	"main.h"
#include	"44780.h"
#include	<util\delay.h>
#include	<avr\interrupt.h>

void lcd_write4(char c){
	c&=0x0f;
	LCD_DATA_PORT=(LCD_DATA_PORT&~(0x0f<<LCD_DATA_SHIFT))|(c<<LCD_DATA_SHIFT);
	LCD_E(1);
	LCD_E(1);
	LCD_E(1);
	LCD_E(0);
	_delay_us(1);
	}

void lcd_write_c(char c){
	LCD_RS(0);
	lcd_write4(c>>4);
	lcd_write4(c);
	_delay_us(50);
	}

void lcd_write_d(char c){
	LCD_RS(1);
	lcd_write4(c>>4);
	lcd_write4(c);
	_delay_us(50);
	}

void lcd_init(){
	_delay_ms(20);
	lcd_write4(3);
	_delay_ms(5);
	lcd_write4(3);
	_delay_us(200);
	lcd_write4(3);
	_delay_us(200);
	lcd_write4(2);
	_delay_us(200);
	lcd_write_c(0b00101000);
	lcd_write_c(0b00001100);	//bit 1	cursor
	lcd_write_c(0b00000001);
	lcd_write_c(0b00000010);
	_delay_ms(2);
	}

void lcd_str(char *p){
	while(*p)
		lcd_write_d(*p++);
	}

void lcd_pos(char p){
	lcd_write_c(p|0x80);
	}

void lcd_clr(void){
	lcd_write_c(1);
	}