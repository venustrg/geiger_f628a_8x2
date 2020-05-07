/*
 *	LCD interface example
 *	Uses routines from delay.c
 *	This code will interface to a standard LCD controller
 *	like the Hitachi HD44780. It uses it in 4 bit mode, with
 *	the hardware connected as follows (the standard 14 pin 
 *	LCD connector is used):
 *	
 *	PORTB bits 4-7 are connected to the LCD data bits 4-7 (high nibble)
 *	PORTA bit 0 is connected to the LCD RS input (register select)
 *	PORTA bit 1 is connected to the LCD EN bit (enable)
 *                                the LCD R/W to GND
 *	
 *	To use these routines, set up the port I/O (TRISA, TRISB) then
 *	call lcd_init(), then other routines as required.
 *	
 */

#include	<pic.h>
#include	"lcd.h"
#include	"delay.h"

static bit LCD_RS @ ((unsigned) &PORTA * 8);    // Register select
static bit LCD_EN @ ((unsigned) &PORTA * 8 + 1);        // Enable

#define	LCD_STROBE	((LCD_EN = 1),(LCD_EN=0))


/* write a byte to the LCD in 4 bit mode */

void lcd_write(unsigned char c)
{
    PORTB = (PORTB & 0x0F) | (c & 0xF0);
    LCD_STROBE;
    PORTB = (PORTB & 0x0F) | (c << 4);
    LCD_STROBE;
    DelayUs(40);
}

/*
 * 	Clear and home the LCD
 */

void lcd_clear(void)
{
    LCD_RS = 0;
    lcd_write(0x1);
    DelayMs(2);
}

/* write a string of chars to the LCD */

void lcd_puts(const char *s)
{
    LCD_RS = 1;                        // write characters
    while (*s)
        lcd_write(*s++);
}

/* write one character to the LCD */

void lcd_putch(char c)
{
    LCD_RS = 1;                        // write characters
    lcd_write(c);
}

/*
 * Go to the specified position
 */

void lcd_goto(unsigned char pos)
{
    LCD_RS = 0;
    lcd_write(0x80 + pos);
}

/* initialise the LCD - put into 4 bit mode */

void lcd_init(void)
{
    LCD_RS = 0;                        // write control bytes
    DelayMs(50);                       // power on delay 15..150
    PORTB = 0x30;                      // attention! 3
    LCD_STROBE;
    DelayMs(5);
    LCD_STROBE;
    DelayUs(100);
    LCD_STROBE;
    DelayMs(5);
    PORTB = 0x20;                      // set 4 bit mode 2
    LCD_STROBE;
    DelayUs(40);
    lcd_write(0x28);                   // 4 bit mode, 1/16 duty, 5x8 font
    lcd_write(0x08);                   // display off
    lcd_write(0x0C);                   // display on, blink curson off (F)
    lcd_write(0x06);                   // entry mode
}

void lcd_createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7;                   // we only have 8 locations 0-7
    LCD_RS = 0;
    lcd_write(LCD_SETCGRAMADDR | (location << 3));
    for (uint8_t i = 0; i < 8; i++) {
        LCD_RS = 1;
        lcd_write(charmap[i]);
    }
}
