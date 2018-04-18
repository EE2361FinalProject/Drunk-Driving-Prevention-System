#include "stdint.h"
#include "xc.h"
#include "levac_lab2b_asm_004.h"
#include "ILEDFinal.h"

volatile int i= 0;

void turnonwheel(void){
 TRISBbits.TRISB13 = 0;
 LATBbits.LATB13 = 0;
_T5IE=1;
_T5IF=0;
TMR5=0;
PR5= 1000;
T5CON = 0x8030;
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
    int shiftr;
    int shiftg;
    int shiftb;
    int i = 7;
    int p = 7;
    int k = 7;
    
    while (i >= 0) {
        shiftr = 1 << i;
        if ((r & shiftr) > 0) {
            write_1();
        } else {
            write_0();
        }
        i = i - 1;
    }

    while (p >= 0) {
        shiftg = 1 << p;
        if ((g & shiftg) > 0) {
            write_1();
        } else {
            write_0();
        }
        p = p - 1;
    }

    while (k >= 0) {
        shiftb = 1 << k;
        if ((b & shiftb) > 0) {
            write_1();
        } else {
            write_0();
        }
        k = k - 1;
    }

}


void __attribute__((__interrupt__,__auto_psv__)) _T5Interrupt(void){
    _T5IF = 0;
    
    writePacCol(Wheel(i));
        i=(i+1)%255;
        
}


