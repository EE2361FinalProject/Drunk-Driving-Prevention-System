#include "xc.h"
#include "sensor.h"

//Calibrate Sensor using Arduino

/*SAMC = 31
 *ADCS = 64
 *SSRC = 0b111 (auto-convert)
 *ASAM = 1
 *AN0 is used
 */

volatile int digitalValues[1 << BUFFPOW]; //Max-heap to be used in data calculations
volatile int ind = 0;

void sensor_init()
{
    AD1CON1=0;
    AD1CON2=0;
    AD1CON3=0;
    AD1CHS=0;
    AD1CON1bits.SSRC=0b111;
    AD1CON1bits.ASAM = 1;
    AD1CON3bits.SAMC = 31;
    AD1CON3bits.ADCS = 63;
    AD1PCFGbits.PCFG0 = 0;
    _AD1IE=1;
    _TRISA0 = 1;
}

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

//ADC interrupt to put data in circular buffer

void __attribute__((__interrupt__, __auto_psv__)) _ADC1Interrupt() {
    _AD1IF = 0;
    digitalValues[ind++] = ADC1BUF0;
    ind &= (1 << BUFFPOW) - 1;
}