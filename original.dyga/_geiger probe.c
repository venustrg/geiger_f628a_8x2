/*
GEIGER COUNTER v 0.9p Probe
01.06.2009
*/

#include <pic1684.h>
#include "delay.c"
#include "lcd.c"

#define LIGHT RA2=1
#define NOLIGHT RA2=0
#define SOUND RA3=1
#define NOSOUND RA3=0

const unsigned char light_time=3; //врем€ свечени€ светодиода (секунд)
const char mkR[5]={' ',228,'k','R'};
const char mR[]=" mR ";

unsigned char light, delay, sek, old_sek;
unsigned char i;
unsigned int count, data[10], result, result_2, result_3;
unsigned long int tmp;
bit mode, beep;

void main(void)
{
TRISA=0; //Port A is OUT
TRISB=0b00000011; //Port B 1 - IN, 0 - OUT

T0CS=0;   //внутренний тактовый сигнал дл€ TMR0
INTEDG=1; //ѕрерывание по переднему фронту RB0/INT
T0IE=1;   //разрешение прерывани€ по переполнению TMR0
INTE=1;   //–азрешение внешнего прерывани€ INT
PSA=0; PS2=0; PS1=1; PS0=1; // ƒелитель таймера на 16
count=0;
delay=0;
sek=9; //задаем 10 секундный интервал (0-9)
for (i=0; i<10; i++) data[i]=0; // ќбнул€ем масив показаний
if (!RB2) beep=1; else beep=0;
TMR0=5;
GIE=1;

lcd_init();
lcd_goto(0); lcd_puts(" GEIGER "); lcd_goto(65); lcd_puts("counter");

LIGHT; //включаем свет 
while (sek>7); 

if (!RB1) mode=1; else mode=0; //≈сли при включении нажата кнопка - накопление

lcd_clear();
lcd_goto(2);lcd_puts("WAIT");

while (sek) { lcd_goto(71-sek); lcd_putch(255); }
if (!mode) {tmp=result; result_3=result; result_2=result;} else tmp=result*4; //начальное присвоение

lcd_clear();
light=light_time;

LOOP:
//:::::::::::::::::::::::::::::::::::::::::::::: key (light) :::::::::::::::::::
if (!RB1)
 {
  LIGHT; 
  light=light_time;

  while (!RB1);
 }
//:::::::::::::::::::::::::::::::::::::::::::::: end of key (light)

//********************************************** program *************************

if (old_sek!=sek) // ≈сли произошло изменение по времени
{
  result=0; for (i=0; i<10; i++) result+=data[i]; // ссумируем результат за 10 сек

  lcd_clear();
  old_sek=sek;

  switch (mode)
  {

    case 0: //режим измерени€
     { 
      if (!sek) // если 10 секундный интервал
       {
         tmp=result_3;
         result_3=result_2;
         result_2=result; 
         tmp=tmp+result_2+result_3;
       } 
      
      if ((tmp+result)>999)
       { lcd_putn((tmp+result)/1000); lcd_puts(mR);}
      else
       { lcd_putn(tmp); lcd_puts(mkR); }
      break;
     }

    case 1: //режим накоплени€
     {
      tmp=tmp*0,75+result;
      if (tmp>999)
       { lcd_putn(tmp/1000); lcd_puts(mR);}
      else
       { lcd_putn(tmp); lcd_puts(mkR); }
      break;
     }
  }

  lcd_goto(64); if (mode) lcd_puts ("mode=1"); else lcd_puts ("mode=0");
  
}

if (!light) NOLIGHT;
//********************************************** end of program ******************
goto LOOP;
}

//---------------------------------------------- interrupt -----------------------
static void interrupt
isr(void)
{
 if (T0IF) //прерывание по таймеру
  {
   T0IF=0;
   TMR0=5;
   delay++;
  
    if (delay==250) // интервал времени 1 секунда
     { 
      delay=0;
      data[sek]=count; count=0; // записываем в массив результатов счета за каждую сек
      if (sek)sek--; else sek=9;// интервал времени 10 секунд
      if (light) light--; // если врем€ свечени€ !=0 тогда уменьшаем на 1 сек
     }
  }

 if (INTF) // прерывание по входу счетчика
  {
   if (beep) SOUND;
   if (count!=6500) count++;
   INTF=0;
   NOSOUND;
  }
}
//---------------------------------------------- end of interrupt -----------------
