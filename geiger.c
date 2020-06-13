/*
GEIGER COUNTER final 1.0 beta
17.06.2009
(c) TOTHEMA software, 2009
(x) mod 1.33 by venus, 2020
indented with: indent -kr -nut -c 40 -cd 40 -l 120 geiger.c
*/

#include <htc.h>
#include <stdint.h>
#include "lcd.h"

__CONFIG(FOSC_INTOSCIO & WDTE_OFF & PWRTE_OFF & MCLRE_ON & BOREN_OFF & LVP_OFF & CPD_OFF & CP_OFF);

// fb - 29/14.5, fa - 27.6/13.8, f4 = 20.6/10.3, f3 = 20 / 9.99
#define T1L 0xf8                       // t1 div for ~28kHz / 14kHz boost
//#define T1L 0xf3                       // t1 div for ~20kHz / 10kHz boost
#define T1H 0xff                       // 65536 - (250000 / 20000 == 13) = 65523/0xfff3

#define BUZZER_ON  RA3 = 1
#define BUZZER_OFF RA3 = 0

#define BRIGHTNESS     720             // 0..1023
#define BACKLIGHT_TIME   3             // backlight time, sec

#define ALARM_RATE  50
#define SCR_WIDTH    8
//#define GEIGER_TIME 75                 // 75 sec for SI29BG
#define GEIGER_TIME 36                 // 36 sec for SBM-20
#define TICK_LEN     6                 // sound tick len *4ms
#define SOUND_ADDR 0x00                // sound enable byte address it eeprom

bit c = 0;
uint32_t data;
volatile uint16_t count = 0, poisk = 0;
volatile uint32_t data2, result = 0;

uint8_t delay = 0, old_sek;
volatile uint16_t light;
volatile uint8_t sek;

#define BOOST_MAX_IDLE    5            // periodical boost (sec), just in case
#define BOOST_PACKET_LEN 60            // boost packet length (in 4msec units)

volatile uint8_t alarm = 0;            // beep N times

volatile bit boost;                    // bust running
volatile bit boost_perm;               // bust running permanently
volatile uint8_t boost_timeout = 0;    // boost packet len (x 4ms)
volatile uint8_t boost_idle = 0;       // periodic boost when no geiger pulses
volatile bit boost_pulse;

volatile uint8_t sound = 0;
volatile uint8_t sound_timeout = 0;

uint8_t i;

enum {
    SCR_RATE = 0,
    SCR_DOSE
} scr_mode = SCR_RATE;

enum button_states {
    PRESSED = 0,
    RELEASED
};

volatile uint32_t dose = 0;            // total dose
volatile uint32_t dose_sec = 0;        // time counter for dose
volatile uint16_t keytime = 0;         // key pressed time
volatile uint8_t misc;                 // utility value for delay4ms()

#define CHAR_MU    0x00
#define CHAR_ALARM 0x01
#define CHAR_DOSE  0x02

// *INDENT-OFF*

uint8_t char_mu[8] = {
    0b00000,
    0b00000,
    0b10001,
    0b10001,
    0b10011,
    0b11101,
    0b10000,
    0b10000
};

uint8_t char_alarm[8] = {
    0b00100,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b00100,
    0b00000
};

uint8_t char_dose[8] = {
    0b00000,
    0b11111,
    0b01000,
    0b00100,
    0b00010,
    0b00100,
    0b01000,
    0b11111
};

// *INDENT-ON*

void print(uint32_t disp)
{
    static char text[] = "    ";
    char *q = text;
    if (disp < 2000) {
        q += 3;
        do {
            i = disp % 10;
            *q-- = '0' + i;
            disp /= 10;
        } while (disp);
        while (q >= text)
            *q-- = ' ';
        lcd_puts(text);
        lcd_putch(' ');
        lcd_putch(CHAR_MU);
    } else {
        if (disp < 10000) {
            i = disp / 1000;
            *q++ = '0' + i;
            *q++ = '.';
            disp = (disp % 1000 + 5) / 10;
            *q++ = '0' + disp / 10;
            *q++ = '0' + disp % 10;
        } else if (disp < 100000) {
            i = disp / 1000;
            *q++ = '0' + i / 10;
            *q++ = '0' + i % 10;
            *q++ = '.';
            i = (disp % 1000 + 50) / 100;
            *q++ = '0' + i;
        } else {
            disp /= 1000;
            q += 3;
            do {
                i = disp % 10;
                *q-- = '0' + i;
                disp /= 10;
            } while (disp);
            while (q >= text)
                *q-- = ' ';
        }
        lcd_puts(text);
        lcd_putch(' ');
        lcd_putch('m');
    }
    lcd_putch('R');
}

