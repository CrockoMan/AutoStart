#include <delay.h>
#include <MT12864.h>

#pragma used+
void WriteLCDByte(unsigned char);
void WaitReset(void);
unsigned char ReadLCDByte(void);
unsigned char ReadCode( void );
void WaitBusy(void);
void WaitReset(void);
void WaitON(void);
unsigned char ReadData( void );
void WriteData(unsigned char );
void WriteCode(unsigned char );
void LCD_INIT(void);
void LCD_CLS(void);
void LCD_PUT_BYTE(unsigned char, unsigned char, unsigned char);
void LCD_PUTC(unsigned char, unsigned char, unsigned char );
void LCD_PUTSF(unsigned char, unsigned char, flash unsigned char *);
void LCD_PUTS(unsigned char, unsigned char, unsigned char *);
void LCD_PUT_PIXEL(unsigned char, unsigned char);
void LCD_LINE(unsigned char, unsigned char, unsigned char, unsigned char);
void LCD_CIRCLE(unsigned char, unsigned char, unsigned char);
#pragma used-


unsigned char ReadLCDByte()
{
    unsigned char cData=0, cByte=0;
    
    cByte=LCD_DATA_PIN;

    if( TestBitLCD(cByte, DB0) )  SetBitLCD(cData, 0);
    else                          ClrBitLCD(cData, 0);
    if( TestBitLCD(cByte, DB1) )  SetBitLCD(cData, 1);
    else                          ClrBitLCD(cData, 1);
    if( TestBitLCD(cByte, DB2) )  SetBitLCD(cData, 2);
    else                          ClrBitLCD(cData, 2);
    if( TestBitLCD(cByte, DB3) )  SetBitLCD(cData, 3);
    else                          ClrBitLCD(cData, 3);
    if( TestBitLCD(cByte, DB4) )  SetBitLCD(cData, 4);
    else                          ClrBitLCD(cData, 4);
    if( TestBitLCD(cByte, DB5) )  SetBitLCD(cData, 5);
    else                          ClrBitLCD(cData, 5);
    if( TestBitLCD(cByte, DB6) )  SetBitLCD(cData, 6);
    else                          ClrBitLCD(cData, 6);
    if( TestBitLCD(cByte, DB7) )  SetBitLCD(cData, 7);
    else                          ClrBitLCD(cData, 7);
    
    return cData;    
}


void WriteLCDByte(unsigned char cByte)
{
    unsigned char cData=0;

    if( TestBitLCD(cByte, 0) )  SetBitLCD(cData, DB0);
    else                        ClrBitLCD(cData, DB0);
    if( TestBitLCD(cByte, 1) )  SetBitLCD(cData, DB1);
    else                        ClrBitLCD(cData, DB1);
    if( TestBitLCD(cByte, 2) )  SetBitLCD(cData, DB2);
    else                        ClrBitLCD(cData, DB2);
    if( TestBitLCD(cByte, 3) )  SetBitLCD(cData, DB3);
    else                        ClrBitLCD(cData, DB3);
    if( TestBitLCD(cByte, 4) )  SetBitLCD(cData, DB4);
    else                        ClrBitLCD(cData, DB4);
    if( TestBitLCD(cByte, 5) )  SetBitLCD(cData, DB5);
    else                        ClrBitLCD(cData, DB5);
    if( TestBitLCD(cByte, 6) )  SetBitLCD(cData, DB6);
    else                        ClrBitLCD(cData, DB6);
    if( TestBitLCD(cByte, 7) )  SetBitLCD(cData, DB7);
    else                        ClrBitLCD(cData, DB7);
    
    LCD_DATA_PORT = cData;
}



/*������� ���������� ������� ���. �������� � ���� ��������� ���������*/
unsigned char ReadCode( void )
{
    unsigned char ret;
    //������������� ����� �� ����
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //��������� ��� �� ��� ����� �������� �������
    ClrBitLCD(LCD_CONTROL_PORT, LCD_A0);
    SetBitLCD(LCD_CONTROL_PORT, LCD_RDWR); 
    //����� ���������, ������ ��� ���������� ���� �             
    delay_us(10); 
    SetBitLCD(LCD_CONTROL_PORT, LCD_E); 
    // ������ �� ���� ���������� �� �����             
    delay_us(10);
//    ret=LCD_DATA_PIN;         
    ret=ReadLCDByte();         
    //���������� ����� � � ���������� ���������
    ClrBitLCD(LCD_CONTROL_PORT,LCD_E);
    return ret;
}  


