#include "xc.h"
#include "lcd_display.h"
#include "sensor.h"
#include "photon.h"
#include "ILEDFinal.h"
#include "levac_lab2b_asm_004.h"

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

//Conversion factor to convert ADC digital value to BAC
#define DIGITAL_TO_BAC_1  8.515
#define DIGITAL_TO_BAC_2  0.0001
#define DAC_OFFSET 21

//Buffer macros
#define BUFFPOW 11
#define SHIFT 24
#define SAMPLES 64 //Must be less than 1 << BUFFPOW

//Timing macros
#define RETURN_HOME 2
#define CLEAR_DISPLAY 1
#define BREATHING_LENGTH 4
#define CAR_ON_TIME 5
#define FAILED_RESULT_TIME 5 
#define THRESHOLD 115 //Threshold for being charged with DUI in the United States of America = 0.08

//#define DEBUG

volatile int mean;
volatile int digitalValues[1 << BUFFPOW]; //Max-heap to be used in data calculations
volatile int stateInit, ind = 0;
volatile int count, breakpoint = 0;

enum State_def {
    STAND_BY,
    INSTRUCTIONS,
    TEST,
    RESULT
} state;

//Function that maintains the max heap property in digital values

void maxHeapify(int i) {
    static int l, r, largest, heap_size = 1 << BUFFPOW;
    while (i <= heap_size) {
        l = (i << 1) + 1;
        r = (i << 1) + 2;
        if (l < heap_size && digitalValues[l] > digitalValues [i])
            largest = l;
        else
            largest = i;
        if (r < heap_size && digitalValues[r] > digitalValues[largest])
            largest = r;
        if (largest != i) {
            int temp = digitalValues[i];
            digitalValues[i] = digitalValues[largest];
            digitalValues[largest] = temp;
            i = largest;
        } else
            return;
    }
}

//Transforms digital values into a max heap in O(1 << BUFFPOW), max value is afterwards indexed at 1

void buildMaxHeap() {
    static int heap_size = 1 << BUFFPOW, i;
    for (i = ((heap_size - 1) >> 1); i >= 0; i--)
        maxHeapify(i);
}

//Function to calculate a digital mean of sample # of max sensor sensor values, extracts samples # of heap in O(SAMPLES*lg(2^BUFFPOW))

int averageData() {
    buildMaxHeap();
    int i, prev_mean, mean = 0, max;
    for (i = 0; i < SAMPLES; i++) {
        prev_mean = mean;
        max = digitalValues [0];
        digitalValues [0] = -1;
        maxHeapify(0);
        mean += (max - prev_mean) / (i + 1);
    }
    return mean;
}

//Function to send data to Photon / decide what to do with data

void handleResultData() {
    char BAC_estimate[20]; //string for outputting to LCD
    mean = averageData(); //computes mean over all values in buffer, about 1 second of data acquisition
    if (mean > DAC_OFFSET)
        sprintf(BAC_estimate, "%1.4f", (DIGITAL_TO_BAC_1) * (mean - DAC_OFFSET) * (DIGITAL_TO_BAC_2)); //convert mean to BAC and place in string
    else
        sprintf (BAC_estimate, "%1.1f", 0.0);
#ifdef DEBUG
    send_dac(950);
#else
    send_dac(mean); //digital value to the photon to be updated on Google Sheets
#endif
    //print result to LCD screen
    lcd_cmd(RETURN_HOME);
    lcd_cmd(CLEAR_DISPLAY);
    lcd_setCursor(0, 0);
    lcd_printStr("BAC:");
    lcd_setCursor(0, 1);
    lcd_printStr(BAC_estimate);
    count = 0;
    TMR1 = 0;
    while (count < 4); //delay so that the BAC is shown

    //deal with result / allow engine to start or not where THRESHOLD is the legal limit
    if (mean > THRESHOLD) {
        lcd_cmd(CLEAR_DISPLAY);
        lcd_setCursor(0, 0);
        lcd_printStr("Don't");
        lcd_setCursor(0, 1);
        lcd_printStr("Drive!");
        writeColor(255, 0, 0); //indicate user failed (red)

        //button press at this point brings user back to standby if count>FAILED_RESULT_TIME
    }
    else {
        LATBbits.LATB12 = 1; //indicate engine has turned on
        lcd_cmd(CLEAR_DISPLAY);
        lcd_setCursor(0, 0);
        lcd_printStr("Drive");
        lcd_setCursor(0, 1);
        lcd_printStr("Safely!");
        writeColor(0, 0, 255); //indicate pass (blue)

        //button press at this point brings user back to standby if count>CAR_ON_TIME
    }
}

//Function to handle button press for sequencing through states

