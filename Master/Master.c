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

// Режим генератора
#define OFF         0 
#define ON          1 
#define START       2 

// Текущий режим питания (батарея, линия, генератор)
#define LINE        0 
#define BATT        1 
#define GEN         2

// Для cStatus
#define TRC         0
#define BATT_IBP    1  
#define DATETIME    2  

//Для Mode
#define STOP            0
#define TIMERLINE       1
#define TIMERGENON      2
#define TIMERGENOFF     2
#define TIMERGENWAIT    3
#define TIMERWAITHEAT   4

#define IS_LINE_ON()    IsBitClear(PINE,5)      //Напряжение на линии PINE 5    
#define IS_GEN_ON()     IsBitClear(PINE,4)      //Напряжение на генераторе PINE 4
#define IS_COMMON_ON()  IsBitClear(PIND,0)      //Общий вход активен PIND 0    

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
        unsigned char GenState;             // Текущйи режим генератора ON OFF START
        unsigned char LineState;            // Текущий режим линии  ON OFF
        unsigned int  GenBatt;              // Напряжение батареи генератора
        unsigned char UPSBatt;              // Напряжение батареи ИБП
        unsigned char GenAttempt;           // Попытка запуска генератора   0...3
        unsigned char GenError;             // Ошибка генератора TRUE FALSE
        unsigned char Mode;                 // Текщий режим (для таймеров)
        unsigned char Seconds;              // Секунды таймера (используется с Mode)
        unsigned char PowerMode;            // Режим питания BATT LINE GEN
        unsigned char LastCMD;              // Последняя отправленная команда 
        unsigned char LastRCV;              // Последний принятый ответ
        unsigned char LastTIM;              // Время ожидания подтверждения
        unsigned char TRCErr;               // Счетчик ошибок трансивера
        } StateStructure;    //Stat

//-------------------------------------------------- Команды генератора
unsigned char StartCmd[] = "G_ON";
unsigned char StopCmd[]  = "GOFF";
unsigned char ErrorCmd[] = "ERRO";
unsigned char BattCmd[]  = "BATT";
//-------------------------------------------------- Команды генератора
volatile unsigned char cSec=0, cStatus=0; //, cGenBatt=0;
volatile unsigned char cRecvBuf[5], cRecvCounter=0; //, cGenBatt=0;
DateTimeStructure DateTime;
volatile StateStructure State;



interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
    TCNT2 = 0x64;   //reload counter value  50Hz
    #asm("wdr");

    if( cSec==0  || cSec==20 || cSec==40 )          //прошло 0.3 секунды
    {
        cStatus SetBit(DATETIME);
    }

    if(cSec==50)    // 1 секунда
    {
        cSec=0;
        if( State.Mode!=STOP )  State.Seconds=State.Seconds+1;
        
        if( State.Mode==STOP && State.Seconds!=0 )  State.Seconds=0;
        
        cStatus SetBit(BATT_IBP);
    }
    
    if(State.LastTIM>0)     State.LastTIM=State.LastTIM+1;  //считаю время после отправки пакета

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



