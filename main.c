#include "xc.h"
#include "lcd_display.h"
#include "sensor.h"
#include "photon.h"
#include "iLED.h"

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
#define RETURN_HOME 2
#define BREATHING_LENGTH 10


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

    LATBbits.LATB12 = 0; //Make sure engine is off
}

//allows to change state. May modifty with conditions for each state this way if needed
//this is bad. User could press button rapidly to hit result state then drive since mean will be zero
//ill add a version below this function that might work better since this is only used in INT0 interrupt
void change_state() 
{
    switch(state){
        case STAND_BY:
            state=INSTRUCTIONS;
            TMR3 = 0;
            break;
        case INSTRUCTIONS:
            state=TEST;
            TMR3 = 0;
            break;
        case TEST:
            state=RESULT;
	    _T3IE = 0; //Stop shifting
            break;
        case RESULT:
            state=STAND_BY;
            break;
    }
    stateInit=1;           
}

void handleButtonPress()
{
	switch(state)
	{
		case STAND_BY:
			if(count > 1) //ensure user isn't speeding through states
			{
				state = INSTRUCTIONS;
				count = 0;
			}
			break;
		case INSTRUCTIONS:
			if(count > 1)	//ensure user isn't speeding thorugh states / reads instructions
			{
				state = TEST;
				count = 0;
			}
			break;
		case TEST:
			//code will auto transition to result when test is complete
			break;
		case RESULT:
			if(count > 5) //random time value to "start car"
			{
				state = STAND_BY;
				count = 0;
			}
			break;
	}
}

void __attribute__((__interrupt__,__auto_psv__)) _INT0Interrupt()
{
    _INT0IF=0;
	
    //Debounce button
    T4CON=0;
    TMR4=0;
    _T4IF=0;
    PR4=15999;
    T4CONbits.TON=1;
    while(!_T4IF);
    _T4IF = 0;
	
    //change_state();
    handleButtonPress();
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

//Timer interrupt to scroll
void __attribute__((__interrupt__,__auto_psv__)) _T3Interrupt()
{
    _T3IF=0;
    lcd_cmd(SHIFT); //Shift display
}

int main(void) {
    int mean;
    int prev_mean;
    char BAC_estimate[20]; //string for outputting to LCD
    while(1)
    {
        switch(state){
            case STAND_BY: 
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
                    //count=0;
                    AD1CON1bits.TON=1;
	            iLED_wheel_on();
                }
                
                if(count > BREATHING_LENGTH)
                {
                    AD1CON1bits.TON=0;
		    iLED_wheel_off();
                    iLED_color(0, 255, 0); //indicate test is finished(green)
                    state=RESULT;
                    stateInit=1;
                }
                
                break;
            case RESULT:
                if(stateInit==1)
                {
			
		    //can we make this a function? really clogs up main when someone is trying to see what we are 
			//doing. Could easily say something like handleData(); to clean up. Thoughts?
                    static int i;
                    mean=0;
                    for(int i=0, i<(1<<BUFFPOW), i++)
                    {
                        prev_mean=mean;
                        mean+=digitalValues[i]/(i+1);
                    }
                    sprintf(BAC_estimate, "%5.1f", (DIGITAL_TO_BAC)*mean);
                    lcd_cmd(RETURN_HOME);
                    lcd_setCursor(0,0);
                    lcd_printStr("BAC:")
                    lcd_setCursor(0,1);
                    count=0
		    TMR1=1;
		    lcd_printStr(BAC_estimate);
		    while(count<2); //delay so that the BAC is shown
                    if(mean>THRESHOLD) 
                    {
			send_dac(mean);
			lcd_setCursor(0,0);
			lcd_printStr("Don't");
			lcd_setCursor(0,1);
			lcd_setCursor("Drive");
			iLED_color(255, 0, 0); //indicate user failed (red)
                    }
                    else
                    {
                        LATBbits.LATB12=1; //indictate engine has turned on
			send_dac(mean);		//should send data either way. Could be good to know if repeat DUI offenders drive buzzed
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
