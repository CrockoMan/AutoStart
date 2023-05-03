/*****************************************************
Chip type               : ATmega128
Program type            : Application
AVR Core Clock frequency: 8,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 1024
*****************************************************/

#include <mega128.h>
#include <delay.h>
#include <stdlib.h>
#include <MT12864.c>  
#include <TRC.c>  
#include <DS1302.c>  


void GenCMD(char );
void InitAVR(void);
void Beep(void);
void LCD_Paint(void);
void ShowDateTime(void);
unsigned int Read_ADC(unsigned char );
void uc2str(unsigned char,unsigned char *);
void ShowDate(void);
unsigned char cCompare(unsigned char *, unsigned char *);

#define ADC_VREF_TYPE 0xC0


//#define SetBit(x)    |= (1<<x) 
//#define ClearBit(x)  &=~(1<<x) 
#define IsBitSet(x, y)   ((x)  & (1 << (y)))
#define IsBitClear(port, bit) (~(port) & (1 << (bit)))

#define LEDON()     PORTB ClearBit(7)
#define LEDOFF()    PORTB SetBit(7)

#define AC_GEN_ON()     PORTF SetBit(3)
#define AC_GEN_OFF()    PORTF ClearBit(3)
#define AC_LINE_ON()    PORTF SetBit(2)
#define AC_LINE_OFF()   PORTF ClearBit(2)

#define TRUE        1 
#define FALSE       0 

// ����� ����������
#define OFF         0 
#define ON          1 
#define START       2 

// ������� ����� ������� (�������, �����, ���������)
#define LINE        0 
#define BATT        1 
#define GEN         2

// ��� cStatus
#define TRC         0
#define BATT_IBP    1  
#define DATETIME    2  

//��� Mode
#define STOP            0
#define TIMERLINE       1
#define TIMERGENON      2
#define TIMERGENOFF     2
#define TIMERGENWAIT    3
#define TIMERWAITHEAT   4

#define IS_LINE_ON()    IsBitClear(PINE,5)      //���������� �� ����� PINE 5    
#define IS_GEN_ON()     IsBitClear(PINE,4)      //���������� �� ���������� PINE 4
#define IS_COMMON_ON()  IsBitClear(PIND,0)      //����� ���� ������� PIND 0    

//struct DateTimeStructure
typedef struct
       { 
            char Day;
            char Month;
            char Year;
            char Hour;
            char Min;
            char Sec;
       }DateTimeStructure;      //DateTime;

typedef struct
        {
        unsigned char GenState;             // ������� ����� ���������� ON OFF START
        unsigned char LineState;            // ������� ����� �����  ON OFF
        unsigned int  GenBatt;              // ���������� ������� ����������
        unsigned char UPSBatt;              // ���������� ������� ���
        unsigned char GenAttempt;           // ������� ������� ����������   0...3
        unsigned char GenError;             // ������ ���������� TRUE FALSE
        unsigned char Mode;                 // ������ ����� (��� ��������)
        unsigned char Seconds;              // ������� ������� (������������ � Mode)
        unsigned char PowerMode;            // ����� ������� BATT LINE GEN
        unsigned char LastCMD;              // ��������� ������������ ������� 
        unsigned char LastRCV;              // ��������� �������� �����
        unsigned char LastTIM;              // ����� �������� �������������
        unsigned char TRCErr;               // ������� ������ ����������
        } StateStructure;    //Stat

//-------------------------------------------------- ������� ����������
unsigned char StartCmd[] = "G_ON";
unsigned char StopCmd[]  = "GOFF";
unsigned char ErrorCmd[] = "ERRO";
unsigned char BattCmd[]  = "BATT";
//-------------------------------------------------- ������� ����������
volatile unsigned char cSec=0, cStatus=0; //, cGenBatt=0;
volatile unsigned char cRecvBuf[5], cRecvCounter=0; //, cGenBatt=0;
DateTimeStructure DateTime;
volatile StateStructure State;



interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
    TCNT2 = 0x64;   //reload counter value  50Hz
    #asm("wdr");

    if( cSec==0  || cSec==20 || cSec==40 )          //������ 0.3 �������
    {
        cStatus SetBit(DATETIME);
    }

    if(cSec==50)    // 1 �������
    {
        cSec=0;
        if( State.Mode!=STOP )  State.Seconds=State.Seconds+1;
        
        if( State.Mode==STOP && State.Seconds!=0 )  State.Seconds=0;
        
        cStatus SetBit(BATT_IBP);
    }
    
    if(State.LastTIM>0)     State.LastTIM=State.LastTIM+1;  //������ ����� ����� �������� ������

    cSec=cSec+1;
}


interrupt [EXT_INT1] void ext_int1_isr(void)
{
    unsigned char cData=0, i;
    
    EIMSK ClearBit(1);

    LEDON();
    cData=SpiRead();
    if(cRecvCounter==0)
    {
        for(i=0; i<5; i++)     cRecvBuf[i]=0;
    }
    cRecvBuf[cRecvCounter]=cData;                
    cRecvCounter=cRecvCounter+1;

    if(cRecvCounter==4)
    {
        cRecvCounter=0;
        cRecvBuf[4]=0;
        FIFOReset();
        cStatus SetBit(TRC);
    }

    LEDOFF();
    EIMSK SetBit(1);

}



void timer2_init(void)
{
 TCCR2 = 0x00; //stop
 TCNT2 = 0x64; //setup
 OCR2  = 0x9C;
 TCCR2 = 0x05; //start
}



// �������� �������� ������
void LookForSend()
{
unsigned char cStr[]="12345";

    State.LastTIM=0;                // ��������� ������� ����������
    State.TRCErr=State.TRCErr+1;    // ��������� ������� ������
    uc2str(State.TRCErr, cStr);
//    LCD_PUTSF(55,40," ");           // ����� �������� ������/����������
    LCD_PUTS(60,40,cStr);           // ����� �������� ������/����������
    GenCMD(State.LastCMD);

    if(State.TRCErr>=15)
    {
        State.LastTIM=0;            // ��������� ������� ����������
        LCD_PUTSF(40,40,"ERR");
        State.TRCErr=0;
        State.LastCMD=5;
        State.LastRCV=5;
    }
}





void ShowPowerMode()
{
           
    switch (State.PowerMode) 
    {
            case LINE:
                LCD_PUTSF(35,05,"����� ");
                LCD_PUTSF(40,10,"��� ");              // ������� ��������� �����
                LCD_PUTSF(40,20,"���� ");             // ������� ��������� ����������
                break;
            case BATT:
                LCD_PUTSF(35,05,"������");
                LCD_PUTSF(40,20,"���� ");             // ������� ��������� ����������
                LCD_PUTSF(40,10,"���� ");             // ������� ��������� �����
                break;
            case GEN:
                LCD_PUTSF(35,05,"YAMAHA");
                LCD_PUTSF(40,20,"��� ");              // ������� ��������� ����������
                LCD_PUTSF(40,10,"���� ");             // ������� ��������� �����
                break;
            default:
                LCD_PUTSF(35,05,"      ");
    }
}




