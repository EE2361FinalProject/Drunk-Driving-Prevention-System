#include "application.h"

//#define DEBUG //Uncomment to DEBUG with Serial

#ifdef DEBUG
#define THRESHOLD 980
#else
#define THRESHOLD 0.08 
#define DIGITAL_TO_BAC (double) 0.0008515
#define DAC_OFFSET 21
#endif

bool publish = false;

#ifdef DEBUG 
int c;
String sC = "";
bool firstTime = true;
#else
int bac = 0;
String sBAC = "";
bool firstTime = true;
#endif

/* Purpose: Handler for I2C receiving
 * Updates bac variable to match most recent data from PIC24 running A/D
 * Turns publish flag true to update Google Spreadsheet with most recent BAC data
 * Avoid publishing events in receive handler as publishing can take at least 1 second to complete
 */
 
void receiveEvent (int howMany) {
    while (Wire.available()) {
        publish = true;
        #ifdef DEBUG
	    if (firstTime) {
            c = Wire.read () * 4;
            firstTime = false;
	    }
        else {
            c += Wire.read ();
            Serial.print (c);
        }
        #else
        if (firstTime) {
           bac = Wire.read() * 4;
           firstTime = false;
        }
        else {
           bac += Wire.read ();
           bac = (bac- DAC_OFFSET) * DIGITAL_TO_BAC;
        }
        #endif
    }
}

/* Purpose: Handler for BAC_result event. Checks to see if bac > THRESHOLD
 * BAC_HIGH event triggers SMS IFTTT recipe
 */
 
void bacHandler (const char *event, const char *data) {
    
    //Arbitrary char picked for debugging threshold
    #ifdef DEBUG
    if (c > THRESHOLD) {
        Particle.publish ("BAC_High");
    }
    #else    
    if (bac > THRESHOLD) {
        Particle.publish ("BAC_High");
    }
    #endif
    
}


void setup() {
	
    //Use Serial for debugging
    #ifdef DEBUG
    Serial.begin(9600);
    while (!Serial);
    #endif
    
    Particle.connect();
    if (!Wire.isEnabled ()) {
        Wire.begin(0x1);
        Serial.println ("Wire is enabled");
        Wire.onReceive (receiveEvent);
    }
    
    Particle.subscribe ("BAC_result", bacHandler);
}

void loop() {
    if (publish)
    {   
        #ifdef DEBUG
        sC = String (c);
        Particle.publish ("BAC_result", sC);
        #else
        sBAC = String (bac);
        Particle.publish ("BAC_result", sBAC);
        #endif
        publish = false;
    }
}
