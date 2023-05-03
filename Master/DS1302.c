#include <delay.h>

//----------------------------------------------------------------------------------------
// Описание подключения DS1302 к порту и пинам контроллера
#define DS1302_DDR  DDRF 
#define DS1302_PORT PORTF 
#define DS1302_PIN  PINF 

#define DS1302_RST  7       // chip enable
#define DS1302_IO   6       // io
#define DS1302_CLK  5       // sclk
/*
    DS1302_SetTime(20, 11, 15);       // Установить время 20:11.15 чч:мм.сс
    DS1302_SetDate(18, 01, 10);       // Установить дату 18-01-10  дд-мм-гг
    DS1302_GetDate(&d, &m, &y);       // Прочитать дату  дд мм гг
    DS1302_GetTime(&h, &m, &s);       // Прочитать время чч мм сс
//----------------------------------------------------------------------------------------
*/

#define SetBit(x)    |= (1<<x) 
#define ClearBit(x)  &=~(1<<x) 

#define DS1302_HOURREAD         0x85       // hour register read
#define DS1302_MINREAD          0x83       // minute register read
#define DS1302_SECREAD          0x81       // second register read

#define DS1302_HOURWRITE        0x84       // hour register write
#define DS1302_MINWRITE         0x82       // minute register write
#define DS1302_SECWRITE         0x80       // second register write

#define DS1302_DAYREAD          0x87       // day register write
#define DS1302_MONTHREAD        0x89       // month register write
#define DS1302_YEARREAD         0x8D       // year register write

#define DS1302_DAYWRITE         0x86       // day register write
#define DS1302_MONTHWRITE       0x88       // month register write
#define DS1302_YEARWRITE        0x8C       // year register write


#define DS1302_CONTROLWRITE     0x8E       // control register write
#define DS1302_TRICLEWRITE      0x90       // TRICKLE CHARGER write

#define DS1302_PREAMBLE         0x00       // preambule
#define DS1302_EOT              0xFF       // end of transmition


#pragma used+
char DS1302_Read(char);
void DS1302_Write(char, char);
void DS1302_SetTime(char, char, char);
void DS1302_GetTime(char *, char *, char *);
void DS1302_SetDate(char, char, char);
void DS1302_GetDate(char *, char *, char *);
char bcd2dec(char);
char dec2bcd(char);
#pragma used-


char bcd2dec(char bcd)
{
    int x,y;
    x = (bcd & 0xf);
    y = ((bcd & 0xf0)>>4)*10;
    return x+y;
}

char dec2bcd(char val)      // входное значение не должно превышать 99
{
 unsigned char out=0, inp;
 
 inp=val;

  while (inp>=10)
  {
    inp-=10;
    out+=1<<4;
  }
  out+=inp;
  return (out);
}



//*************************************
// чтение байта данных из DS1302
char DS1302_Read(char caddr)
{
   char cdata=0;
   char i=8;
//   DS1302_DDR= (1<<DS1302_CLK) | (1<<DS1302_RST) | (1<<DS1302_IO);
   DS1302_DDR SetBit(DS1302_CLK);
   DS1302_DDR SetBit(DS1302_RST);
   DS1302_DDR SetBit(DS1302_IO);
   
   DS1302_PORT SetBit(DS1302_RST);
   
   for(i=0; i<8; i++)                                       // запись адреса нужного регистра
   {
         if(caddr&0x01) DS1302_PORT SetBit(DS1302_IO); 
         else           DS1302_PORT ClearBit(DS1302_IO);
         caddr>>=1;
         DS1302_PORT SetBit(DS1302_CLK);
         delay_us(10);
         DS1302_PORT ClearBit(DS1302_CLK);
         delay_us(10);
   }
   
   DS1302_DDR ClearBit(DS1302_IO);
   for(i=0; i<8; i++)                                       // чтение из DS1302
   {
         cdata >>= 1;
         if(DS1302_PIN & (1<<DS1302_IO))         cdata |= 0x80;
         DS1302_PORT SetBit(DS1302_CLK);
         delay_us(10);
         DS1302_PORT ClearBit(DS1302_CLK);
         delay_us(10);
//         if(DS1302_PIN & (1<<DS1302_IO))         cdata |= 0x80;
   }
   DS1302_PORT ClearBit(DS1302_RST);
//   DS1302_DDR= (0<<DS1302_CLK) | (0<<DS1302_RST) | (0<<DS1302_IO);
   return cdata;
}