void main(void)
{
unsigned char i, cStr[]="123456789";
unsigned int BattADCW=0, iUBatt=0;

float Ubat=0;

InitAVR();

#pragma optsize-
WDTCR=0x1F;
WDTCR=0x0F;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif



MCUCSR = 0x80;                              // JTAG OFF
MCUCSR = 0x80;                              // JTAG OFF
ADMUX=ADC_VREF_TYPE & 0xff;                 // ADC
ADCSRA=0x83;                                // ADC
PORTF = 0x00;
DDRF  = (1<<2) | (1<<3);                    // PF2, PF3 -> ����� �� ����
DDRB  = (1<<4) | (1<<7);                    // PB4 - Buzer  PB7 - LED
//DDRE  = (1<<4) | (1<<5);                    // ���� ����� � ����������
DDRD  = (1<<0);                             // ����� ����

State.PowerMode=10;     // ����� ������� ����������.
State.UPSBatt=990;      // ����� ������� ����������
State.GenState=STOP;    // ��������� ����������

LCD_INIT();  
PaintMetod_KS1080=MET_OR;
LCD_CLS();
PaintMetod_KS1080=MET_CLS;

LCD_PUTSF(00,05,"������");    

//DS1302_SetTime(21, 37, 00);

    DS1302_GetTime(&DateTime.Hour, &DateTime.Min,   &DateTime.Sec);
    DS1302_GetDate(&DateTime.Day,  &DateTime.Month, &DateTime.Year);
    DateTime.Sec=88;
    DateTime.Min=88;
    DateTime.Hour=88;
    delay_ms(200);
LCD_PUTSF(60,05,"OK");


 
LCD_PUTSF(00,10,"���������");
TrcPortInit();
TrcInitSnd();
//GenCMD(BATT);                           // ������ ���������� �� Yamaha
//TrcInitSnd();
TrcSendPocket(BattCmd, 4);
TrcInitRcv();
FIFOReset();
LCD_PUTSF(60,10,"��");              // ������� ��������� �����

LCD_PUTSF(00,20,"���� 1");

LEDOFF();

      delay_ms(500);
      AC_GEN_ON();
      delay_ms(500);
      AC_GEN_OFF();
    LCD_PUTSF(60,20,"��");
    LCD_PUTSF(00,25,"���� 2");
      delay_ms(500);
      
      AC_LINE_ON();
      delay_ms(500);
      AC_LINE_OFF();
    LCD_PUTSF(60,25,"��");
    LCD_PUTSF(00,35,"����");
      delay_ms(500);

      for(i=0; i<2; i++)
      {
        LEDON();
        Beep();
        delay_ms(100);
        LEDOFF();
        delay_ms(125);
      }
LCD_PUTSF(60,35,"��");
delay_ms(500);

PaintMetod_KS1080=MET_OR;
LCD_CLS();
LCD_Paint();
//------------------------------------------------------------------------------------------
//TrcInitSnd();
//TrcSendPocket(BattCmd, 4);         
//TrcInitRcv();
//FIFOReset();
GenCMD(BATT);                   // ������ ���������� �� Yamaha
//------------------------------------------------------------------------------------------

// INT1 Rising edge
EICRA=0x0C;
EICRB=0x00;
EIMSK=0x02;
EIFR=0x02;

timer2_init();
TIMSK = 0x40; //timer interrupt sources
ETIMSK = 0x00; //extended timer interrupt sources
#asm("sei")

while (1)
      {
      
        if(State.LastTIM>=100)              // ��� ������ �� ���������� ����������
        {
            LookForSend();
        }

//-------------------------------------------------------------------- ��������� ������        
        if( IsBitSet(cStatus, TRC))         // ������ ������ �� ����������
        {
            cStatus ClearBit(TRC);
            State.LastTIM=0;                // �������� ������ �������� ������ �� TRC

            LCD_PUTSF(30,60,"     ");       // ������� ������ ������
            LCD_PUTS(30,60,cRecvBuf);       // ����� ��������� ������
            

            if( (cRecvBuf[0]-0x30)>=0 && (cRecvBuf[0]-0x30)<=9)         // ������ ����������
            {
                State.LastRCV=BATT;
                State.GenBatt=atoi(cRecvBuf);
                Beep();
            }
            
            if( cCompare(cRecvBuf, StartCmd) )  State.LastRCV=START;    // ������ ������� Start
            if( cCompare(cRecvBuf, StopCmd) )   State.LastRCV=STOP;     // ������ ������� Stop
            if( cCompare(cRecvBuf, ErrorCmd) )
            {
                State.GenAttempt=State.GenAttempt+1;
                if(State.GenAttempt>=3)
                {
                     State.GenError=TRUE;    // ��� ���� ������ ������ ����������
                     State.GenState=STOP;    // ���� ��������� ���������� 
                }
            }
            
            if( State.LastCMD != State.LastRCV )                // ������� �� ������ � ���������, ������
            {
                LookForSend();
            }
            else
            {
                LCD_PUTSF(40,40,"OK    ");
                State.TRCErr=0;                             // ����� �������� ������
                for(i=0; i<5; i++)     cRecvBuf[i]=0;       // ������� ������ ������

            }
        }


        if( IsBitSet(cStatus, BATT_IBP))      // ������ ���������� �� ������� ���
        {
            cStatus ClearBit(BATT_IBP);
            
            ShowPowerMode();                  // ����� �������� ������ �������
            
            BattADCW = Read_ADC(0);
            itoa(BattADCW,cStr);
            LCD_PUTSF(50,25," ");             // ���������� �� ������������� ���
            Ubat= ( (15*(float)BattADCW)/576 );
            ftoa(  Ubat, 1 ,cStr);
            LCD_PUTS(35,25,cStr);             // ���������� �� ������������� ���
            
            iUBatt = (int) (Ubat*10);         // ���������� �� ������������� ��� ������� � ���������
            State.UPSBatt=iUBatt;

            Ubat= (float) State.GenBatt/10;
            ftoa(  Ubat, 1 ,cStr);
            LCD_PUTS(35,35,cStr);             // ���������� �� ������������� ���


            if(DateTime.Sec==0 ||  State.GenBatt==0)    //  ������ ���������� � ������ ������
            {
              if(State.LastTIM==0)          // ��� ������� ��������
              {
                if(State.GenState!=START)   // ��������� �� ��������� � ������ �������
                {
                    GenCMD(BATT);           // ������ ��������� ����������
                }
              }
            }
        }
        
        if( IsBitSet(cStatus, DATETIME) )       // �������� ���� �����
        {        
            cStatus ClearBit(DATETIME);
            ShowDateTime();
        }

//----- ��������� ������ -- ����� ------------------------------------------------------------        

//-------------------------------------------------------------------- ���� ������� �� �����        
            if( IS_LINE_ON() && State.PowerMode!=LINE )
            {
                if( State.Mode!=TIMERLINE )     // ������ ������ ��� ��������� �����
                {
                    State.Seconds=0;
                    State.Mode=TIMERLINE;
                }
                        
                if( State.Mode==TIMERLINE )
                {
                    if( State.Seconds>=10 )     // ������ �������, �������� �����
                    {
                        LCD_PUTSF(65,10,"  ");
                        State.Mode=STOP;        // ���� ��������� ��������� 
                        AC_GEN_OFF();           // ��������� ��������� �� �����
                        while(IS_COMMON_ON());  // �������� ���������� ����
                        delay_ms(100);          // ����� �� ������ ������
                        AC_LINE_ON();           // ���������� �����
                        State.PowerMode=LINE;   // ���� ������� �� �����
                        ShowPowerMode();        // ����� �������� ������ �������
                    }
                    else
                    {
                        uc2str( 10-State.Seconds ,cStr);
                        LCD_PUTS(65,10,cStr);
                    }
                }
            }
            else
//-------------------------------------------------------------------- ��� ������� �� �����
            {
// ������� �� ����� ���, ���� ����� ������ ����������� � �����, ���������� ��� � �������� �������
                if(State.Mode==TIMERLINE)
                {
                    State.Mode=STOP;
                    State.Seconds=0;
                    LCD_PUTSF(65,10,"  ");
                }
                
// ��������� �������, ��������� ����� ������� ����������, ������ ������� �������� ����������
                if( IS_GEN_ON() && State.GenState!=ON )
                {
                    State.GenState=ON;
                    State.GenAttempt=0;
                    State.Seconds=0;
                    State.Mode=TIMERWAITHEAT;           // ���� ������� ������� �������� ����������
                    LCD_PUTSF(40,20,"��� ");             // ������� ��������� ����������
                }
// ���������� ��������� ����������. ����� ��������� ����������
                if( !IS_GEN_ON() && State.GenState==ON )
                {
                    State.GenState=OFF;
                    State.GenAttempt=0;
                    LCD_PUTSF(40,20,"����  ");           // ������� ��������� ����������
                }
                

                if( IS_GEN_ON() && State.Mode==TIMERWAITHEAT && State.GenState==ON )
                {
                    if( State.Seconds>=90 )             // ��������� �������� ��� 90 ������
                    {
                        AC_LINE_OFF();          // ��������� ��������� �� �����
                        while(IS_COMMON_ON());  // �������� ���������� ����
                        delay_ms(100);          // ����� �� ������ ������
                        AC_GEN_ON();            // ���������� �����
                        State.PowerMode=GEN;    // ���� ������� �� ����������
                        ShowPowerMode();        // ����� �������� ������ �������
                    }
                    else
                    {
                        uc2str( 90-State.Seconds ,cStr);    // ������ ������� �������� ����������
                        LCD_PUTS(65,20,cStr);
                    }
                }
                
                    
// ��������� ��������, ����� ���������
                if( !IS_GEN_ON() && !IS_LINE_ON() )     // ��������� ��������, ����� ���������
                {
                    if(State.PowerMode!=BATT)           // ������� �� �������
                    {
                        AC_LINE_OFF();                  // ��������� �����
                        AC_GEN_OFF();                   // ��������� ��������� �� �����
                        while(IS_COMMON_ON());          // �������� ���������� ����
                        delay_ms(100);                  // ����� �� ������ ������
                        State.PowerMode=BATT;           // ���� ������� �� �������
                    }
                }
                
// ������� �� �������, ������ ���������� ���, ������ ������� �������� ����������
                if( State.PowerMode==BATT && State.GenError==FALSE) // ������� �� �������
                {                                                   // ������ ���������� ���
                    if( State.UPSBatt<=150 )            // ���������� ������� ����� �� 15 �����
                    {
                        if(State.Mode!=TIMERGENWAIT)    // ������ �������� ������� ����������
                        {
                            if(State.GenState==STOP)
                            {
                                if( State.GenAttempt==0 )    State.Seconds=28;
                                else                         State.Seconds=0;
//                                State.GenState=START;
                                State.Mode=TIMERGENWAIT;
                            }
                        }
                    }
                }

// ��������� ������� �������� ������� ����������                
                if( State.Mode==TIMERGENWAIT )
                {
                    if( State.Seconds>=30 )     // ������ �������, ������ ����������
                    {
                        State.GenAttempt=1;
                        State.Mode=STOP;        // ���� ��������� ��������� �������
                        GenCMD(START);          // ������� ������� ����������
                        State.GenState=START;   // ���� ������� ���������� 
                        LCD_PUTSF(65,20,"  ");
                    }
                    else
                    {
                        uc2str( 30-State.Seconds ,cStr);
                        LCD_PUTS(65,20,cStr);
                    }
                }
                
            }        

      };
}




