/*
GEIGER COUNTER final 1.0 beta
17.06.2009
(c) TOTHEMA software, 2009
(x) mod 1.0 by venus, 2020
*/

#include <htc.h>
#include "lcd.h"

__CONFIG(FOSC_INTOSCIO & WDTE_OFF & PWRTE_OFF & MCLRE_ON & BOREN_OFF & LVP_OFF & CPD_OFF & CP_OFF);

#define LIGHT RA2=1
#define NOLIGHT RA2=0
#define SCR_WIDTH 8
#define GEIGER_TIME 40
#define TICK_LEN 6              // sound tick len *4ms

const char uR[] = " uR";
const char mR[] = " mR";

bit c = 0;
unsigned long int data, disp;
volatile unsigned int count = 0, poisk = 0;
volatile unsigned long int data2, result = 0;

typedef unsigned char uint8_t;

const uint8_t light_time = 3;
uint8_t light, delay = 0, old_sek;
volatile uint8_t sek;

#define BOOST_MAX_IDLE  5       // periodical boost (sec), just in case
#define BOOST_PACKET_LEN 60     // boost packet length (in 4msec units)

volatile bit boost;             // bust running
volatile bit boost_perm;        // bust running permanently
volatile uint8_t boost_timeout = 0;     // boost packet len (x 4ms)
volatile uint8_t boost_idle = 0;        // periodic boost when no geiger pulses
volatile bit boost_pulse;

volatile bit sound = 0;
volatile uint8_t sound_timeout = 0;

char text[] = "    ";
char *q;
uint8_t i;

// fb - 29/14.5, fa - 27.6/13.8, f4 = 20.6/10.3, f3 = 20 / 9.99
//#define T1L 0xf8                // t1 div for ~28kHz / 14kHz boost
#define T1L 0xf3                // t1 div for ~20kHz / 10kHz boost
#define T1H 0xff                // 65536 - (250000 / 20000 == 13) = 65523/0xfff3

