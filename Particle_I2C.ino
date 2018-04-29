#include "application.h"

#define THRESHOLD 115
#define DIGITAL_TO_BAC_1 8.515
#define DIGITAL_TO_BAC_2 10000.0
#define DAC_OFFSET 21

bool publish = false;

int bac = 0;
float realBAC = 0;
String sBAC = "";
bool firstTime = true;

/* Purpose: Handler for I2C receiving
 * Updates bac variable to match most recent data from PIC24 running A/D
 * Turns publish flag true to update Google Spreadsheet with most recent BAC data
 * Avoid publishing events in receive handler as publishing can take at least 1 second to complete
 */
 
void receiveEvent (int howMany) {
    while (Wire.available()) {
        publish = true;
        if (firstTime) {
           bac = 0;
           bac = Wire.read() * 4;
           firstTime = false;
        }
        else {
           bac += Wire.read ();
           realBAC = (bac- 21) * DIGITAL_TO_BAC_1 / DIGITAL_TO_BAC_2;
        }
    }
}

/* Purpose: Handler for BAC_result event. Checks to see if bac > THRESHOLD
 * BAC_HIGH event triggers SMS IFTTT recipe
 */
 
void bacHandler (const char *event, const char *data) {
    if (bac > THRESHOLD) {
        Particle.publish ("BAC_High");
    }
}


void setup() {
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
        firstTime = true;
        sBAC = String (realBAC);
        Particle.publish ("BAC_result", sBAC);
        publish = false;
    }
}
