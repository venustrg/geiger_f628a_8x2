/*
GEIGER COUNTER final 1.0 beta
17.06.2009
*/

#include <pic16f62xa.h>
#include "delay.c"
#include "lcd.c"

#define LIGHT RA2=1
#define NOLIGHT RA2=0
#define SOUND RA3=1
#define NOSOUND RA3=0

const unsigned char light_time=3;
//const char mkR[]=" mkR";
const char mR[]=" mR";

unsigned char light, delay, sek, old_sek;
unsigned char i;
unsigned int count; //poisk,a
unsigned long int data, data2, result;
bit c;
main()
{ 
TRISA=0; //Port A is OUT
TRISB=0b00000011; //Port B 1 - IN, 0 - OUT
CMCON = 0x07; //выключение компаратора

T0CS=0;   //внутренний тактовый сигнал дл€ TMR0
INTEDG=1; //ѕрерывание по переднему фронту RB0/INT
T0IE=1;   //разрешение прерывани€ по переполнению TMR0
INTE=1;   //–азрешение внешнего прерывани€ INT

lcd_init();

PSA=0; PS2=0; PS1=1; PS0=1; // ƒелитель таймера на 16
c=0; // первый цикл
count=0;
result=0;
//poisk=0;
delay=0;
sek=39; //отмер€ем 20 секундный интервал (0-39)
 
LIGHT; //включаем свет
TMR0=6;
GIE=1; 

lcd_goto(0); lcd_puts("TOTHEMA");
lcd_goto(64); lcd_puts("software");
while (sek>37);

if (!RB1) NOSOUND; else SOUND;
lcd_clear();
lcd_goto(2);lcd_puts("WAIT");
while (sek>29){ lcd_goto(101-sek); lcd_putch(255); }

lcd_clear();
light=light_time;
//poisk=0;
LOOP:
//:::::::::::::::::::::::::::::::::::::::::::::: key :::::::::::::::::::
if (!RB1) //Light
 {
  LIGHT;
light=light_time;


 }
//:::::::::::::::::::::::::::::::::::::::::::::: end of key

//********************************************** program *************************

if (old_sek!=sek) // ≈сли произошло изменение по времени
{
 if (!c) // первый цикл
    data=result*40/(39-sek);
    else  // второй цикл
    //data=data2*(41+sek)/80 + result/2;
    data=((data2*sek)/40) + result;
old_sek=sek;
    lcd_clear();

 if (data<1000) 
{lcd_clear();lcd_goto(0);lcd_puts ("0");}
 if (data>999) 
{lcd_clear();lcd_goto(0);lcd_puts ("1");}
 if (data>1999) 
{lcd_clear();lcd_goto(0);lcd_puts ("2");}
 if (data>2999) 
{lcd_clear();lcd_goto(0);lcd_puts ("3");}
 if (data>3999) 
{lcd_clear();lcd_goto(0);lcd_puts ("4");}
 if (data>4999) 
{lcd_clear();lcd_goto(0);lcd_puts ("5");}
 if (data>5999) 
{lcd_clear();lcd_goto(0);lcd_puts ("6");}
 if (data>6999) 
{lcd_clear();lcd_goto(0);lcd_puts ("7");}
 if (data>7999) 
{lcd_clear();lcd_goto(0);lcd_puts ("8");}
 if (data>8999) 
{lcd_clear();lcd_goto(0);lcd_puts ("9");}
lcd_puts(".");lcd_putn(data-(1000*(data/1000)));lcd_puts(mR);
 if (data>9999)
 {lcd_clear();lcd_goto(0);lcd_putn (data/999); lcd_puts(mR);}
 
//if (result>999){lcd_goto(64);lcd_putn(result-(1000*(result/1000)));}
if (result<1000){lcd_goto(64);lcd_putn(result);}
if ((result>999)&(result<10000)){lcd_goto(64);lcd_putn(result/10);lcd_puts("0");}
if (result>9999){lcd_goto(64);lcd_putn(result/999);lcd_puts("K");}

lcd_goto(69);lcd_putn(sek);
//lcd_goto(64);
//for (a=0; a<poisk; a++)
//lcd_putch(255); 
//poisk=0;


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
  TMR0=6;
  delay++;

    if (delay==249) // интервал времени 1 секунда
     {
      delay=0;
      if (sek) sek--;
       else
        {
         sek=39; // интервал времени 20 секунд
         if (!c) c=1;
         data2=result;
         result=0;
        }


      result+=count; count=0; // записываем в массив результатов счета за каждую сек
      if (light) light--; // если врем€ свечени€ !=0 тогда уменьшаем на 1 сек
     }
 }

 if (INTF){ // прерывание по входу счетчика
   if (count!=6500) count++;
   //if (poisk<8) poisk++;
   INTF=0;
   }
}
//---------------------------------------------- end of interrupt -----------------
