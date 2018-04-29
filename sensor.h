#ifndef SENSOR_H
#define	SENSOR_H

#define ALCOHOL 0
#define MOUTHWASH 1

#define CONFIGURATION MOUTHWASH

//Conversion factor to convert ADC digital value to BAC
#if CONFIGURATION == MOUTHWASH
#define DIGITAL_TO_BAC_1  0.0001
#define DIGITAL_TO_BAC_2  13
#define DAC_OFFSET 0
#define THRESHOLD 62
#elif CONFIGURATION == ALCOHOL
#define DIGITAL_TO_BAC_1  0.0001
#define DIGITAL_TO_BAC_2  8.515
#define DAC_OFFSET 21
#define THRESHOLD 115
#endif

//Buffer macros
#define BUFFPOW 11
#define SAMPLES 64 //Must be less than 1 << BUFFPOW

void sensor_init();
int averageData ();

#endif	/* SENSOR_H */