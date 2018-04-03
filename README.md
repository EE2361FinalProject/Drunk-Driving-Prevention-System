# Drunk-Driving-Prevention-System
This project aims to use an alcohol gas sensor to measure the BAC of a driver when entering their car, warn the user about the danger that lies ahead, send a SMS message to an emergency contact alerting them of the situation, and potentially prevent the car from igniting. To implement this project, 0-3.3V ADC conversion libraries would have to be implemented so that the PIC24 can interface with the analog sensor. An LCD display could be used to display a warning message for the driver. To send a SMS message, the PIC24 could communicate with an Arduino with cellular GSM modules or the PIC24 could communicate with a Particle Photon that will activate a IFTTT recipe. The user will be asked to input number of their emergency contact using a keypad. Finally, the prevention of ignition could be simulated by connecting a button (functioning as the car key) to an LED and waiting for the user to breathe into the sensor: if the BAC is above a certain level then pressing the button will not turn the LED on, otherwise it will. This repository contains code that is used to implement a drunk driving prevention system. 

Macroscopic view:
1)	Initialize LCD to display welcome text: “Welcome, please input emergency contact number…”
2)	Wait for user to input number.
3)	Check to see if viable phone number. 
a)	If viable, then 4).
b)	Else repeat 2) and display “Please enter viable phone number”.
4)	Wait for user to press the ignition button
a)	If the button is pressed, then commence countdown sequence (T0 = 5 seconds) for data collection and display “Breathe in sensor in T0—seconds for X seconds”
b)	Else, continue to display “Press button to start car”
5)	Sample data from alcohol sensor and convert to BAC values. Display “Continue breathing for X—seconds” where X = 3 seconds
6)	Check BAC < Thresh
a)	If true, then turn on LED
b)	If false, then 7)
7)	Send phone number to Arduino using I2C and send SMS message to emergency contact number. Display “Don’t drive. A message has been sent”

Materials:
•	PIC24FJ64GA002
•	Arduino Genuino UNO
•	SIM900 Quad Band GSM GPRS Shield Development Board for Arduino
•	Switch Science 8x2 LCD Display
•	Keypad 4x4 Matrix
•	LED & Push-button
•	Adafruit MiCS5524 CO, Alcohol and VOC Gas Sensor

Useful Links:
•	Adafruit gas sensor: https://www.adafruit.com/product/3199 
•	Sparkfun gas sensor: https://www.sparkfun.com/products/8880
•	Arduino I2C: https://www.arduino.cc/en/Tutorial/MasterWriter
•	Arduino GSM Shield: https://www.arduino.cc/en/Guide/ArduinoGSMShield
•	GSM Shield Purchase: https://www.banggood.com/SIM900-Quad-band-GSM-GPRS-Shield-Development-Board-For-Arduino-p-964229.html?gmcCountry=US&currency=USD&createTmp=1&utm_source=googleshopping&utm_medium=cpc_elc&utm_content=zouzou&utm_campaign=pla-us-ele-prof-1-pc&gclid=EAIaIQobChMIl5fmo4-S2gIVhbjACh01PAkbEAQYBSABEgKuB_D_BwE&cur_warehouse=CN
