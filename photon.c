#include "xc.h"
#include "photon.h"

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

void send_dac (int mean) {
    photon_cmd (mean >> 2);
    photon_cmd (mean & 3);
}