void GenCMD(char CMD)
{
    LEDON();
    TrcInitSnd();
    switch (CMD) 
    {
    case START:
        State.GenState=START;
        State.LastCMD=START;
        State.LastRCV=0;
        State.LastTIM=1;
        TrcSendPocket(StartCmd, 4);         // ��������� ������� �����
//        LCD_PUTSF(00,60,"     ");           // ������� ������ ������
        LCD_PUTS(00,60,StartCmd);
        break;
    case STOP:
        State.LastCMD=STOP;
        State.LastRCV=0;
        State.LastTIM=1;
        TrcSendPocket(StopCmd, 4);          // ��������� ������� �����
//        LCD_PUTSF(00,60,"     ");           // ������� ������ ������
        LCD_PUTS(00,60,StopCmd);
        break;
    case BATT:
        State.LastCMD=BATT;
        State.LastRCV=0;
        State.LastTIM=1;
        TrcSendPocket(BattCmd, 4);          // ��������� ������� �����
//        LCD_PUTSF(00,60,"     ");           // ������� ������ ������
        LCD_PUTS(00,60,BattCmd);
        break;
    }; 
    TrcInitRcv();
    FIFOReset();
    LEDOFF();
}



unsigned char cCompare(unsigned char *Buf1, unsigned char *Buf2)
{ 
	while(*Buf2)
		if(*Buf1++ != *Buf2++)	return 0;
	return 1;
}



