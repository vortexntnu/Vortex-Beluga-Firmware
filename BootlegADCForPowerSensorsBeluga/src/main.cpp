/*
General info about the code:
  - This code is made for Arduino on Beluga
  - Basicaly a arduino whose only purpose is to work as a ADC
  - Code takes in analog signals from different sensors and samples it to convert it to a digital signal
  - Takes in analog signal from PSM (Power Sense Module)
  - Takes in analog signal with information about the current from ESCs (Electronic speed controllers)
  - For technical documentation of the Power Sense Module, check out https://bluerobotics.com/store/comm-control-power/control/psm-asm-r2-rp/
  - ESC datasheet: https://www.hobbywing.com/products/enpdf/XRotorMico60ADShot1200en.pdf
I2C buss info:
  - The digital signal gets sent to I2C buss on request from RPI
  - I2C Buss uses pins number:
    SDA = A4
    SCL = A5
    DO NOT USE THESE PINS FOR OTHER PURPOSES!
  - These are the request mesages Arduino can get from RPI
    0 (Request for PSM Voltage) 
    1 (Request for PSM Current) 
*/
#include <Arduino.h>
#include <Wire.h>

// Variables for I2C
int dataSendAdress = 12; // Data to I2C buss is sent to adress nr 7
int mesage = -1; // Message value that we will be receiving from RPI. Set to non existent value first just in case something goes wrong

// Variables for PSM (Power Sence Module)
int voltageInPSM = A0;
int currentInPSM = A1;
int currentInESC1 = A2; // Takes in voltage from ESC with info about current, where 1A current equals 11.75mV
int currentInESC2 = A3; // CHECK IF A2 AND A3 PINS ARE AVAILIBLE

// Define functions for I2C signal handling 
void receiveEvent(int howMany);
void requestEvent();
void send2bytes(long int msg2byte);

void setup() {
  // Set up for I2C buss
  Wire.begin(dataSendAdress); // Configure Arduino to be able to publish data to I2C buss
  Wire.onReceive(receiveEvent); // Register trigger when data received
  Wire.onRequest(requestEvent); // Register trigger for sending data back

  // Set up PSM parameters
  pinMode(voltageInPSM, INPUT);
  pinMode(currentInPSM, INPUT);
  pinMode(currentInESC1, INPUT); 
  pinMode(currentInESC2, INPUT);

  // For debuging
  Serial.begin(9600);
}

void loop() {
  // Loop Empty
}

void receiveEvent(int howMany) {
  // Read the mesages from I2C buss
  while(Wire.available()) // loop through all but the last
  {
    mesage = Wire.read(); // receive byte as a character
  }
}

void requestEvent() {
  // Send correct data acording to request from RPI
  switch (mesage){
    case 0: // Request for PSM Voltage 
      send2bytes(analogRead(voltageInPSM));
      break;
    case 1: // Request for PSM Current
      send2bytes(analogRead(currentInPSM));
      break;
    case 2: // Request for ESC1 Current 
      send2bytes(analogRead(currentInESC1));
      break;
    case 3: // Request for ESC2 Current 
      send2bytes(analogRead(currentInESC2));
      break;

    default:
      Serial.println("Incorect message request");
      break;
  }
}

void send2bytes(long int msg2byte) {
  /*
  I2C sends data in packages of 1 byte at the time
  1 byte is 7bits long
  but out ADC messages are between 1 bit and 10 bits!
  that is why we need to send 2 bytes from I2C
  We take the whole message and seperate it into 2 bytes
  
  First we take the message and shift by 7 bits, this makes it so that we only take in the 3 bigest bytes as the other 7 are gone
  Then we aplie a mask to those 3 bites, this ensures that if we still have some other even bigger bite values they get filtered out
  now we send the first byte consisting of the 3 bits
  Now we  take the messgae again, but we apply a mask for the first 7 bits, this filters every byte that is larger than 7 bit
  now we send the second byte consisting of the maximum 7 bits
  */
  Wire.write((msg2byte >> 7) & 0x7);
  Wire.write(msg2byte & 0x7F);
}