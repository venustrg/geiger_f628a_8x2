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

const unsigned char light_time=3; //����� �������� ���������� (������)
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

T0CS=0;   //���������� �������� ������ ��� TMR0
INTEDG=1; //���������� �� ��������� ������ RB0/INT
T0IE=1;   //���������� ���������� �� ������������ TMR0
INTE=1;   //���������� �������� ���������� INT
PSA=0; PS2=0; PS1=1; PS0=1; // �������� ������� �� 16
count=0;
delay=0;
sek=9; //������ 10 ��������� �������� (0-9)
for (i=0; i<10; i++) data[i]=0; // �������� ����� ���������
if (!RB2) beep=1; else beep=0;
TMR0=5;
GIE=1;

lcd_init();
lcd_goto(0); lcd_puts(" GEIGER "); lcd_goto(65); lcd_puts("counter");

LIGHT; //�������� ���� 
while (sek>7); 

if (!RB1) mode=1; else mode=0; //���� ��� ��������� ������ ������ - ����������

lcd_clear();
lcd_goto(2);lcd_puts("WAIT");

while (sek) { lcd_goto(71-sek); lcd_putch(255); }
if (!mode) {tmp=result; result_3=result; result_2=result;} else tmp=result*4; //��������� ����������

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

if (old_sek!=sek) // ���� ��������� ��������� �� �������
{
  result=0; for (i=0; i<10; i++) result+=data[i]; // ��������� ��������� �� 10 ���

  lcd_clear();
  old_sek=sek;

  switch (mode)
  {

    case 0: //����� ���������
     { 
      if (!sek) // ���� 10 ��������� ��������
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

    case 1: //����� ����������
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
 if (T0IF) //���������� �� �������
  {
   T0IF=0;
   TMR0=5;
   delay++;
  
    if (delay==250) // �������� ������� 1 �������
     { 
      delay=0;
      data[sek]=count; count=0; // ���������� � ������ ����������� ����� �� ������ ���
      if (sek)sek--; else sek=9;// �������� ������� 10 ������
      if (light) light--; // ���� ����� �������� !=0 ����� ��������� �� 1 ���
     }
  }

 if (INTF) // ���������� �� ����� ��������
  {
   if (beep) SOUND;
   if (count!=6500) count++;
   INTF=0;
   NOSOUND;
  }
}
//---------------------------------------------- end of interrupt -----------------
