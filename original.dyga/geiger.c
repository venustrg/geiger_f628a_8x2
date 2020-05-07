/*
GEIGER COUNTER 2012 8x2 version
*/

#include <pic16f62xa.h>
#include "delay.c"
#include "lcd.c"

#define LIGHT RA2=1
#define NOLIGHT RA2=0
#define SOUND RA3=1
#define NOSOUND RA3=0
#define ON RB2=1
#define OFF RB2=0
//#define DISPON RB3=1
//#define DISPOFF RB3=0

const unsigned char light_time=10;
const char mkR[]=" mkR";
const char mR[]=" mR";

unsigned char light, delay, buff, cycle;
unsigned char i;
unsigned int count, poisk, poisk2, a, a2, button;
unsigned long int data, data2, result, sek, old_sek;
bit c, d;
main()
{ 
TRISA=0; //Port A is OUT
TRISB=0b00000011; //Port B 1 - IN, 0 - OUT
CMCON = 0x07; //���������� �����������

T0CS=0;   //���������� �������� ������ ��� TMR0
INTEDG=1; //���������� �� ��������� ������ RB0/INT
T0IE=1;   //���������� ���������� �� ������������ TMR0
INTE=1;   //���������� �������� ���������� INT

lcd_init();

PSA=0; PS2=0; PS1=1; PS0=1; // �������� ������� �� 16
c=0; // ������ ����
d=0; //����� 40 ���
ON;
buff=125;
count=0;
result=0;
poisk=0;
delay=0;
cycle=2; // ���������� ������ �� ��������� 400 ���������� ������
if (!RB1) { sek=399, d=1; } else sek=39; //�������� 40 ��� 400 ��������� �������� ���� ������ ������


NOLIGHT; //�� �������� ����
NOSOUND; //�� �������� ����
TMR0=6;
GIE=1; 

if (!d) while (sek>38) {lcd_goto(0); lcd_puts("TOTHEMA"); lcd_goto(64);lcd_puts("software"); }
else while (sek>398) {lcd_goto(0); lcd_puts("TOTHEMA"); lcd_goto(64);lcd_puts("software"); }//�����������
lcd_clear();

if (!d) while (sek>35) {lcd_goto(0); lcd_puts("T=40 sek");lcd_goto(64);lcd_puts("Wait."); lcd_goto(107-sek);lcd_puts("."); }
else  while (sek>395)  {lcd_goto(0); lcd_puts("T=400sek");lcd_goto(64);lcd_puts("Wait.");lcd_goto(467-sek);lcd_puts("."); }

lcd_clear();
if (!RB1) SOUND; else NOSOUND;
light=light_time;
poisk=0;
LOOP:
//:::::::::::::::::::::::::::::::::::::::::::::: key :::::::::::::::::::

if (!RB1) //Light
 {
  LIGHT;
  light=light_time;
//if (button<7) poisk=(poisk+1);



 }
//:::::::::::::::::::::::::::::::::::::::::::::: end of key

//********************************************** program *************************

if (old_sek!=sek) // ���� ��������� ��������� �� �������
{
 if (!c) // ���� ������ ����
if (!d) { data=result*40/(39-sek); } //
else {data=(result*400/(399-sek))/10; }
else  // ����� ������ ����
if(!d) { data=((data2*sek)/40) + result; }
else { data=(((data2*sek)/400) + result)/10; }
old_sek=sek;

 if (data<1000) 
{lcd_clear();lcd_goto(0);}
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
 lcd_putn(data-(1000*(data/1000)));lcd_puts(mkR);
 if (data>9999)
 {lcd_clear();lcd_goto(0); lcd_putn (data/999); lcd_puts(mR);}
 


//if (!d) 
//{if (result<1000){lcd_goto(64); lcd_putn(result);}
//if ((result>999)&(result<10000)){lcd_goto(64); lcd_putn(result/10);lcd_puts("0");}
//if (result>9999){lcd_goto(64);lcd_putn(result/999);lcd_puts("K");}
//lcd_goto(69); lcd_putn(sek);}
//else
{lcd_goto(64);
for (a2=0; a2<poisk2; a2++)
lcd_puts("-");}
{lcd_goto(64);
for (a=0; a<poisk; a++)
lcd_putch(255); }
//if (!d) {lcd_goto(69); lcd_putn(sek);}
if (poisk) poisk=(poisk-1);
if (poisk2) poisk2=(poisk2-1);
if (RA3) {lcd_goto(7);lcd_putch(237);}





}

  
 if (!light) NOLIGHT;


if((poisk>5)&(c==1))
{
//lcd_goto(64);lcd_puts("TPEBO�A!");
PSA=0; PS2=0; PS1=1; PS0=1;
c=0; 
d=0; 
count=0;
result=0;
delay=0;
cycle=2;
sek=39; 
TMR0=6;
GIE=1; 
SOUND;

}

 
 
  

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
  buff--;
  if (buff==0) { OFF; }
  delay++;

    if (delay==250) // �������� ������� 1 �������
     {
      delay=0;


if (!poisk2) { ON; buff=10; }
      if (sek) sek--; 
       else
        {
if (cycle) cycle--;
if ((!cycle)&(!d)) {(d=1);result=(result*10);}
if (!d) sek=39; else {sek=399; NOSOUND;} // �������� ������� 40 ������ ��� 400 ������

         if (!c) c=1;
         data2=result;
         result=0;
        }


      result+=count; count=0; // ���������� � ������ ����������� ����� �� ������ ���
      if (light) {light--;} // ���� ����� �������� !=0 ����� ��������� �� 1 ���
     }
 }

 if (INTF){ // ���������� �� ����� ��������
   { ON; buff=1; } 
   if (count!=6500) count++; 
   if (poisk<16) poisk++;
   if (poisk2<12) {poisk2++; poisk2++; poisk2++; poisk2++; poisk2++; }
   INTF=0;
   }
}
//---------------------------------------------- end of interrupt -----------------
