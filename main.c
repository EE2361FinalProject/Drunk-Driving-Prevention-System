#include "xc.h"
#include "lcd_display"

//configuration
#pragma config ICS=PGx1
#pragma config FWDTEN=OFF
#pragma config GWRP=OFF
#pragma config GCP=OFF
#pragma config JTAGEN=OFF

#pragma config I2C1SEL=PRI
#pragma config IOL1WAY=OFF
#pragma config OSCIOFNC=ON
#pragma config FCKSM=CSECME
#pragma config FNOSC=FRCPLL

#define BUFFPOW 13
#define RETURN_HOME 2
#define SHIFT 24

volatile int digitalValues[1 << BUFFPOW];
volatile int stateInit;
volatile int count;

enum State_def{
    STAND_BY,
    INSTRUCTIONS,
    TEST,
    RESULT
} state;

void setup()
{
    lcd_init();
    adc_init();
    state=STAND_BY;
    
    //Timer to measure time elapsed in each state
    T1CON=0;
    TMR1=0;
    PR1=62499;
    _T1IF=0;
    _T1IE=1;
    T1CONbits.TCKPS=3;
    T1CONbits.TON=1;
    
    //Timer to use for shifting LCD display
    T3CON=0;
    TMR3=0;
    PR3=62499;
    _T3IF=0;
    _T3IE=1;
    T3CONbits.TCKPS=2;
    T3CONbits.TON=1;
    
    AD1CON1=0;
    AD1CON2=0;
    AD1CON3=0;
    AD1CHS=0;
    AD1PCFG=0;
    AD1CSSL=0;
     
}

void __attribute__((__interrupt__,__auto_psv__)) _ADC1Interrupt()
{
    _AD1IF=0;
    digitalValues[ind++]=AD1BUF0;
    ind&=((1 << BUFFPOW) -1);
}

void __attribute__((__interrupt__,__auto_psv__)) _TInterrupt()
{
    _T1IF=0;
    count++;
}

void __attribute__((__interrupt__,__auto_psv__)) _T3Interrupt()
{
    _T3IF=0;
    lcd_cmd(SHIFT); //Shift display
}

//set lcdRefresh to 1 at any point a state is changed.

int main(void) {
    
    while(1)
    {
        switch(state){
            case STAND_BY: 
                //print "Press Button to Start Engine"
                //timer interrupt to scroll
                if(stateInit==1)
                {
                    lcd_cmd(RETURN_HOME); 
                    lcd_setCursor(0,0);
                    lcd_printStr("Press button to start engine");
                    stateInit ^=1;
                }
                break;
                
            case INSTRUCTIONS:
                if(stateInit==1)
                {
                    lcd_cmd(RETURN_HOME);
                    lcd_setCursor(0,0);
                    lcd_printStr("Press button to start breathing, breathe until green light");
                    stateInit ^=1;
                }
                break;
                
            case TEST:
                if(stateInit==1)
                {
                    TMR1=1;
                    count=0;
                    AD1CON1bits.TON=1;
                
                }
                
                if(count>10)
                {
                    AD1CON1bits.TON=0;
                    //set led green
                    state=RESULT;
                    stateInit=1;
                }
                
                break;
            case RESULT:
                if(stateInit==1)
                {
                    //perform calculations
                    //deal with result
                    // turn car on
                    //display BAC, send messages, and all
                }
                break;
            
                
        }
    }
    return 0;
}