void main(void)
{
    TRISA = 0;                  // port A - all pins are out
    RA3 = 0;                    // stop sound
    TRISB = 0 b00000011;        // port B - B0/B1 - in 
    RB2 = boost_pulse = 0;      // set boost low
    CMCON = 0x07;               //выключение компаратора

    INTCON = 0 b11110000;
    /*
       GIE = 1;     // global int enable
       PEIE = 0;    // periferal irq
       T0IE = 1;    // t0 enabled
       INTE = 1;    // ext irq int enable
       RBIE = 0;
       T0IF = 0;    // t0 int flag
       INTF = 0;    // ext int flag
       RFIF = 0
     */
    OPTION_REG = 0 b00000011;
    /*
       nRBPU = 0;   // pullup on PB0/PB1 (def)
       INTEDG = 0;  // RB0/int on falling edge
       T0CS = 0;    // t0 internal source
       T0SE = 0;
       PSA = 0;     // select t0 div (1 -- wdt)
       PS2 = 0;     // PS2:PS0 = 011 - div16
       PS1 = 1;
       PS0 = 1;
     */
    T1CON = 0 b00010001;
    /*
       unused = 0;
       unused = 0;
       T1CKPS1 = 0; // T1CKPS1:T1CKPS0 = 01 (div == 4)
       T1CKPS0 = 1; // 250kHz base
       T1OSCEN = 0; // t1 clock gen enable
       T1SYNC = 0;  // t1 clock gen enable
       TMR1CS = 0;  // t1 internal FOSC/4 source
       TMR1ON = 1;  // t1 enable
     */
    TMR1IE = 1;                 // t1 int enable

    boost_perm = 1;             // no boost timeout initially
    boost = 1;                  // initial boost

    lcd_init();

    sek = GEIGER_TIME - 1;      // GEIGER_TIME sec measure time

    LIGHT;                      // light on
    TMR0 = 6;                   // start t0, 1MHz / 16 / (256-6) = 250Hz

    if (!RB1)                   // button pressed
        sound = 0;              // start w/o sound
    else
        sound = 1;              // start w/ sound

    lcd_goto(0);
    lcd_puts("GEiGER");
    lcd_goto(64);
    lcd_puts("C0UNtER");
    while (sek > GEIGER_TIME - 3);

    lcd_clear();
    if (!RB1) {                 // button not pressed
        lcd_puts("b00St 0N");
    } else {
        lcd_puts("StARtiNG");
        boost_perm = 0;         // no permanent boost
        boost = 0;              // stop init boost
    }
    lcd_goto(64);
    if (sound)
        lcd_puts("S0UNd 0N");
    else
        lcd_puts("SilENt");
    while (sek > GEIGER_TIME - 5);

    lcd_clear();
    light = light_time;
    while (1) {
        if (!RB1) {             // backlight on button press
            LIGHT;
            light = light_time;
        }
        if (old_sek != sek) {   // time (sec) changed
            if (!c)             // first measurement
                data = result * GEIGER_TIME / (GEIGER_TIME - 1 - sek);
            else                // 2nd and others
                //data = data2 * (40 + sek) / 80 + result / 2;
                data = ((data2 * sek) / GEIGER_TIME) + result;

            old_sek = sek;

            // 1999 uR*
            // 2.xx mR*
            // 22.x mR*
            q = text;
            //lcd_clear();
            lcd_goto(0);
            disp = data;
            if (data < 2000) {
                q += 3;
                do {
                    i = disp % 10;
                    *q-- = '0' + i;
                    disp /= 10;
                } while (disp);
                while (q >= text)
                    *q-- = ' ';
                lcd_puts(text);
                lcd_puts(uR);
            } else {
                if (disp < 10000) {
                    i = disp / 1000;
                    *q++ = '0' + i;
                    *q++ = '.';
                    disp = (disp % 1000 + 5) / 10;
                    *q++ = '0' + disp / 10;
                    *q++ = '0' + disp % 10;
                } else {
                    i = disp / 1000;
                    *q++ = '0' + i / 10;
                    *q++ = '0' + i % 10;
                    *q++ = '.';
                    i = (disp % 1000 + 50) / 100;
                    *q++ = '0' + i;
                }
                lcd_puts(text);
                lcd_puts(mR);
            }
            if (data >= 50)
                lcd_putch('*');
            else
                lcd_putch(' ');
            lcd_goto(64);
            for (i = 0; i < SCR_WIDTH; i++)
                if (i <= poisk)
                    lcd_putch(255);
                else
                    lcd_putch('-');
            poisk = 0;
            // just in case - boost every 10 sec when idle
            if (!boost_perm && !boost && ++boost_idle > BOOST_MAX_IDLE) {
                boost_idle = 0; // reset idle timer
                boost_timeout = BOOST_PACKET_LEN;
                boost = 1;
            }
        }

        if (!light)
            NOLIGHT;
    }
}

static void interrupt isr(void)
{
    if (TMR1IF) {               // timer1 int (10/14 kHz)
        if (boost_pulse)
            RB2 = boost_pulse = 0;      // boost high - set low
        else if (boost)
            RB2 = boost_pulse = 1;      // boost low - set high
        TMR1L = T1L;            // reset t1
        TMR1H = T1H;
        TMR1IF = 0;
    }
    if (T0IF) {                 // timer0 int (250 Hz)
        delay++;
        if (delay == 249) {     // 1 sec block
            delay = 0;
            if (sek)
                sek--;
            else {
                sek = GEIGER_TIME - 1;  // start new measurement loop
                if (!c)
                    c = 1;
                data2 = result; // store old result
                result = 0;
            }
            result += count;    // current result
            count = 0;          // zero count for next second
            if (light)
                light--;        // count backlight time
        }
        if (!boost_perm && boost && !--boost_timeout)
            boost = 0;
        if (sound && sound_timeout && !--sound_timeout)
            RA3 = 0;            // stop sound / tick
        TMR0 = 6;
        T0IF = 0;
    }
    if (INTF) {                 // irq from geiger
        if (count != 6500)
            count++;
        if (poisk < SCR_WIDTH)
            poisk++;
        if (!boost_perm) {      // if not constant boost
            boost_idle = 0;     // reset boost idle timer
            boost_timeout = BOOST_PACKET_LEN;
            if (!boost)
                boost = 1;
        }
        if (sound && !sound_timeout) {  // sound enabled and not running
            RA3 = 1;            // sound tick after geiger pulse
            sound_timeout = TICK_LEN;
        }
        INTF = 0;
    }
}