void uc2str(unsigned char ucVal,unsigned char  *cStr)
{
	unsigned char cVal;
    char cCount;

	cVal=ucVal;
    cCount=0;
	if(cVal>99)
	{
		while(cVal>=100)
		{
			cVal=cVal-100;
			cCount=cCount+1;
		}
		*cStr=cCount+0x30;
		cStr++;
	}

	cCount=0;
	while(cVal>=10)
	{
		cVal=cVal-10;
		cCount=cCount+1;
	}
	*cStr=cCount+0x30;
//	cCount=0;
	cStr++;

	*cStr=cVal+0x30;
//	ucCount=0;
	cStr++;
	*cStr=0;
}



void ShowDate()
{
unsigned char cStr[]="123456789";

    PaintMetod_KS1080=MET_CLS;
    DS1302_GetDate(&DateTime.Day, &DateTime.Month, &DateTime.Year);
    uc2str(DateTime.Day,cStr);
    LCD_PUTS(80,10,cStr);

    uc2str(DateTime.Month,cStr);
    LCD_PUTS(98,10,cStr);

    uc2str(DateTime.Year,cStr);
    LCD_PUTS(116,10,cStr);
}





void ShowDateTime()
{
unsigned char cStr[]="123456789", cSec=0;
DateTimeStructure DT;

    PaintMetod_KS1080=MET_CLS;
    cSec=DS1302_Read(DS1302_SECREAD);      //�������
    cSec=bcd2dec(cSec);
    if(cSec < DateTime.Sec)
    {
        DS1302_GetTime(&DT.Hour, &DT.Min, &DT.Sec);
    }
    else
    {
        DT.Sec=cSec;
        DT.Min=DateTime.Min;
        DT.Hour=DateTime.Hour;
    }

    if(DateTime.Sec != DT.Sec)
    {
        uc2str(DT.Sec,cStr);
        LCD_PUTS(116,05,cStr);
        DateTime.Sec=DT.Sec;
    }

    if(DateTime.Min != DT.Min)
    {
        uc2str(DT.Min,cStr);
        LCD_PUTS(98,05,cStr);
        DateTime.Min=DT.Min;
    }

    if(DateTime.Hour != DT.Hour)
    {
        uc2str(DT.Hour,cStr);
        LCD_PUTS(80,05,cStr);
        DateTime.Hour=DT.Hour;
    }

    if( DateTime.Sec==0 && DateTime.Min==0)
    {
        ShowDate();
    }
}


