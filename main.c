#include "xc.h"
#include "lcd_display.h"
#include "sensor.h"
#include "photon.h"

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

#define DIGITAL_TO_BAC 1/1000 //currently arbitary conversion factor
#define BUFFPOW 13
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
    sensor_init();
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
    
    //setup button as an input to interrupt INT0
    TRISBbits.TRISB7=1;
    CNPU2bits.CN23PUE=1;
    _INT0IF=0;
    _INT0EP=1; //interrupt on negative edge
    _INT0IE=1;
}

//allows to change state. May modifty with conditions for each state this way if needed
void change_state() 
{
    switch(state){
        case STAND_BY:
            state=INSTRUCTIONS;                   
            break;
        case INSTRUCTIONS:
            state=TEST;
            break;
        case TEST:
            state=RESULT;
            break;
        case RESULT:
            state=STAND_BY;
            break;
    }
    stateInit=1;           
}

void __attribute__((__interrupt__,__auto_psv__)) _INT0Interrupt()
{
    _INT0IF=0;
    //debounce
    change_state();
}

//should go in sensor file
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
    //computations done in main in Results state?
    int mean;
    int prev_mean;
    char BAC_estimate[20]; //string for outputting to LCD
    while(1)
    {
        switch(state){
            case STAND_BY: 
                //timer interrupt to scroll
                if(stateInit==1)
                {
                    lcd_setCursor(0,0);
                    lcd_printStr("Press button to start engine");
                    stateInit ^=1;
                }
                break;
                
            case INSTRUCTIONS:
                if(stateInit==1)
                {
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
                    static int i;
                    mean=0;
                    for(int i=0, i<(1<<BUFFPOW), i++)
                    {
                        prev_mean=mean;
                        mean+=digitalValues[i]/(i+1);
                    }
                    sprintf(BAC_estimate, "%5.1f", (DIGITAL_TO_BAC)*mean);
                    lcd_setCursor(0,0);
                    lcd_printStr("BAC:")
                    lcd_setCursor(0,1);
                    //delay so that the BAC is actually shown
                    lcd_printStr(BAC_estimate);
                    if(mean>0) //whatever the condition is for not allowing driving(in digital form)
                    {
                        //send message
                        //display to the lcd that they should not drive
                    }
                    else
                    {
                        //just an example. whatever pin we have the engine indicator on
                        LATBbits.LATB0=1;
                        lcd_setCursor(0,0);
                        lcd_printStr("Drive");
                        lcd_setCursor(0,1);
                        lcd_printStr("Safely");
                    }
                }
                break;            
        }
    }
    return 0;
}
