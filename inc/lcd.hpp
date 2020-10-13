#ifndef LCD_HPP
#define LCD_HPP

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

// Define some device parameters
#define I2C_ADDR                    0x27

#define ENABLE                      0b00000100
#define LCD_BACKLIGHT               0x08

// Linhas
#define LINE1                       0x80
#define LINE2                       0xC0

// Define some device constants
#define LCD_CHR                     1
#define LCD_CMD                     0

void lcd_toggle_enable(int fd, int bits);
void lcd_byte(int fd, int bits, int mode);
void lcdLoc(int fd, int line);
void typeln(int fd, const char *s);
void lcd_init(int fd);
void ClrLcd(int fd);

#endif // LCD_HPP
