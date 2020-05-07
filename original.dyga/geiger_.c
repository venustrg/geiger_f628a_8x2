/*
GEIGER COUNTER v 0.6
29.05.2009
*/

#include <pic1684.h>
#include "delay.c"
#include "lcd.c"

#define LIGHT RA2=1
#define NOLIGHT RA2=0
#define SOUND RA3=1
#define NOSOUND RA3=0

const unsigned char light_time=3;
const char mkR[5]={' ',228,'k','R'};
const char mR[]=" mR ";

unsigned char light, delay, sek, old_sek;
unsigned char i;
unsigned int count, data[10], result, result_2, result_3, result_4, tmp;
bit beep;

main()
{
TRISA=0; //Port A is OUT
TRISB=0b00000011; //Port B 1 - IN, 0 - OUT

T0CS=0;   //внутренний тактовый сигнал для TMR0
INTEDG=1; //Прерывание по переднему фронту RB0/INT
T0IE=1;   //разрешение прерывания по переполнению TMR0
INTE=1;   //Разрешение внешнего прерывания INT

lcd_init();
lcd_goto(0); lcd_puts(" GEIGER ");
lcd_goto(64); lcd_puts("counter");

PSA=0; PS2=0; PS1=1; PS0=1; // Делитель таймера на 16
count=0;
delay=0;
sek=9; //отмеряем 10 секундный интервал (0-9)
for (i=0; i<10; i++) data[i]=0; // Обнуляем масив показаний
LIGHT; //включаем свет
TMR0=5;
GIE=1;

while (sek>7); 
lcd_clear();
lcd_goto(2);lcd_puts("WAIT");

if (!RB1) beep=1; else beep=0; //Если при включении нажата кнопка - звук есть

while (sek){ lcd_goto(71-sek); lcd_putch(255); }
while (!sek);
result=0; for (i=0; i<10; i++) result+=data[i]; // результат за 10 сек
result_4=result; result_3=result; result_2=result;

lcd_clear();
light=light_time;

LOOP:
//:::::::::::::::::::::::::::::::::::::::::::::: key (light) :::::::::::::::::::
if (!RB1) //Light
 {
  LIGHT; 
  light=light_time;
  while (!RB1);
 }
//:::::::::::::::::::::::::::::::::::::::::::::: end of key (light)

//********************************************** program *************************

if (old_sek!=sek) // Если произошло изменение по времени
{
  lcd_clear();
  old_sek=sek;
  result=0; for (i=0; i<10; i++) result+=data[i]; // результат за 10 сек

   if ((result>999) || (result_2>999) || (result_3>999) || (result_4>999))
    {
     tmp=(result/1000)+(result_2/1000);
     lcd_putn(tmp+(result_3/1000)+(result_4/1000));
     lcd_puts(mR);
    }
   else
    { 
     tmp=result+result_2+result_3+result_4;
     if (tmp>999) {lcd_putn(tmp/1000); lcd_puts(mR);}
     else {lcd_putn(tmp); lcd_puts(mkR);}
    }
  lcd_goto(64); if (beep) lcd_puts ("beep=1"); else lcd_puts ("beep=0");
 }
if (!light) NOLIGHT;
//********************************************** end of program ******************
goto LOOP;
}

//---------------------------------------------- interrupt -----------------------
static void interrupt
isr(void)
{
 if (T0IF){ //прерывание по таймеру
  T0IF=0;
  TMR0=5;
  delay++;

    if (delay==250){ // интервал времени 1 секунда
     delay=0;
     data[sek]=count; count=0; // записываем в массив результатов счета за каждую сек
     if (sek)sek--; else { // интервал времени 10 секунд
                     sek=9;
                     result_4=result_3;
                     result_3=result_2;
                     result_2=result;
                         }
     if (light) light--; // если время свечения !=0 тогда уменьшаем на 1 сек
    }
 }

 if (INTF){ // прерывание по входу счетчика
   if (beep) SOUND;
   if (count!=65000) count++;
   INTF=0;
   NOSOUND;
   }
}
//---------------------------------------------- end of interrupt -----------------