/*������� �������� ������ ����� ���������*/
void WaitBusy(void)
{
    unsigned char stat;
    //������ ������ ���� �� ���������� ��� BUSY
    stat=ReadCode();
    while(stat==(1<<BUSY)) stat=ReadCode();
}


//���� ����� ������ ��� RESET
void WaitReset(void)
{
    unsigned char stat;
    stat=ReadCode();
    while(stat==(1<<RESET)) stat=ReadCode();
} 


//���� ����� ������ ��� ON/OFF
void WaitON(void)
{
    unsigned char stat;
    stat=ReadCode();
    while(stat==(1<<ONOFF)) stat=ReadCode();
}


/*������� ���������� ����� �� ������ ���*/
unsigned char ReadData( void )
{
    unsigned char ret;
    //���� ���� �������� ������������
    WaitBusy();
    //������������� ����
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //����� ������ ������
    SetBitLCD(LCD_CONTROL_PORT, LCD_A0);
    SetBitLCD(LCD_CONTROL_PORT, LCD_RDWR); 
    //����� ��������� ������ ��� ���������� ���� �                     
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E); 
    //������ �� ���� ���������� �� �����                
    delay_us(10);
//    ret=LCD_DATA_PIN;         
    ret=ReadLCDByte();         
    //���������� ����� �
    ClrBitLCD(LCD_CONTROL_PORT,LCD_E);
    //���������� ���������
    return ret;
 }


/*������� ������� ������ �� ���
��� ����� ����� ��� � � �������*/
void WriteData(unsigned char data)
{ 
    WaitBusy();   
//    LCD_DATA_DDR=0xFF;
    
    SetBitLCD(LCD_CONTROL_PORT, LCD_A0);
    ClrBitLCD(LCD_CONTROL_PORT, LCD_RDWR);

    LCD_DATA_DDR=0xFF;

    
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E);
//    LCD_DATA_PORT=data;
    WriteLCDByte(data);
                               
    delay_us(10);
    ClrBitLCD(LCD_CONTROL_PORT, LCD_E);
}


/*������� ������ ������� �� �������*/
void WriteCode(unsigned char code)
{         
    WaitBusy();
//    LCD_DATA_DDR=0xFF;    
    
    ClrBitLCD(LCD_CONTROL_PORT, LCD_A0);
    ClrBitLCD(LCD_CONTROL_PORT, LCD_RDWR);

    LCD_DATA_DDR=0xFF;
     
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E);
//    LCD_DATA_PORT=code;
    WriteLCDByte(code);
        
    delay_us(10);
    ClrBitLCD(LCD_CONTROL_PORT, LCD_E);
}


/*������� ������������� �������*/                              
void LCD_INIT( void )
{ 
//    unsigned char stat;
    //��������� �������� Jtag
    
//    MCUCSR = 0x80;
//    MCUCSR = 0x80;

    //������������� ����
    LCD_DATA_DDR=0xFF;           
//    LCD_CONTROL_DDR=0xFF;
    LCD_CONTROL_DDR=(1<<LCD_A0) | (1<<LCD_RDWR) | (1<<LCD_E) | (1<<LCD_E1) | (1<<LCD_E2) | (1<<LCD_RES);
    //��� ����� ������������ ����� � 0. � ��� ����� � RES
//    LCD_CONTROL_PORT=0x00;
    LCD_CONTROL_PORT=(0<<LCD_A0) | (0<<LCD_RDWR) | (0<<LCD_E) | (0<<LCD_E1) | (0<<LCD_E2) | (0<<LCD_RES);
    //���������� ������ ������ �� �����
    delay_ms(100); 
    //���������� ������ ������.(������ RES �������������)      
    SetBitLCD(LCD_CONTROL_PORT, LCD_RES);    
    //�������� 1 �������   
    SetBitLCD(LCD_CONTROL_PORT, LCD_E1); 
    //�������� ������� � ������������� ��������� �����
    LCD_ON;
    LCD_START_LINE(0);

    ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
    //���� ����� ��� 2-�� ��������
    SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
    WaitBusy();
    LCD_ON;
    LCD_START_LINE(0);
}
                                   

