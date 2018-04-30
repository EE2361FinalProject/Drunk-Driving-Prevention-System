# Drunk-Driving-Prevention-System
This project aims to use an alcohol gas sensor to measure the BAC of a driver when entering their car, warn the user about the danger that lies ahead, send a SMS message to an emergency contact alerting them of the situation, and potentially prevent the car from igniting. To implement this project, 0-3.3V ADC conversion libraries would have to be implemented so that the PIC24 can interface with the analog sensor. An LCD display could be used to display a warning message for the driver. To send a SMS message, the PIC24 will communicate with a Particle Photon that will activate two IFTTT recipes. The value received by the sensor will be transmitted to the Photon using I2C, then pushed to a Google Sheet that will keep a log of the value recorded at every usage. Finally, the prevention of ignition could be simulated by connecting a button (functioning as the car key) to an LED. If the BAC is above a certain level then pressing the button will not turn the LED on and the second IFTTT recipe will send an SMS to the emergency contact. Otherwise, the LED will turn on. 

Macroscopic view:
0)  Initialize IFTTT recipes that publish BAC data to Google Sheets and sends SMS message to emergency contact number.
1)	Initialize LCD to display welcome text.
2)	Wait for user to press the ignition button.
a)	If the button is pressed, then await button press to breathe into breathalyzer.
3)	Sample data from alcohol sensor and convert to BAC values. iLED will wheel through colors.
4)  Send data to Particle Photon which will then be pushed to Google Sheets.
5)	Check BAC < Thresh
a)	If true, then turn on LED 
b)	If false, then keep LED off and send SMS message to emergency contact set in an IFTTT recipe. 

Materials:
•	PIC24FJ64GA002
•	Particle Photon 
•	iLED
•	Switch Science 8x2 LCD Display
•	LED & Push-button
•	Adafruit MiCS5524 CO, Alcohol and VOC Gas Sensor

Useful Links:
•	Adafruit gas sensor: https://www.adafruit.com/product/3199 
•	Photon I2C forum: https://community.particle.io/t/i2c-photon-electron/27371/3
•	Photon I2C documentation: https://docs.particle.io/reference/firmware/photon/#wire-i2c-
