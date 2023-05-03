/*****************************************************

Project : 
Version : 
Date    :
Author  :
Company : 
Comments: 


Chip type           : ATmega8
Program type        : Application
Clock frequency     : 1,000000 MHz
Memory model        : Small
External RAM size   : 0
Data Stack size     : 256
*****************************************************/

#include <mega8.h>
#include <delay.h>
#include <TRC.h>

#define MYBOARD

#define SetBit(x)    |= (1<<x) 
#define ClearBit(x)  &=~(1<<x) 

#define IsBitSet(port, bit)   ((port)  & (1 << (bit)))
#define IsBitClear(port, bit) (~(port) & (1 << (bit)))

#define STOP        0 
#define BATT        1 
#define START       2 
#define ERR         3 

#define ENGINE      1
#define STARTER     7
#define ENGINEDETECT 2

#ifdef MYBOARD
    #define LED     1
#else
    #define LED     6
#endif

#ifdef MYBOARD
    #define LEDON()     PORTC ClearBit(LED)
    #define LEDOFF()    PORTC SetBit(LED)
#else
    #define LEDON()     PORTB ClearBit(LED)
    #define LEDOFF()    PORTB SetBit(LED)
#endif

#define GeneratorON()   IsBitClear(PIND, ENGINEDETECT)
#define GeneratorOFF()  IsBitSet(PIND, ENGINEDETECT)

#define EngineON()  PORTB SetBit(ENGINE)    
#define EngineOFF() PORTB ClearBit(ENGINE)

#define StarterON()  PORTB SetBit(STARTER)
#define StarterOFF() PORTB ClearBit(STARTER)

#define Start_ADC() ADCSRA |= (1<<6)                // Установить 6-й бит = начать ADC
#define Wait_ADC()  while( (ADCSRA & (1 << 4))==0 ) // Ожидание окончания ADC

#define BUFERLEN    20

unsigned char StartCmd[] = "G_ON";
unsigned char StopCmd[]  = "GOFF";
unsigned char ErrorCmd[] = "ERRO";
unsigned char BattCmd[]  = "BATT";

volatile unsigned char  cCounter=0, cRecv=0, cRecvBuf[5], EngineWork=0;

interrupt [EXT_INT1] void ext_int1_isr(void)
{
    unsigned char cData=0, i;
    
    GICR ClearBit(7);

    LEDON();
    cData=SpiRead();
    if(cCounter==0)
    {
        for(i=0; i<5; i++)     cRecvBuf[i]=0;
    }
    cRecvBuf[cCounter]=cData;                
    cCounter=cCounter+1;

    if(cCounter==4)
    {
        cCounter=0;
        cRecvBuf[4]=0;
        FIFOReset();
        cRecv=1;
    }

    LEDOFF();
    GICR SetBit(7);
}



unsigned char cCompare(unsigned char *Buf1, unsigned char *Buf2)
{ 
	while(*Buf2)
		if(*Buf1++ != *Buf2++)	return 0;
	return 1;
}


//-------------------------------------------- Процедура запуска двигателя
void StartEngine(void)
{
unsigned char i=0, j=0;    //, StartAttempt=0;

    EngineON();                                 // Зажигание ВКЛ
    for(j=0; j<3; j++)
    {
        delay_ms(1500);
        StarterON();                            // Стартер ВКЛ
        for(i=0; i<30; i++)
        {
            delay_ms(100);
            if( GeneratorON() )                 // Проверка запуска двигателя
            {
                EngineWork=1;
                i=40;
                j=10;
            }
        }
        StarterOFF();                           // Стартер ВЫКЛ
    }
}


void uc2str(unsigned int ucVal,unsigned char  *cStr)
{
	unsigned int cVal;
    char cCount;

	cVal=ucVal;
    cCount=0;
	if(cVal>999)
	{
		while(cVal>=1000)
		{
			cVal=cVal-1000;
			cCount=cCount+1;
		}
		*cStr=cCount+0x30;
		cStr++;
	}

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
	cStr++;

	*cStr=cVal+0x30;
	cStr++;
	*cStr=0;
}


