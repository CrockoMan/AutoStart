#define TRCPORT PORTD
#define TRCDDR  DDRD
#define TRCPIN  PIND
#define FINTPIN PIND
#define SDI     7 // SDI SPI Data input (RFM12B side)
#define SCK     6 // SCK SPI clock
#define CS      5 // nCS SPI SS (chip select)
#define SDO     4 // SDO SPI Data output (RFM12B side)
#define NFSEL   3 // DATA/nFSEL
#define FINT    1 // CR/FINT (PORTD)

#define HI(x) TRCPORT |=  (1<<(x))
#define LO(x) TRCPORT &= ~(1<<(x))
#define WAIT_SDO_LOW() TRCPIN&(1<<SDO)
#define WAIT_SDO_HI() ~TRCPIN&(1<<SDO)
#define WAIT_FINT_HI() ~FINTPIN&(1<<FINT)

#pragma used+
void TrcPortInit(); 
void TrcInitSnd(); 
unsigned int writeCmd(unsigned int);
void TrcInitRcv();
void FIFOReset();
void TrcSendByte(unsigned char );
void TRCSpiSend(unsigned char, unsigned char );
unsigned char SpiRead();
void GetTRCPoket(unsigned char *, unsigned char);
void TrcSendPocket(unsigned char *, unsigned char);
void TrcSendPocketF(flash char *, unsigned char);
#pragma used-
