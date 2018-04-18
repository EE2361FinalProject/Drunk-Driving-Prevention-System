#include "xc.h"
#include "sensor.h"

//Calibrate Sensor using Arduino

/*SAMC = 31
 *ADCS = 64
 *SSRC = 0b111 (auto-convert)
 *ASAM = 1
 *AN0 is used
 */

void sensor_init()
{
    AD1CON1bits.SSRC=0b111;
    AD1CON1bits.ASAM = 1;
    AD1CON3bits.SAMC = 31;
    AD1CON3bits.ADCS = 63;
    AD1CON2=0;
    AD1PCFGbits.PCFG0 = 0;
    _TRISA0 = 1;
}