// make delay for cnt*4ms
void delay4ms(uint8_t cnt)
{
    for (misc = 0; misc < cnt;)
        asm("nop");
}

// setup backlight pwm (duty cycle = 0..1023)
void pwm_set(uint16_t dc)
{
    CCPR1L = dc >> 2;
    CCP1CON &= 0xcf;
    CCP1CON |= 0x30 & (dc << 4);
}

void main(void)
{
    static uint8_t alarm_wait = 0;     // no alarm, waiting for normal rate
    static bit keystate, old_keystate;

    TRISA = 0x00;                      // port A - all pins are out
    RA3 = 0;                           // stop sound

// *INDENT-OFF*

    TRISB = 0b00000011;                // port B - B0/B1 - in 
    RB2 = boost_pulse = 0;             // set boost low
    CMCON = 0x07;                      //выключение компаратора


    INTCON = 0b11110000;
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
    OPTION_REG = 0b01000011;
    /*
       nRBPU = 0;   // pullup on PB0/PB1 (def)
       INTEDG = 1;  // RB0/int on rising edge
       T0CS = 0;    // t0 internal source
       T0SE = 0;
       PSA = 0;     // select t0 div (1 -- wdt)
       PS2 = 0;     // PS2:PS0 = 011 - div16
       PS1 = 1;
       PS0 = 1;
     */
    T1CON = 0b00010001;
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
    CCP1CON = 0x0F;                    // PWM mode CCP
    PR2 = 0xFF;                        // t2 period 255 
    T2CON = 0b00000110;
    /*
       TMR2ON = 1;  // t2 on
       T2CKPS1 = 1; // t2 prescaler 1:16
       T2CKPS0 = 0;

       tick for 4MHz = 0.25 usec
       pwm period = (255 + 1) * 4 * 16 * 0.25usec = 4096 usec
       pwm freq = 1 sec / 4096 usec ~= 244 Hz
     */

// *INDENT-ON*

    TMR1IE = 1;                        // t1 int enable

    boost_perm = 1;                    // no boost timeout initially
    boost = 1;                         // initial boost

    lcd_init();
    lcd_createChar(CHAR_MU, char_mu);
    lcd_createChar(CHAR_ALARM, char_alarm);
    lcd_createChar(CHAR_DOSE, char_dose);
    sek = GEIGER_TIME - 1;             // GEIGER_TIME sec measure time

    pwm_set(BRIGHTNESS);

    TMR0 = 6;                          // start t0, 1MHz / 16 / (256-6) = 250Hz

    if ((sound = EEPROM_READ(SOUND_ADDR)) > 1)
        sound = 1;

    if (!RB1) {

        // button pressed
        sound = 1 - sound;             // invert sound
        EEPROM_WRITE(SOUND_ADDR, sound);
    }

    lcd_goto(0);
    lcd_puts("GEiGER");
    lcd_goto(64);
    lcd_puts("C0UNtER");
    while (sek > GEIGER_TIME - 3);

    lcd_clear();
    if (!RB1) {                        // button not pressed
        lcd_puts("b00St 0N");
    } else {
        lcd_puts("StARtiNG");
        boost_perm = 0;                // no permanent boost
        boost = 0;                     // stop init boost
    }
    lcd_goto(64);
    if (sound)
        lcd_puts("S0UNd 0N");
    else
        lcd_puts("SilENt");
    while (sek > GEIGER_TIME - 5);

    lcd_clear();
    light = BACKLIGHT_TIME * 250;
    while (1) {
        if ((keystate = RB1) != old_keystate) {
            delay4ms(3);               // anti-rattle
            if (keystate == RB1) {
                keytime = 0;
                old_keystate = keystate;
            } else
                keystate = old_keystate;
        }
        if (keystate == PRESSED) {
            // backlight on button press
            pwm_set(BRIGHTNESS);
            light = BACKLIGHT_TIME * 250;
            if (keytime >= 375) {
                // pressed for ~2 sec - switch display mode
                keytime = 0;
                if (scr_mode == SCR_RATE)
                    scr_mode = SCR_DOSE;
                else
                    scr_mode = SCR_RATE;
            }
        }
        if (old_sek != sek) {          // time (sec) changed
            // calculate rate
            if (!c)                    // first measurement
                data = result * GEIGER_TIME / (GEIGER_TIME - 1 - sek);
            else                       // 2nd and others
                //data = data2 * (40 + sek) / 80 + result / 2;
                data = ((data2 * sek) / GEIGER_TIME) + result;
            old_sek = sek;
            if (scr_mode == SCR_RATE) {
                // 1999 uR*
                // 2.xx mR*
                // 22.x mR*
                //lcd_clear();
                lcd_goto(0);
                print(data);
                if (data >= ALARM_RATE)
                    lcd_putch(CHAR_ALARM);
                else
                    lcd_putch(' ');
                lcd_goto(64);
                for (i = 0; i < SCR_WIDTH; i++)
                    if (i <= poisk)
                        lcd_putch(255);
                    else
                        lcd_putch('-');
                poisk = 0;
            } else if (scr_mode == SCR_DOSE) {
                lcd_goto(0);
                // hours
                i = dose_sec / 3600;
                if (i > 9)
                    lcd_putch('0' + i / 10);
                else
                    lcd_putch(' ');
                lcd_putch('0' + i % 10);
                lcd_putch(':');
                // minutes
                i = (dose_sec % 3600) / 60;
                lcd_putch('0' + i / 10);
                lcd_putch('0' + i % 10);
                lcd_putch(':');
                // seconds
                i = dose_sec % 60;
                lcd_putch('0' + i / 10);
                lcd_putch('0' + i % 10);
                // dose
                lcd_goto(64);
                lcd_putch(CHAR_DOSE);
                print(dose * GEIGER_TIME / 3600);
            }
            if (data >= ALARM_RATE) {
                if (sound && !alarm && !alarm_wait) {
                    alarm = 5;         // beep 5 times
                    alarm_wait = 1;
                }
            } else if (alarm_wait)
                alarm_wait = 0;
            if (alarm) {
                BUZZER_ON;
                sound_timeout = 60;    // beep 240ms
                alarm--;
            }
            // just in case - boost every 10 sec when idle
            if (!boost_perm && !boost && ++boost_idle > BOOST_MAX_IDLE) {
                boost_idle = 0;        // reset idle timer
                boost_timeout = BOOST_PACKET_LEN;
                boost = 1;
            }
        }
    }
}