void GenCMD(char CMD)
{
    LEDON();
    delay_ms(20);
    TrcInitSnd();
    switch (CMD) 
    {
    case START:
        TrcSendPocket(StartCmd, 4);         // Отправить команду СТАРТ
        break;
    case STOP:
        TrcSendPocket(StopCmd, 4);         // Отправить команду СТОП
        break;
    case BATT:
        TrcSendPocket(BattCmd, 4);         // Отправить команду БАТАРЕЯ
        break;
    case ERR:
        TrcSendPocket(ErrorCmd, 4);
        break;
    }; 
    TrcInitRcv();
    FIFOReset();
    LEDOFF();
}


void main(void)
{
unsigned int Result=0;
unsigned char cString[20];
float Ubat;

PORTB=0b11111111;
PORTC=0b111111;
PORTD=0b11111111;

#ifdef MYBOARD
PORTC=(1<<3);
DDRC = (1<<LED);                                // Led

DDRC  ClearBit(3);                              // Кнопка Test
//PORTC SetBit(3);                                // PullUp для кнопки Test

PORTB =(0<<ENGINE) | (0<<STARTER);
DDRB = (1<<ENGINE) | (1<<STARTER);              // Пины Зажигания, Стартера - выходы

#else

//PORTB =0;
PORTB =(0<<ENGINE) | (0<<STARTER) | (0<<LED);
DDRB = (1<<ENGINE) | (1<<STARTER) | (1<<LED);   // Пины Зажигания, Стартера, Led - выходы

DDRC  ClearBit(3);                              // Кнопка Test
//PORTC SetBit(3);                                // PullUp для кнопки Test
#endif


TrcPortInit();
TrcInitSnd();
TrcSendPocketF("REDY", 4);
TrcInitRcv();
FIFOReset();
LEDOFF();

// ADC на ADC0, Int VRef 2.56V
ADMUX =0b11000000;
ADCSRA=0b10000011;
//ADCSRA=0b10000011;

// INT1 по переднему фронту
GICR =(1<<7);
MCUCR=0b00001100;
GIFR=0b10000000;
#asm("sei");



while (1)
    {

        // Команда измерения напряжения аккумулятора
        if( cRecv==1 && cCompare(cRecvBuf, BattCmd))
        {
//            LEDON();
            GenCMD(BATT);
            cRecv=0;
            Start_ADC();
            Wait_ADC();
            Result=0;
//-----------------------------------------------------------
            Ubat= ( (12*(float)ADCW)/783 );
            Result = (int) (Ubat*10);       // Напряжение на аккумуляторах для анализа и сравнения
            uc2str(Result, cString);
//-----------------------------------------------------------
            TrcInitSnd();
            TrcSendPocket(cString, 4);
            TrcInitRcv();
            FIFOReset();
//            LEDOFF();
        }

        // Команда СТАРТ
        if( cRecv==1 && cCompare(cRecvBuf, StartCmd) && EngineWork==0 )
        {
            cRecv=0;
            GenCMD(START);
            if( GeneratorOFF() )                            // Двигатель вручную НЕ ЗАПУЩЕН, начать Старт 
            {
                cRecv=0;
                StartEngine();
                
                if(EngineWork==1)                           // Двигатель ЗАВЕЛСЯ!
                {
                    GenCMD(START);
                }
                else                                        // Двигатель НЕ завелся!
                {
                    EngineOFF();                            // Зажигание ВЫКЛ
                    GenCMD(ERR);
                }
            }
        }
        


        if(EngineWork==1 && GeneratorOFF() )        // Двигатель заглох САМ !!!
        {
            EngineWork=0;
            StarterOFF();                           // Стартер ВЫКЛ
            EngineOFF();                            // Двигатель ВЫКЛ

            GenCMD(ERR);
        }



        if( cRecv==1 && cCompare(cRecvBuf, StopCmd) && EngineWork==1 )     // Команда СТОП
        {
            EngineWork=0;
            StarterOFF();                           // Стартер ВЫКЛ
            EngineOFF();                            // Двигатель ВЫКЛ

            GenCMD(STOP);
        }


        if(EngineWork==0 && GeneratorOFF() )
        {
            if( IsBitClear(PINC, 3) )       // Нажатие кнопки ТЕСТ
            {
                delay_ms(30);               // Устранение дребезга
                if( IsBitClear(PINC, 3) )   // Нажатие кнопки ТЕСТ
                {
                    EngineWork=1;
                    StartEngine();
                    delay_ms(10000);
                    EngineOFF();                            // Двигатель ВЫКЛ
                    EngineWork=0;
                }
            }
        }
    };
}
