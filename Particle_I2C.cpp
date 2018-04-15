#include "application.h"

void receiveEvent (int howMany) {
    while (Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
    }
}

void setup() {
	Serial.begin(9600);
    while (!Serial);
    
    if (!Wire.isEnabled ()) {
        Wire.begin(0x1);
        Serial.println ("Wire is enabled");
        Wire.onReceive (receiveEvent);
    }
}

void loop() {
    delay(100);
}