void DS1302_Write(char caddr, char cdata)
{
   char i=8;
//   DS1302_DDR= (1<<DS1302_CLK) | (1<<DS1302_RST) | (1<<DS1302_IO);
   DS1302_DDR SetBit(DS1302_CLK);
   DS1302_DDR SetBit(DS1302_RST);
   DS1302_DDR SetBit(DS1302_IO);

   DS1302_PORT SetBit(DS1302_RST);
   for(i=0; i<8; i++) 
   {
         if(caddr&0x01) DS1302_PORT SetBit(DS1302_IO); 
         else           DS1302_PORT ClearBit(DS1302_IO);
         caddr>>=1;
         DS1302_PORT SetBit(DS1302_CLK);
         delay_us(10);
         DS1302_PORT ClearBit(DS1302_CLK);
         delay_us(10);
   }
   for(i=0; i<8; i++) 
   {
         if(cdata&0x01) DS1302_PORT SetBit(DS1302_IO); 
         else           DS1302_PORT ClearBit(DS1302_IO);
         cdata >>= 1;
         DS1302_PORT SetBit(DS1302_CLK);
         delay_ms(1);
         DS1302_PORT ClearBit(DS1302_CLK);
         delay_ms(1);
   }
   DS1302_PORT ClearBit(DS1302_RST);
//   DS1302_DDR= (0<<DS1302_CLK) | (0<<DS1302_RST) | (0<<DS1302_IO);
}


void DS1302_GetTime(char *hour, char *min, char *sec)
{
    char cTmp;
    cTmp=DS1302_Read(DS1302_HOURREAD);     //часы
    *hour=bcd2dec(cTmp);

    cTmp=DS1302_Read(DS1302_MINREAD);      //минуты
    *min=bcd2dec(cTmp);

    cTmp=DS1302_Read(DS1302_SECREAD);      //секунды
    *sec=bcd2dec(cTmp);
}


void DS1302_SetTime(char hour, char min, char sec)
{
//   char ucSec=0;
   DS1302_Write(DS1302_CONTROLWRITE, 0x00);   // WP bit should be cleared before attempting to write to the device.
   //data_write(TCW, 0xAB); // write TRICKLE CHARGER, 8K, 2 DIODES

   DS1302_Write(DS1302_HOURWRITE, dec2bcd(hour) );   // set hour
   DS1302_Write(DS1302_MINWRITE,  dec2bcd(min)  );   // set min
//   ucSec=dec2bcd(sec);
//   ucSec SetBit(7);
   DS1302_Write(DS1302_SECWRITE,  dec2bcd(sec)  );    // set sec(start time)
//   DS1302_Write(DS1302_SECWRITE,  ucSec  );          // set sec(start time)
}

void DS1302_GetDate(char *day, char *month, char *year)
{
    char cTmp;
    cTmp=DS1302_Read(DS1302_DAYREAD);     //часы
    *day=bcd2dec(cTmp);

    cTmp=DS1302_Read(DS1302_MONTHREAD);      //минуты
    *month=bcd2dec(cTmp);

    cTmp=DS1302_Read(DS1302_YEARREAD);      //секунды
    *year=bcd2dec(cTmp);
}

void DS1302_SetDate(char Day, char Month, char Year)
{
   DS1302_Write(DS1302_CONTROLWRITE, 0x00);   // WP bit should be cleared before attempting to write to the device.
   //data_write(TCW, 0xAB); // write TRICKLE CHARGER, 8K, 2 DIODES

   DS1302_Write(DS1302_DAYWRITE,  dec2bcd(Day)  );     // set day
   DS1302_Write(DS1302_MONTHWRITE,dec2bcd(Month));   // set month
   DS1302_Write(DS1302_YEARWRITE, dec2bcd(Year) );    // set year
}
