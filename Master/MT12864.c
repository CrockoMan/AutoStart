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



/*Функция считывания статуса ЛСД. Работает с ране выбранным кристалом*/
unsigned char ReadCode( void )
{
    unsigned char ret;
    //Конфигурируем порты на ввод
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //Указываем ЛСД то что будет читаться команда
    ClrBitLCD(LCD_CONTROL_PORT, LCD_A0);
    SetBitLCD(LCD_CONTROL_PORT, LCD_RDWR); 
    //Нужно подождать, прежде чем выставлять стоб Е             
    delay_us(10); 
    SetBitLCD(LCD_CONTROL_PORT, LCD_E); 
    // Данные на шине появляются не сразу             
    delay_us(10);
//    ret=LCD_DATA_PIN;         
    ret=ReadLCDByte();         
    //Сбрасываем строб Е и возвращаем результат
    ClrBitLCD(LCD_CONTROL_PORT,LCD_E);
    return ret;
}  


/*Функция ожидания сброса флага занятости*/
void WaitBusy(void)
{
    unsigned char stat;
    //Читаем статус пока не сброситься бит BUSY
    stat=ReadCode();
    while(stat==(1<<BUSY)) stat=ReadCode();
}


//Тоже самое только для RESET
void WaitReset(void)
{
    unsigned char stat;
    stat=ReadCode();
    while(stat==(1<<RESET)) stat=ReadCode();
} 


//Тоже самое только для ON/OFF
void WaitON(void)
{
    unsigned char stat;
    stat=ReadCode();
    while(stat==(1<<ONOFF)) stat=ReadCode();
}


/*Функция считывания байта из памяти ЛСД*/
unsigned char ReadData( void )
{
    unsigned char ret;
    //Ждем пока кристалл освободиться
    WaitBusy();
    //Конфигурируем порт
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //Будим читать данные
    SetBitLCD(LCD_CONTROL_PORT, LCD_A0);
    SetBitLCD(LCD_CONTROL_PORT, LCD_RDWR); 
    //Нужно подождать прежде чем выставлять стоб Е                     
    delay_us(10);
    SetBitLCD(LCD_CONTROL_PORT, LCD_E); 
    //Данные на шине появляются не сразу                
    delay_us(10);
//    ret=LCD_DATA_PIN;         
    ret=ReadLCDByte();         
    //Сбрасываем строб Е
    ClrBitLCD(LCD_CONTROL_PORT,LCD_E);
    //Возвращаем результат
    return ret;
 }


/*Функция выводит данные на ЛСД
Все почти также как и с чтением*/
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


/*Функция вывода команды на дисплей*/
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


/*Функция Инициализации дисплея*/                              
void LCD_INIT( void )
{ 
//    unsigned char stat;
    //Програмно вырубаем Jtag
    
//    MCUCSR = 0x80;
//    MCUCSR = 0x80;

    //Конфигурируем порт
    LCD_DATA_DDR=0xFF;           
//    LCD_CONTROL_DDR=0xFF;
    LCD_CONTROL_DDR=(1<<LCD_A0) | (1<<LCD_RDWR) | (1<<LCD_E) | (1<<LCD_E1) | (1<<LCD_E2) | (1<<LCD_RES);
    //Все линии управляещего порта в 0. В том числе и RES
//    LCD_CONTROL_PORT=0x00;
    LCD_CONTROL_PORT=(0<<LCD_A0) | (0<<LCD_RDWR) | (0<<LCD_E) | (0<<LCD_E1) | (0<<LCD_E2) | (0<<LCD_RES);
    //Удерживаем сигнал сброса на линии
    delay_ms(100); 
    //Сбрасываем сигнал Сброса.(Сигнал RES инвертирующий)      
    SetBitLCD(LCD_CONTROL_PORT, LCD_RES);    
    //Выбираем 1 кристал   
    SetBitLCD(LCD_CONTROL_PORT, LCD_E1); 
    //Включаем дисплей и устанавливаем стартовую линию
    LCD_ON;
    LCD_START_LINE(0);

    ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
    //Тоже самое для 2-го кристала
    SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
    WaitBusy();
    LCD_ON;
    LCD_START_LINE(0);
}
                                   

//Функция отчистки экрана
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