// Read the AD conversion result
unsigned int Read_ADC(unsigned char adc_input)
{
    ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
    delay_us(10);
    ADCSRA|=0x40;
    while ((ADCSRA & 0x10)==0);
    ADCSRA|=0x10;
    return ADCW;
}



void Beep()
{
    unsigned char i;
    
    for(i=0; i<100; i++)
    {
        PORTB SetBit(4);
        delay_us(100);
        PORTB ClearBit(4);
        delay_us(50);
    }
}

void LCD_Paint()
{
    PaintMetod_KS1080=MET_OR;
    LCD_LINE(77, 05,  77, 55);
    LCD_LINE(80, 20, 128, 20);
    
    PaintMetod_KS1080=MET_CLS;
    LCD_PUTSF(00,05,"�����");    
//    LCD_PUTSF(35,05,"�����");

    LCD_PUTSF(00,10,"�����");
//    LCD_PUTSF(40,10,"���");              // ������� ��������� �����
    LCD_PUTSF(40,10,"����");              // ������� ��������� �����
    
    LCD_PUTSF(00,20,"Yamaha");
    LCD_PUTSF(40,20,"����");             // ������� ��������� ����������

    LCD_PUTSF(00,25,"U ���");    
    LCD_PUTSF(35,25,"     �");          // ���������� �� ������������� ���
//    LCD_PUTSF(35,25,"28.2 �");          // ���������� �� ������������� ���
    LCD_PUTSF(00,35,"U ���");    
//    LCD_PUTSF(35,35,"12.5 �");          // ���������� �� ������������ ����������
    LCD_PUTSF(35,35,"     �");          // ���������� �� ������������ ����������

    LCD_PUTSF(00,40,"�����");
//    LCD_PUTSF(40,40,"OK");              // ������� ��������� �����
//    LCD_PUTSF(00,50,"�������");
//    LCD_PUTSF(45,50,"STOP");
    


//    LCD_PUTSF(80,05,"18:32:01");        // ������� �����
//    LCD_PUTSF(80,10,"16.01.10");        // ������� ����
    LCD_PUTSF(80,05,"  :  :  ");        // ������� �����
    LCD_PUTSF(80,10,"  .  .  ");        // ������� ����
    ShowDate();

    LCD_PUTSF(85,25,"Yamaha");
    LCD_PUTSF(80,40,"16.01.10");        // ��������� ���� ������
    LCD_PUTSF(80,50,"ON");
    LCD_PUTSF(99,50,"18:32");           // ����� ������
    LCD_PUTSF(80,60,"OF");
    LCD_PUTSF(99,60,"18:34");           // ����� �����

}