// Проверка отправки пакета
void LookForSend()
{
unsigned char cStr[]="12345";

    State.LastTIM=0;                // Остановка таймера трансивера
    State.TRCErr=State.TRCErr+1;    // Увеличить счетчик ошибок
    uc2str(State.TRCErr, cStr);
//    LCD_PUTSF(55,40," ");           // Вывод счетчика ошибок/повторений
    LCD_PUTS(60,40,cStr);           // Вывод счетчика ошибок/повторений
    GenCMD(State.LastCMD);

    if(State.TRCErr>=15)
    {
        State.LastTIM=0;            // Остановка таймера трансивера
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
                LCD_PUTSF(35,05,"ЛИНИЯ ");
                LCD_PUTSF(40,10,"ВКЛ ");              // Текущее состояние Линии
                LCD_PUTSF(40,20,"ВЫКЛ ");             // Текущее состояние Генератора
                break;
            case BATT:
                LCD_PUTSF(35,05,"РЕЗЕРВ");
                LCD_PUTSF(40,20,"ВЫКЛ ");             // Текущее состояние Генератора
                LCD_PUTSF(40,10,"ВЫКЛ ");             // Текущее состояние Линии
                break;
            case GEN:
                LCD_PUTSF(35,05,"YAMAHA");
                LCD_PUTSF(40,20,"ВКЛ ");              // Текущее состояние Генератора
                LCD_PUTSF(40,10,"ВЫКЛ ");             // Текущее состояние Линии
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
DDRF  = (1<<2) | (1<<3);                    // PF2, PF3 -> Выход на реле
DDRB  = (1<<4) | (1<<7);                    // PB4 - Buzer  PB7 - LED
//DDRE  = (1<<4) | (1<<5);                    // Вход линии и генератора
DDRD  = (1<<0);                             // Общий вход

State.PowerMode=10;     // Режим питания неизвестен.
State.UPSBatt=990;      // Заряд батарей неизвестен
State.GenState=STOP;    // Генератор остановлен

LCD_INIT();  
PaintMetod_KS1080=MET_OR;
LCD_CLS();
PaintMetod_KS1080=MET_CLS;

LCD_PUTSF(00,05,"Таймер");    

//DS1302_SetTime(21, 37, 00);

    DS1302_GetTime(&DateTime.Hour, &DateTime.Min,   &DateTime.Sec);
    DS1302_GetDate(&DateTime.Day,  &DateTime.Month, &DateTime.Year);
    DateTime.Sec=88;
    DateTime.Min=88;
    DateTime.Hour=88;
    delay_ms(200);
LCD_PUTSF(60,05,"OK");


 
LCD_PUTSF(00,10,"Трансивер");
TrcPortInit();
TrcInitSnd();
//GenCMD(BATT);                           // Запрос напряжения на Yamaha
//TrcInitSnd();
TrcSendPocket(BattCmd, 4);
TrcInitRcv();
FIFOReset();
LCD_PUTSF(60,10,"ОК");              // Текущее состояние Линии

LCD_PUTSF(00,20,"Реле 1");

LEDOFF();

      delay_ms(500);
      AC_GEN_ON();
      delay_ms(500);
      AC_GEN_OFF();
    LCD_PUTSF(60,20,"ОК");
    LCD_PUTSF(00,25,"Реле 2");
      delay_ms(500);
      
      AC_LINE_ON();
      delay_ms(500);
      AC_LINE_OFF();
    LCD_PUTSF(60,25,"ОК");
    LCD_PUTSF(00,35,"Звук");
      delay_ms(500);

      for(i=0; i<2; i++)
      {
        LEDON();
        Beep();
        delay_ms(100);
        LEDOFF();
        delay_ms(125);
      }
LCD_PUTSF(60,35,"ОК");
delay_ms(500);

PaintMetod_KS1080=MET_OR;
LCD_CLS();
LCD_Paint();
//------------------------------------------------------------------------------------------
//TrcInitSnd();
//TrcSendPocket(BattCmd, 4);         
//TrcInitRcv();
//FIFOReset();
GenCMD(BATT);                   // Запрос напряжения на Yamaha
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
      
        if(State.LastTIM>=100)              // Нет ответа от трансивера генератора
        {
            LookForSend();
        }

//-------------------------------------------------------------------- Обработка флагов        
        if( IsBitSet(cStatus, TRC))         // Пришли данные из трансивера
        {
            cStatus ClearBit(TRC);
            State.LastTIM=0;                // Сбросить таймер ожидания ответа от TRC

            LCD_PUTSF(30,60,"     ");       // Очистка статус строки
            LCD_PUTS(30,60,cRecvBuf);       // Вывод пришедших данных
            

            if( (cRecvBuf[0]-0x30)>=0 && (cRecvBuf[0]-0x30)<=9)         // Пришло напряжение
            {
                State.LastRCV=BATT;
                State.GenBatt=atoi(cRecvBuf);
                Beep();
            }
            
            if( cCompare(cRecvBuf, StartCmd) )  State.LastRCV=START;    // Пришла команда Start
            if( cCompare(cRecvBuf, StopCmd) )   State.LastRCV=STOP;     // Пришла команда Stop
            if( cCompare(cRecvBuf, ErrorCmd) )
            {
                State.GenAttempt=State.GenAttempt+1;
                if(State.GenAttempt>=3)
                {
                     State.GenError=TRUE;    // Три раза пришла ошибка генератора
                     State.GenState=STOP;    // Флаг остановки генератора 
                }
            }
            
            if( State.LastCMD != State.LastRCV )                // Команда не пришла к приемнику, повтор
            {
                LookForSend();
            }
            else
            {
                LCD_PUTSF(40,40,"OK    ");
                State.TRCErr=0;                             // Сброс счетчика ошибок
                for(i=0; i<5; i++)     cRecvBuf[i]=0;       // Очистка буфера приема

            }
        }


        if( IsBitSet(cStatus, BATT_IBP))      // Запрос напряжения на батарее ИБП
        {
            cStatus ClearBit(BATT_IBP);
            
            ShowPowerMode();                  // Вывод текущего режима питания
            
            BattADCW = Read_ADC(0);
            itoa(BattADCW,cStr);
            LCD_PUTSF(50,25," ");             // Напряжение на аккумуляторах ИБП
            Ubat= ( (15*(float)BattADCW)/576 );
            ftoa(  Ubat, 1 ,cStr);
            LCD_PUTS(35,25,cStr);             // Напряжение на аккумуляторах ИБП
            
            iUBatt = (int) (Ubat*10);         // Напряжение на аккумуляторах для анализа и сравнения
            State.UPSBatt=iUBatt;

            Ubat= (float) State.GenBatt/10;
            ftoa(  Ubat, 1 ,cStr);
            LCD_PUTS(35,35,cStr);             // Напряжение на аккумуляторах ИБП


            if(DateTime.Sec==0 ||  State.GenBatt==0)    //  Запрос напряжения в начале минуты
            {
              if(State.LastTIM==0)          // Все команды переданы
              {
                if(State.GenState!=START)   // Генератор не находится в режиме запуска
                {
                    GenCMD(BATT);           // Запрос измерения напряжения
                }
              }
            }
        }
        
        if( IsBitSet(cStatus, DATETIME) )       // Обновить дату время
        {        
            cStatus ClearBit(DATETIME);
            ShowDateTime();
        }

//----- Обработка флагов -- КОНЕЦ ------------------------------------------------------------        

//-------------------------------------------------------------------- Есть питание на линии        
            if( IS_LINE_ON() && State.PowerMode!=LINE )
            {
                if( State.Mode!=TIMERLINE )     // начать отсчет для включения линии
                {
                    State.Seconds=0;
                    State.Mode=TIMERLINE;
                }
                        
                if( State.Mode==TIMERLINE )
                {
                    if( State.Seconds>=10 )     // отсчет окончен, включить линию
                    {
                        LCD_PUTSF(65,10,"  ");
                        State.Mode=STOP;        // Флаг остановки счетчиков 
                        AC_GEN_OFF();           // Отключить генератор от линии
                        while(IS_COMMON_ON());  // Ожидание отключения реле
                        delay_ms(100);          // Пауза на всякий случай
                        AC_LINE_ON();           // Подключить линию
                        State.PowerMode=LINE;   // Флаг питания из линии
                        ShowPowerMode();        // Вывод текущего режима питания
                    }
                    else
                    {
                        uc2str( 10-State.Seconds ,cStr);
                        LCD_PUTS(65,10,cStr);
                    }
                }
            }
            else
//-------------------------------------------------------------------- Нет питания на линии
            {
// Питания на линии нет, если велся отсчет подключения к линии, остановить его и сбросить счетчик
                if(State.Mode==TIMERLINE)
                {
                    State.Mode=STOP;
                    State.Seconds=0;
                    LCD_PUTSF(65,10,"  ");
                }
                
// Генератор запущен, установка флага запуска генератора, запуск таймера прогрева генератора
                if( IS_GEN_ON() && State.GenState!=ON )
                {
                    State.GenState=ON;
                    State.GenAttempt=0;
                    State.Seconds=0;
                    State.Mode=TIMERWAITHEAT;           // Флаг запуска таймера прогрева генератора
                    LCD_PUTSF(40,20,"ВКЛ ");             // Текущее состояние Генератора
                }
// Запущенный генератор выключился. Сброс счетчиков генератора
                if( !IS_GEN_ON() && State.GenState==ON )
                {
                    State.GenState=OFF;
                    State.GenAttempt=0;
                    LCD_PUTSF(40,20,"ВЫКЛ  ");           // Текущее состояние Генератора
                }
                

                if( IS_GEN_ON() && State.Mode==TIMERWAITHEAT && State.GenState==ON )
                {
                    if( State.Seconds>=90 )             // генератор работает уже 90 секунд
                    {
                        AC_LINE_OFF();          // Отключить генератор от линии
                        while(IS_COMMON_ON());  // Ожидание отключения реле
                        delay_ms(100);          // Пауза на всякий случай
                        AC_GEN_ON();            // Подключить линию
                        State.PowerMode=GEN;    // Флаг питания от генератора
                        ShowPowerMode();        // Вывод текущего режима питания
                    }
                    else
                    {
                        uc2str( 90-State.Seconds ,cStr);    // таймер отсчета прогрева генератора
                        LCD_PUTS(65,20,cStr);
                    }
                }
                
                    
// Генератор выключен, линия выключена
                if( !IS_GEN_ON() && !IS_LINE_ON() )     // Генератор выключен, линия выключена
                {
                    if(State.PowerMode!=BATT)           // Питание от батареи
                    {
                        AC_LINE_OFF();                  // Отключить линию
                        AC_GEN_OFF();                   // Отключить генератор от линии
                        while(IS_COMMON_ON());          // Ожидание отключения реле
                        delay_ms(100);                  // Пауза на всякий случай
                        State.PowerMode=BATT;           // Флаг питания от батарей
                    }
                }
                
// Питание от батарей, ошибки генератора нет, запуск таймера задержки генератора
                if( State.PowerMode==BATT && State.GenError==FALSE) // питание от батарей
                {                                                   // ошибки генератора нет
                    if( State.UPSBatt<=150 )            // напряжение батарей упало до 15 вольт
                    {
                        if(State.Mode!=TIMERGENWAIT)    // Таймер задержки запуска генератора
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

// Обработка таймера задержки запуска генератора                
                if( State.Mode==TIMERGENWAIT )
                {
                    if( State.Seconds>=30 )     // отсчет окончен, запуск генератора
                    {
                        State.GenAttempt=1;
                        State.Mode=STOP;        // Флаг остановки счетчиков таймера
                        GenCMD(START);          // Команда запуска генератора
                        State.GenState=START;   // Флаг запуска генератора 
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
        TrcSendPocket(StartCmd, 4);         // Отправить команду СТАРТ
//        LCD_PUTSF(00,60,"     ");           // Очистка статус строки
        LCD_PUTS(00,60,StartCmd);
        break;
    case STOP:
        State.LastCMD=STOP;
        State.LastRCV=0;
        State.LastTIM=1;
        TrcSendPocket(StopCmd, 4);          // Отправить команду СТАРТ
//        LCD_PUTSF(00,60,"     ");           // Очистка статус строки
        LCD_PUTS(00,60,StopCmd);
        break;
    case BATT:
        State.LastCMD=BATT;
        State.LastRCV=0;
        State.LastTIM=1;
        TrcSendPocket(BattCmd, 4);          // Отправить команду СТАРТ
//        LCD_PUTSF(00,60,"     ");           // Очистка статус строки
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
    cSec=DS1302_Read(DS1302_SECREAD);      //секунды
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
    LCD_PUTSF(00,05,"Режим");    
//    LCD_PUTSF(35,05,"ЛИНИЯ");

    LCD_PUTSF(00,10,"Линия");
//    LCD_PUTSF(40,10,"ВКЛ");              // Текущее состояние Линии
    LCD_PUTSF(40,10,"ВЫКЛ");              // Текущее состояние Линии
    
    LCD_PUTSF(00,20,"Yamaha");
    LCD_PUTSF(40,20,"ВЫКЛ");             // Текущее состояние Генератора

    LCD_PUTSF(00,25,"U ибп");    
    LCD_PUTSF(35,25,"     В");          // Напряжение на аккумуляторах ИБП
//    LCD_PUTSF(35,25,"28.2 В");          // Напряжение на аккумуляторах ИБП
    LCD_PUTSF(00,35,"U ген");    
//    LCD_PUTSF(35,35,"12.5 В");          // Напряжение на аккумуляторе Генератора
    LCD_PUTSF(35,35,"     В");          // Напряжение на аккумуляторе Генератора

    LCD_PUTSF(00,40,"Связь");
//    LCD_PUTSF(40,40,"OK");              // Текущее состояние связи
//    LCD_PUTSF(00,50,"Команда");
//    LCD_PUTSF(45,50,"STOP");
    


//    LCD_PUTSF(80,05,"18:32:01");        // Текущее время
//    LCD_PUTSF(80,10,"16.01.10");        // Текущая дата
    LCD_PUTSF(80,05,"  :  :  ");        // Текущее время
    LCD_PUTSF(80,10,"  .  .  ");        // Текущая дата
    ShowDate();

    LCD_PUTSF(85,25,"Yamaha");
    LCD_PUTSF(80,40,"16.01.10");        // Последняя дата старта
    LCD_PUTSF(80,50,"ON");
    LCD_PUTSF(99,50,"18:32");           // Время старта
    LCD_PUTSF(80,60,"OF");
    LCD_PUTSF(99,60,"18:34");           // Время стопа

}
