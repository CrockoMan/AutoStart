#include <mega8.h>
#include <delay.h>
#include <trc.h>

void TrcPortInit() 
{
    TRCDDR = (1<<SDI) | (1<<SCK) | (1<<CS) | (1<<NFSEL);
    
    HI(CS);
    HI(SDI);
    LO(SCK);
    LO(NFSEL);
    delay_ms(2500);
}


void TrcInitRcv()               // Инициализация TRC как приемника
{
    writeCmd(0x80D7); //EL,EF,433band,12.0pF
    writeCmd(0x8299); //er,!ebb,ET,ES,EX,!eb,!ew,DC (init as receiver)
    writeCmd(0xA640); //freq select
    writeCmd(0xC647); //4.8kbps
    writeCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
    writeCmd(0xC2AC); //AL,!ml,DIG,DQD4
    writeCmd(0xCA81); //FIFO8,SYNC,!ff,DR (FIFO level = 8)
    writeCmd(0xCED4); //SYNC=2DD4;
    writeCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
    writeCmd(0x9850); //!mp,90kHz,MAX OUT
    writeCmd(0xCC17); //!OB1,!OB0, LPX,!ddy,DDIT,BW0
    writeCmd(0xE000); //NOT USE
    writeCmd(0xC800); //NOT USE
    writeCmd(0xC000); //1MHz,2.2V
}


void TrcInitSnd()               // Инициализация TRC как передатчика 
{
    writeCmd(0x80D7); //EL,EF,433band,12.0pF
    writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
    writeCmd(0xA640); //frequency select
    writeCmd(0xC647); //4.8kbps
    writeCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
    writeCmd(0xC2AC); //AL,!ml,DIG,DQD4
    writeCmd(0xCA81); //FIFO8,SYNC,!ff,DR
    writeCmd(0xCED4); //SYNC=2DD4 , AG
    writeCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
    writeCmd(0x9850); //!mp,90kHz,MAX OUT
//    writeCmd(0x9855); //!mp,90kHz,MIN OUT
    writeCmd(0xCC17); //OB1 , ACOB0, LPX,Iddy,CDDIT,CBW0
    writeCmd(0xE000); //NOT USED
    writeCmd(0xC800); //NOT USED
    writeCmd(0xC000); //1MHz,2.2V
}



unsigned int writeCmd(unsigned int cmd) 
{
    unsigned char i;
    unsigned int recv=0;
    recv = 0;
    LO(SCK);
    LO(CS);
    for(i=0; i<16; i++) 
    {
        if(cmd&0x8000) HI(SDI); 
        else LO(SDI);
        HI(SCK);
        recv<<=1;
        if( TRCPIN & (1<<SDO) ) 
        {
            recv|=0x0001;
        }
        delay_us(1);
        LO(SCK);
        cmd<<=1;
        delay_us(1);
    }
    HI(CS);
    delay_us(10);
    return recv;
}


void TrcSendByte(unsigned char cData)
{
        writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
        delay_ms(5);

        LO(CS);                 // Признак начала
        HI(NFSEL);              // передачи пакета
        TRCSpiSend(0xB8,0);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x2D,1);
        TRCSpiSend(0xD4,1);
    
        TRCSpiSend(cData,1);     // Передаваемый байт данных
        
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x00,1);
        HI(CS);                 // Передача пакета
        LO(NFSEL);              // окончена

        writeCmd(0x8201); //!er,!ebb,ET,ES,EX,!eb,!ew,DC    // Отключение передатчика
}

void TrcSendPocket(unsigned char *cData, unsigned char cLong)
{
        unsigned char i=0;

        writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
        delay_ms(5);

        LO(CS);                 // Признак начала
        HI(NFSEL);              // передачи пакета
        TRCSpiSend(0xB8,0);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x2D,1);
        TRCSpiSend(0xD4,1);
        
        for(i=0; i<cLong; i++)
        {
            TRCSpiSend(cData[i],1);     // Передаваемый байт данных
        }
        
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x00,1);
        HI(CS);                 // Передача пакета
        LO(NFSEL);              // окончена

        writeCmd(0x8201); //!er,!ebb,ET,ES,EX,!eb,!ew,DC    // Отключение передатчика
}


void TrcSendPocketF(flash char *cData, unsigned char cLong)
{
        unsigned char i=0;

        writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
        delay_ms(5);

        LO(CS);                 // Признак начала
        HI(NFSEL);              // передачи пакета
        TRCSpiSend(0xB8,0);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x2D,1);
        TRCSpiSend(0xD4,1);
        
        for(i=0; i<cLong; i++)
        {
            TRCSpiSend(cData[i],1);     // Передаваемый байт данных
        }
        
        TRCSpiSend(0xAA,1);
        TRCSpiSend(0x00,1);
        HI(CS);                 // Передача пакета
        LO(NFSEL);              // окончена

        writeCmd(0x8201); //!er,!ebb,ET,ES,EX,!eb,!ew,DC    // Отключение передатчика
}




void FIFOReset()
{
    writeCmd(0xCA81);
    writeCmd(0xCA83);
}



void TRCSpiSend(unsigned char data, unsigned char WaitSDO)
{
    unsigned char i;

    if(WaitSDO==1)
    {
        while(WAIT_SDO_HI());
    }
    LO(SCK);
    for(i=0; i<8; i++) 
    {
        if(data&0x80) HI(SDI); 
        else LO(SDI);
        HI(SCK);
        delay_us(1);

        LO(SCK);
        data<<=1;
        delay_us(1);
    }
    delay_us(10);
}



unsigned char SpiRead() 
{
    unsigned char i;
    unsigned char recv=0;
    recv = 0;
    LO(NFSEL);
    LO(SCK);
    for(i=0; i<8; i++) 
    {
        HI(SCK);
        recv<<=1;
        if( TRCPIN & (1<<SDO) ) 
        {
            recv|=0x0001;
        }
        delay_us(1);
        LO(SCK);
        delay_us(1);
    }
    HI(NFSEL);
    delay_us(10);
    return recv;
}


void GetTRCPoket(unsigned char *cArray, unsigned char nLen)
{

    unsigned char i=0, cData=0;
    for(i=0;i<nLen;i++)
    {
        while( WAIT_FINT_HI() );        // Ожидание признака начала пакета FINT == 1 
        if( FINTPIN&(1<<FINT) )         // FINT == 1 
        {
            cData=SpiRead();
            *cArray = cData;
            cArray++;
        }
    }
    delay_ms(50);
}