/*Функция отображения на экране байта*/
void LCD_PUT_BYTE(unsigned char x, unsigned char y, unsigned char data)
{
    //Обьявляем локальные переменные
    unsigned char tmp_data;
    unsigned char page;
    unsigned char bite;
//    unsigned char inv_data;
    //Если точка за пределами экрана выходим
    if((x>MAX_X)|(y>MAX_Y)) return;
    //Если х больше 63 выбираем второй кристал
    if(x>=64){
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E2);
        x=x-64;
        }
        else{
        ClrBitLCD(LCD_CONTROL_PORT, LCD_E2);
        SetBitLCD(LCD_CONTROL_PORT, LCD_E1);
        }     
    //Определяем на какой странице будем выводить байт
    page=y/8;
    bite=y%8;
    //Выбираем откуда будем считывать(то что уже есть на экране)
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //Считываем байт, 2 цикла, т.к. в первый раз выдаст мусор
    tmp_data=ReadData(); 
    tmp_data=ReadData();  
    //Заново выбираем адресс
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //В зависимосте от метода вывода меняем байт и выводим                     
    switch(PaintMetod_KS1080){
        case MET_OR : {WriteData(tmp_data|(data<<bite)); break;}
        case MET_XOR : {WriteData(tmp_data^(data<<bite)); break;} 
        case MET_NOT_OR : {WriteData(tmp_data| ((data^0xFF)<<bite)); break;}
        case MET_NOT_XOR : {WriteData(tmp_data^((data^0xFF)<<bite)); break;}  
        case MET_CLS : {WriteData(data); break;}
        case MET_INVERSE : {WriteData(~data); break;}
        }
    //Если у был не кратен восьми выводим оставшуюся часть нашего байта
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



/*Функция вывода символа. Каждый символ состоит из 8*5 точек. */
void LCD_PUTC(unsigned char x, unsigned char y, unsigned char ch)
{
    unsigned char textL;
    unsigned char i;   
    //В таблице ASCII символы идут несколько иначе, поэтому преобразуем 
    //номер символа под нашу таблицу
    if(ch<0x90) textL=0x20;
    else textL=0x60;

    for(i=0; i<5; i++)
    {
        LCD_PUT_BYTE(x+i, y, sym[ch-textL][i]);
    }
     
    //Для разделения символов
    LCD_PUT_BYTE(x+i, y, 0x00);
    
//    ClrBitLCD(LCD_CONTROL_PORT, LCD_E1);
//    ClrBitLCD(LCD_CONTROL_PORT, LCD_E2);
//    SetBitLCD(LCD_CONTROL_PORT, LCD_E);
//    delay_ms(10);
}
   
/*Функция вывода строки из флеш памяти*/            
void LCD_PUTSF(unsigned char x, unsigned char y, flash unsigned char *str)
{
    unsigned char i=0;
    //Пока не конец строки выводим символы
//    while(str[i])
    while(*str)
    {
        LCD_PUTC(x+i*5+i, y, *str);
        str++;
        i++;
    }  
}

/*Функция вывода строки из флеш памяти*/            
void LCD_PUTS(unsigned char x, unsigned char y, unsigned char *str)
{
    unsigned char i=0;
    //Пока не конец строки выводим символы
//    while(str[i])
    while(*str)
    {
        LCD_PUTC(x+i*5+i, y, *str);
        str++;
        i++;
    }  
}




/*Функция которая рисует на экране 1 пиксел*/
void LCD_PUT_PIXEL(unsigned char x, unsigned char y)
{
    //Обьявляем переменные
    unsigned char bite;
    unsigned char page; 
    unsigned char data;  //, data2;
    //Выход если точка лежит вне экрана 
    if((x>MAX_X)|(y>MAX_Y)) return;
    //Выбираем кристалл
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
    //page-номер страницы 
    page=y/8;
    //bite-Номер байта который нам предстоит вывести
    bite=y%8;
    //Устанавливаем страницу и адресс
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //Читаем байт с LCD(2 цикла т.к. в первом мусор)
    data=ReadData(); 
    data=ReadData();
    //В зависимости от метода вывода выводим наш пиксел.
    // Адресс страницы не менялся, поэтому его не устанавливаем  
    switch(PaintMetod_KS1080){
        case MET_OR : { data=data|(1<<bite); break;}
        case MET_XOR : {data=data^(1<<bite); break;}  
    }
    LCD_SET_ADDRESS(x);
    WriteData(data);
}


//Функция вывода прямой по алгоритму Брезенхома.
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
    
/*Функция вывода окружности по алгоритму Брезенхома. 
При малом радиусе есть неточности, но работает 
гораздо быстрее sin и cos*/

void LCD_CIRCLE( unsigned char xc, unsigned char yc, unsigned char r )
{
  int d, x, y;
  x=0;
  y=r;
#pragma warn-  
  d=3-2*r; // возможно переполнение !
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