include "xc.h"
#include "lcd_display.h"

void lcd_cmd(char command) //sends a series of data to lcd including a start bit
//address byte, control byte, and command byte. Finished with stop byte
{
    I2C2CONbits.SEN=1;
    while(I2C2CONbits.SEN==1);
    I2C2TRN = 0b01111100; 
    _MI2C2IF=0;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2TRN = 0b00000000;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2TRN = command;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN ==1);
}

void lcd_setCursor(char x, char y) 
{
    char Loc=y<<6; //y bit is sixth in from left
    Loc|=x; //x location in command is first 3 bits
    Loc+=0x80; //necessary part for setting location with lcd_cmd
    lcd_cmd(Loc);
}

void lcd_printChar(char myChar) //much like lcd_cmd, but control byte changed
{
    I2C2CONbits.SEN=1;
    while(I2C2CONbits.SEN==1);
    I2C2TRN = 0b01111100;
    _MI2C2IF=0;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2TRN = 0b01000000; //control byte starts with 01
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2TRN = myChar;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN ==1);
}

void lcd_printStr(const char *s)
{
    char *a=(char*) s;
    if(*a==0)
    {
        return; //empty string, do nothing
    }
    I2C2CONbits.SEN=1;
    while(I2C2CONbits.SEN==1); 
    
    I2C2TRN = 0b01111100; //slave address
    _MI2C2IF=0;
    while(!_MI2C2IF);
    _MI2C2IF=0;
    I2C2TRN = 0b01000000; //control byte
    while(!_MI2C2IF);
    _MI2C2IF=0;
    while((*a)!=0)
    {
        
        I2C2TRN = *a; //write data repetitively
        while(!_MI2C2IF);
        _MI2C2IF=0;
        a+=1;
    }
   
    I2C2CONbits.PEN = 1; //stop bit
    while(I2C2CONbits.PEN ==1);
}

void lcd_init(char contrast)
{
    //contrast variables are set below with proper form for later use
    static char lowCon;
    static char highCon;
    lowCon=contrast&(0xF);
    highCon=contrast&(0x30);
    highCon=highCon>>4; 
    static int delay=0;
    //output pins initialized below
    AD1PCFGbits.PCFG4=1;
    AD1PCFGbits.PCFG5=1;
    TRISB=0x0003;
    TRISA=0;
    //timer two initialized below for mandatory i2c delays
    T2CON=0;
    TMR2=0;
    _T2IF=0;
    PR2=12499;
    T2CON=0x8020;
    I2C2CON=0;
    I2C2BRG=157; //to 100 kHz
    _MI2C2IF=0;
    I2C2CONbits.I2CEN=1;
    TMR2=0;
    _T2IF=0;
    while(!_T2IF); //mandatory delay
    _T2IF=0;
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00111001); // function set, extended instruction mode
    lcd_cmd(0b00010100); // interval osc
    lcd_cmd(0b01110000|lowCon); // contrast C3-C0
    lcd_cmd(0b01010100|highCon); // C5-C4, Ion, Bon //inconsistency in background
    lcd_cmd(0b01101100); // follower control
    delay=0;
    TMR2=0;
    _T2IF=0;
    while(delay<4) //mandatory delay
    {
        while(!_T2IF);
        _T2IF=0;
        delay++;
    }
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00001100); // Display On
    lcd_cmd(0b00000001); // Clear Display
    
    TMR2=0;
    _T2IF=0;
    PR2=499;
    while(!_T2IF); //mandatory delay
    _T2IF=0;
}