//������� �������� ������
void LCD_CLS(void)
{
    unsigned char i;
    unsigned char j;  
//    unsigned char stat;

    ClrBitLCD(LCD_CONTROL_PORT, LCD_E2); 
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E1);
    LCD_OFF;
    LCD_SET_ADDRESS(0);
    for(i=0;i<8; i++)
    {
        LCD_SET_PAGE(i);
        for(j=0; j<64; j++)
        {
            WriteData(0x00);
        }
    }                  
    
    LCD_ON;
    ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
    LCD_OFF;
    LCD_SET_ADDRESS(0);
    for(i=0;i<8; i++)
    {
        LCD_SET_PAGE(i); 
        for(j=0; j<64; j++)
        {
            WriteData(0x00);
        }
    } 
    LCD_ON;
}









/*������� ����������� �� ������ �����*/
void LCD_PUT_BYTE(unsigned char x, unsigned char y, unsigned char data)
{
    //��������� ��������� ����������
    unsigned char tmp_data;
    unsigned char page;
    unsigned char bite;
//    unsigned char inv_data;
    //���� ����� �� ��������� ������ �������
    if((x>MAX_X)|(y>MAX_Y)) return;
    //���� � ������ 63 �������� ������ �������
    if(x>=64){
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
        x=x-64;
        }
        else{
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E2);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E1);
        }     
    //���������� �� ����� �������� ����� �������� ����
    page=y/8;
    bite=y%8;
    //�������� ������ ����� ���������(�� ��� ��� ���� �� ������)
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //��������� ����, 2 �����, �.�. � ������ ��� ������ �����
    tmp_data=ReadData(); 
    tmp_data=ReadData();  
    //������ �������� ������
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //� ����������� �� ������ ������ ������ ���� � �������                     
    switch(PaintMetod_KS1080){
        case MET_OR : {WriteData(tmp_data|(data<<bite)); break;}
        case MET_XOR : {WriteData(tmp_data^(data<<bite)); break;} 
        case MET_NOT_OR : {WriteData(tmp_data| ((data^0xFF)<<bite)); break;}
        case MET_NOT_XOR : {WriteData(tmp_data^((data^0xFF)<<bite)); break;}  
        case MET_CLS : {WriteData(data); break;}
        case MET_INVERSE : {WriteData(~data); break;}
        }
    //���� � ��� �� ������ ������ ������� ���������� ����� ������ �����
    if(bite>0){ 
        LCD_SET_PAGE(page+1);
        LCD_SET_ADDRESS(x);
       
        tmp_data=ReadData(); 
        tmp_data=ReadData();
        
        LCD_SET_PAGE(page+1);
        LCD_SET_ADDRESS(x);
    
        switch(PaintMetod_KS1080){
            case MET_OR : {WriteData(tmp_data|(data>>(8-bite))); break;}
            case MET_XOR : {WriteData(tmp_data^(data>>(8-bite))); break;} 
            case MET_NOT_OR : {WriteData(tmp_data|((data^0xFF)>>(8-bite))); break;}
            case MET_NOT_XOR : {WriteData(tmp_data^((data^0xFF)>>(8-bite))); break;} 
//            case MET_CLS : {WriteData(data); break;}
        }
    } 
}



/*������� ������ �������. ������ ������ ������� �� 8*5 �����. */
void LCD_PUTC(unsigned char x, unsigned char y, unsigned char ch)
{
    unsigned char textL;
    unsigned char i;   
    //� ������� ASCII ������� ���� ��������� �����, ������� ����������� 
    //����� ������� ��� ���� �������
    if(ch<0x90) textL=0x20;
    else textL=0x60;

    for(i=0; i<5; i++)
    {
        LCD_PUT_BYTE(x+i, y, sym[ch-textL][i]);
    }
     
    //��� ���������� ��������
    LCD_PUT_BYTE(x+i, y, 0x00);
    
//    ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
//    ClrBitLCD(LCD_CONTROL_PORT, LCD_E2);
//    SetBitLCD(LCD_CONTROL_PORT, LCD_E);
//    delay_ms(10);
}
   
