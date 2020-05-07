/*
 *	LCD interface header file
 *	See lcd.c for more info
 */

typedef unsigned char uint8_t;

/* write a byte to the LCD in 4 bit mode */

void lcd_write(unsigned char);

/* Clear and home the LCD */

void lcd_clear(void);

/* write a string of characters to the LCD */

void lcd_puts(const char *s);

/* Go to the specified position */

void lcd_goto(unsigned char pos);

/* intialize the LCD - call before anything else */

void lcd_init(void);

void lcd_putch(char);

/*	Set the cursor position */
#define	lcd_cursor(x)	lcd_write(((x)&0x7F)|0x80)

#define LCD_SETCGRAMADDR 0x40

void lcd_createChar(uint8_t location, uint8_t charmap[]);