static void interrupt isr(void)
{
    if (TMR1IF) {                      // timer1 int (10/14 kHz)
        if (boost_pulse)
            RB2 = boost_pulse = 0;     // boost high - set low
        else if (boost)
            RB2 = boost_pulse = 1;     // boost low - set high
        TMR1L = T1L;                   // reset t1
        TMR1H = T1H;
        TMR1IF = 0;
    }
    if (T0IF) {                        // timer0 int (250 Hz)
        misc++;
        keytime++;
        if (++delay == 249) {          // 1 sec block
            delay = 0;
            if (dose_sec < 100 * 3600 - 1)      // max 99:59:59
                dose_sec++;
            else {
                dose = 0;
                dose_sec = 0;
            }
            if (sek)
                sek--;
            else {
                sek = GEIGER_TIME - 1; // start new measurement loop
                if (!c)
                    c = 1;
                data2 = result;        // store old result
                result = 0;
            }
            result += count;           // current result
            count = 0;                 // zero count for next second
        }
        if (light)
            if (!--light)
                pwm_set(0);
        if (!boost_perm && boost && !--boost_timeout)
            boost = 0;
        if ((sound || alarm) && sound_timeout && !--sound_timeout)
            BUZZER_OFF;                // stop sound / tick
        TMR0 = 6;
        T0IF = 0;
    }
    if (INTF) {                        // irq from geiger
        if (count != 6500)
            count++;
        if (poisk < SCR_WIDTH)
            poisk++;
        if (dose < 9999999L * 3600 / GEIGER_TIME)
            dose++;
        if (!boost_perm) {             // if not constant boost
            boost_idle = 0;            // reset boost idle timer
            boost_timeout = BOOST_PACKET_LEN;
            if (!boost)
                boost = 1;
        }
        if (sound && !alarm && !sound_timeout) {        // sound enabled and not running
            BUZZER_ON;                 // sound tick after geiger pulse
            sound_timeout = TICK_LEN;
        }
        INTF = 0;
    }
}
