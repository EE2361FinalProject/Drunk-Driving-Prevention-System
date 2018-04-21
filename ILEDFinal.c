#include "stdint.h"
#include "xc.h"
#include "levac_lab2b_asm_004.h"
#include "ILEDFinal.h"

volatile int i = 0;

void iLED_setup(void) {
    TRISBbits.TRISB13 = 0;
    LATBbits.LATB13 = 0;
}

void turnonwheel(void) {
    _T5IF = 0;
    _T5IE = 1;
    TMR5 = 0;
    PR5 = 1000;
    T5CON = 0x8030;
}

void turnoffwheel(void) {
    T5CONbits.TON = 0;
}

uint32_t packColor(unsigned char Red, unsigned char Blu, unsigned char Grn) {
    unsigned long int RGBval = 0;
    RGBval = ((long) Red << 16) | ((long) Grn << 8) | ((long) Blu);
    return RGBval;
}

unsigned char getR(uint32_t RGBval) {
    unsigned char Red = 0;
    Red = (unsigned char) (RGBval >> 16);

    return Red;
}

unsigned char getG(uint32_t RGBval) {
    unsigned char Green = 0;
    Green = (unsigned char) (RGBval >> 8);

    return Green;
}

unsigned char getB(uint32_t RGBval) {
    unsigned char Blue = 0;
    Blue = (unsigned char) (RGBval >> 0);

    return Blue;
}

void writePacCol(uint32_t PackedColor) {
    writeColor(getR(PackedColor), getG(PackedColor), getB(PackedColor));
}

uint32_t Wheel(unsigned char WheelPos) {
    WheelPos = 255 - WheelPos;

    if (WheelPos < 85) {
        return packColor(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        return packColor(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return packColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void writeColor(int r, int g, int b) {
    int i = 7;
    int j = 7;
    int k = 7;
    int redShift;
    int blueShift;
    int greenShift;
    LATBbits.LATB13 = 0;
    
    delay_100us();
        while (i >= 0) {
        redShift = 1 << i;
        if ((redShift & r) > 0)
            write_1();
        else
            write_0();
        i = i - 1;
    }
    while (j >= 0) {
        greenShift = 1 << j;
        if ((greenShift & g) > 0)
            write_1();
        else
            write_0();
        j = j - 1;
    }
    while (k >= 0) {
        blueShift = 1 << k;
        if ((blueShift & b) > 0)
            write_1();
        else
            write_0();
        k = k - 1;
    }

}

void __attribute__((__interrupt__, __auto_psv__)) _T5Interrupt(void) {
    _T5IF = 0;

    writePacCol(Wheel(i));
    i = (i + 1) % 255;

}


