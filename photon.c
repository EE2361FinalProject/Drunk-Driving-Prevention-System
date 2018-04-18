#include "xc.h"
#include "photon.h"

//#define DEBUG //Uncomment line to debug with main code

#ifdef DEBUG
// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1 // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF // JTAG Port Enable (JTAG port is disabled)
// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME // Clock Switching and Monitor (Clock switching is enabled,
// Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#endif

void photon_init () {
    CLKDIVbits.RCDIV = 0;
    AD1PCFGbits.PCFG4 = 1; //SDA2
    AD1PCFGbits.PCFG5 = 1; //SCL2
    I2C2CONbits.I2CEN = 0;
    I2C2BRG = 157; //100 KHz
    I2C2CONbits.I2CEN = 1;
    IFS3bits.MI2C2IF = 0;
}

void photon_cmd (char command) {
    I2C2CONbits.SEN = 1;
    while (I2C2CONbits.SEN == 1);
    I2C2TRN = 0b00000010; //Slave address is 1. 0th bit indicates a write instruction
    _MI2C2IF = 0;
    while (_MI2C2IF == 0);
    _MI2C2IF = 0;
    I2C2TRN = command; //data byte
    _MI2C2IF = 0;
    while (_MI2C2IF == 0);
    _MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while (I2C2CONbits.PEN == 1);
}

/*
 * Code to test I2C with Photon: Code characters 1 by 1
 * The character is printed every 3 seconds
 * Baud Rate = 100 kHz
 */

#ifdef DEBUG

int main(void) {
    photon_init ();
    char c = 'A';
    int i;
    T1CON = 0x0030; //PRE = 256
    TMR1 = 0;
    PR1 = 62499; //1 second delay
    _T1IF = 0;
    while (1) {
        
        T1CONbits.TON = 1;
        //Wait 20 seconds
        for (i = 0; i < 20; i++)
        {
            while (_T1IF ==0);
            _T1IF = 0;
        }
        photon_cmd (c++);
        //Threshold is 'L' in Photon_I2C.ino
        if (c == 'M')
            c = 'A';
        T1CONbits.TON = 0;
        TMR1 = 0;
        
    }
    
    return 0;
}

#endif