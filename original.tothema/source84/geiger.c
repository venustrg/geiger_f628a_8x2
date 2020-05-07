/*
GEIGER COUNTER final 1.0 beta
17.06.2009
*/

#include <pic1684.h>
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
unsigned int count;//, poisk, a;
unsigned long int data, data2, result;
bit c;

main()
{
TRISA=0; //Port A is OUT
TRISB=0b00000011; //Port B 1 - IN, 0 - OUT

T0CS=0;   //���������� �������� ������ ��� TMR0
INTEDG=1; //���������� �� ��������� ������ RB0/INT
T0IE=1;   //���������� ���������� �� ������������ TMR0
INTE=1;   //���������� �������� ���������� INT

lcd_init();

PSA=0; PS2=0; PS1=1; PS0=1; // �������� ������� �� 16
c=0; // ������ ����
count=0;
result=0;
//poisk=0;
delay=0;
sek=39; //�������� 40 ��������� �������� (0-39)

LIGHT; //�������� ����
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

if (old_sek!=sek) // ���� ��������� ��������� �� �������
{
if (!c) // ������ ����
    data=result*40/(39-sek);
    else  // ������ ����
    data=((data2*sek)/40) + result;
old_sek=sek;
lcd_clear();
 if (data<1000)
{lcd_clear();lcd_goto(0);lcd_puts ("0");}
else
{lcd_clear();lcd_goto(0);lcd_puts ("1");}
lcd_puts(".");lcd_putn(data-(1000*(data/1000)));lcd_puts(mR);
 if (data>1999)
{lcd_clear();lcd_goto(0);lcd_putn (data/999); lcd_puts(mR);}

if (result<1000){lcd_goto(64);lcd_putn(result);}
else {lcd_goto(64);lcd_putn(result/10);lcd_puts ("0");}

lcd_goto(69);lcd_putn(sek);

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
 if (T0IF){ //���������� �� �������
  T0IF=0;
  TMR0=6;
  delay++;

    if (delay==249) // �������� ������� 1 �������
     {
      delay=0;
      if (sek) sek--;
       else
        {
         sek=39; // �������� ������� 10 ������
         if (!c) c=1;
         data2=result;
         result=0;
        }
      result+=count; count=0; // ���������� � ������ ����������� ����� �� ������ ���
      if (light) light--; // ���� ����� �������� !=0 ����� ��������� �� 1 ���
     }
 }

 if (INTF){ // ���������� �� ����� ��������
   if (count!=6500) count++;
   //if (poisk<8) poisk++;
   INTF=0;
   }
}
//---------------------------------------------- end of interrupt -----------------