/*������� ������ ������ �� ���� ������*/            
void LCD_PUTSF(unsigned char x, unsigned char y, flash unsigned char *str)
{
    unsigned char i=0;
    //���� �� ����� ������ ������� �������
//    while(str[i])
    while(*str)
    {
        LCD_PUTC(x+i*5+i, y, *str);
        str++;
        i++;
    }  
}

/*������� ������ ������ �� ���� ������*/            
void LCD_PUTS(unsigned char x, unsigned char y, unsigned char *str)
{
    unsigned char i=0;
    //���� �� ����� ������ ������� �������
//    while(str[i])
    while(*str)
    {
        LCD_PUTC(x+i*5+i, y, *str);
        str++;
        i++;
    }  
}




/*������� ������� ������ �� ������ 1 ������*/
void LCD_PUT_PIXEL(unsigned char x, unsigned char y)
{
    //��������� ����������
    unsigned char bite;
    unsigned char page; 
    unsigned char data;  //, data2;
    //����� ���� ����� ����� ��� ������ 
    if((x>MAX_X)|(y>MAX_Y)) return;
    //�������� ��������
    if(x>=64)
    {
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
        x=x-64;
    }
    else
    {
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E2);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E1);
    }   
    //page-����� �������� 
    page=y/8;
    //bite-����� ����� ������� ��� ��������� �������
    bite=y%8;
    //������������� �������� � ������
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //������ ���� � LCD(2 ����� �.�. � ������ �����)
    data=ReadData(); 
    data=ReadData();
    //� ����������� �� ������ ������ ������� ��� ������.
    // ������ �������� �� �������, ������� ��� �� �������������  
    switch(PaintMetod_KS1080){
        case MET_OR : { data=data|(1<<bite); break;}
        case MET_XOR : {data=data^(1<<bite); break;}  
    }
    LCD_SET_ADDRESS(x);
    WriteData(data);
}


//������� ������ ������ �� ��������� ����������.
void LCD_LINE(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
        int dx, dy, i1, i2, i, kx, ky;
        int d;     
        int x=0, y=0;
        int flag;

        dy = y2 - y1;
        dx = x2 - x1;
        if (dx == 0 && dy == 0){
                LCD_PUT_PIXEL(x,y);    
                return;
        }
        kx = 1; 
        ky = 1; 

        if( dx < 0 ){ dx = -dx; kx = -1; }
        else if(dx == 0)        kx = 0;    

        if(dy < 0) { dy = -dy; ky = -1; }

        if(dx < dy){ flag = 0; d = dx; dx = dy; dy = d; }
        else         flag = 1;

        i1 = dy + dy; d = i1 - dx; i2 = d - dx;
        x = x1; y = y1;

        for(i=0; i < dx; i++){
                LCD_PUT_PIXEL(x,y);     

                if(flag) x += kx;
                else     y += ky;

                if( d < 0 ) 
                         d += i1;
                else{       
                         d += i2;
                         if(flag) y += ky; 
                         else     x += kx;
                }
        }
        LCD_PUT_PIXEL(x,y);
}
    
/*������� ������ ���������� �� ��������� ����������. 
��� ����� ������� ���� ����������, �� �������� 
������� ������� sin � cos*/

void LCD_CIRCLE( unsigned char xc, unsigned char yc, unsigned char r )
{
  int d, x, y;
  x=0;
  y=r;
#pragma warn-  
  d=3-2*r; // �������� ������������ !
#pragma warn+  
  LCD_PUT_PIXEL(x+xc,y+yc);
  LCD_PUT_PIXEL(x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,y+yc);
  LCD_PUT_PIXEL(y+xc,x+yc);
  LCD_PUT_PIXEL(y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,x+yc);
  while(x<y){
        if(d<=0){
        d=d+4*x+6;
        }else{
        d=d+4*(x-y)+10;
        y--;
        }   
  x++;
  //LCD_PUT_PIXEL(x, y);
  LCD_PUT_PIXEL(x+xc,y+yc);
  LCD_PUT_PIXEL(x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,y+yc);
  LCD_PUT_PIXEL(y+xc,x+yc);
  LCD_PUT_PIXEL(y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,x+yc);
  }  
}         