void handleButtonPress() {
    switch (state) {
        case STAND_BY:
            if (count > 1) //ensure user isn't speeding through states
            {
                state = INSTRUCTIONS;
                count = 0;
                TMR3 = 0;
                stateInit = 1;
            }
            break;
        case INSTRUCTIONS:
            if (count > 1) //ensure user isn't speeding through states / reads instructions
            {
                state = TEST;
                count = 0;
                TMR3 = 0;
                stateInit = 1;
            }
            break;
        case TEST:
            //code will auto transition to result when test is complete
            break;
        case RESULT:
            if (mean > THRESHOLD) {
                if (count > FAILED_RESULT_TIME) //allows user to try again if failed
                {
                    state = STAND_BY;
                    stateInit = 1;
                    count = 0;
                }
            }
            else {
                if (count > CAR_ON_TIME) //random time value to "start car"
                {
                    state = STAND_BY;
                    stateInit = 1;
                    LATBbits.LATB12 = 0; //Turn off car
                    count = 0;
                }
            }
            break;
    }
}

//Button interrupt w/ debouncing

void __attribute__((__interrupt__, __auto_psv__)) _INT0Interrupt() {
    _INT0IE = 0;
    T4CON = 0;
    T4CONbits.TCKPS = 1;
    TMR4 = 0;
    _T4IF = 0;
    PR4 = 15999;
    T4CONbits.TON = 1;
    while (!_T4IF);
    _T4IF = 0;
    _INT0IF = 0;
    handleButtonPress();
    _INT0IE = 1;
}

//ADC interrupt to put data in circular buffer

void __attribute__((__interrupt__, __auto_psv__)) _ADC1Interrupt() {
    _AD1IF = 0;
    digitalValues[ind++] = ADC1BUF0;
    ind &= (1 << BUFFPOW) - 1;
}

//Timer interrupt to keep track of time in state

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt() {
    _T1IF = 0;
    count++;
}

//Timer interrupt to scroll

void __attribute__((__interrupt__, __auto_psv__)) _T3Interrupt() {
    _T3IF = 0;
    lcd_cmd(SHIFT); //Shift display
}

void setup() {
    CLKDIVbits.RCDIV = 0;
    iLED_setup();
    lcd_init(32);
    sensor_init();

    state = STAND_BY;
    stateInit = 1;

    //Timer to measure time elapsed in each state
    T1CON = 0;
    TMR1 = 0;
    PR1 = 62499;
    _T1IF = 0;
    _T1IE = 1;
    T1CONbits.TCKPS = 3;
    T1CONbits.TON = 1;

    //Timer to use for shifting LCD display
    T3CON = 0;
    TMR3 = 0;
    PR3 = 62499;
    _T3IF = 0;
    _T3IE = 1;
    T3CONbits.TCKPS = 2;
    T3CONbits.TON = 1;

    //setup button as an input to interrupt INT0
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;
    _INT0IF = 0;
    _INT0EP = 1; //interrupt on negative edge
    _INT0IE = 1;

    TRISBbits.TRISB12 = 0; //set engine LED as output
    LATBbits.LATB12 = 0; //Make sure engine is off
}

int main(void) {

    setup();
    while (1) {
        switch (state) {
            case STAND_BY:
                //buttonPress may occur here. state init is set to 1. so you reinit 
                //standby. next case, you go to instructions without running instructions init. 
                //this is why it is necessary to check the state upon entrance to if stateInit=1 body
                if (stateInit == 1) {
                    if (state != STAND_BY)
                        break;
                    writeColor(255,255,255);
                    _T3IE = 1; //enable scrolling
                    LATBbits.LATB12 = 0; //ensure engine is off
                    lcd_cmd(RETURN_HOME); //necessary after scrolling
                    lcd_cmd(CLEAR_DISPLAY);
                    lcd_setCursor(0, 0);
                    lcd_printStr("Press button to start engine");
                    stateInit ^= 1;
                }
                break;

            case INSTRUCTIONS:
                if (stateInit == 1) {
                    if (state != INSTRUCTIONS)
                        break;
                    lcd_cmd(RETURN_HOME);
                    lcd_setCursor(0, 0);
                    lcd_printStr("Press button, breathe until green light");
                    stateInit ^= 1;
                }
                break;

            case TEST:
                if (stateInit == 1) {
                    if (state != TEST)
                        break;
                    _T3IE = 0; //stop scrolling
                    lcd_cmd(RETURN_HOME);
                    lcd_cmd(CLEAR_DISPLAY);
                    lcd_setCursor(0, 0);
                    lcd_printStr("Taking");
                    lcd_setCursor(0, 1);
                    lcd_printStr("Data");

                    //reset time for accurate test results
                    TMR1 = 0;
                    count = 0;
                    AD1CON1bits.ADON = 1; //begin capturing data
                    turnonwheel(); //have iLED wheel during test so user knows breathalizer is working
                    stateInit ^= 1;
                }

                if (count > BREATHING_LENGTH) {
                    AD1CON1bits.ADON = 0; //stop capturing data
                    turnoffwheel();
                    writeColor(0, 255, 0); //indicate test is finished(green)
                    state = RESULT;
                    stateInit = 1;
                }
                break;

            case RESULT:
                if (stateInit == 1) {
                    if (state != RESULT)
                        break;
                    handleResultData();
                    stateInit ^= 1;
                }
                //button press depending on outcome will bring you back to STAND_BY
                break;
        }
    }
    return 0;
}
