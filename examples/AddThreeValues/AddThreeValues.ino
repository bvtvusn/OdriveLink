// OdriveLink_ReadValues sketch (c) 2022 Bjørn Vegard Tveraaen

// This example is made to run on an Arduino device with two Serial ports. One Serial port is the USB and one serial port is the RX and TX pins. (Software serial can be used if you have an Arduino board, like Arduino Uno, with only one port)

// Below you can see how to read values from the Odrive in three different ways.
// The advantage of this library is that it does not hang if the Odrive does not respond to a command. Instead the callback will just not be called. 
// If you want to know if the callback has been called or not, you can make the callback activate a boolean flag.
// This library Uses Xor checksums on each command to ensure that few errors are sent/received. 
// If the signal quality is bad, the commands will not be handled (instead of prowiding a faulty value)
// Don't forget to also connect ODrive GND to Arduino GND.

#include <OdriveLink.h>
#include <Chrono.h>
Chrono myChrono; // used to send requests at regular intervals

OdriveLink _odrive;

void setup() {
  Serial.begin(115200); // For computer connection
  Serial1.begin(115200); // for Odrive Connection
  _odrive.setSerial(&Serial1);
  _odrive.setTimeout(50); // The response must now come within 50 milliseconds to be used.
}

void loop() {
  // The chrono library is used to make the code run once a second.
  if (myChrono.hasPassed(1000) ) {
    myChrono.restart();


    // Sending a request and attaching a callback function to handle the response
    _odrive.RequestString("r axis1.encoder.pos_estimate ", PositionCallback);


    // Reading bus voltage, but with an anonymuous callback function.
    _odrive.RequestString("r vbus_voltage ", [](char str[], int length) {
      // On Response:
      Serial.print("bus voltage: ");
      float inVal = OdriveLink::CharsToFloat(str, length);
      Serial.println(inVal);
    });


    // Get speed and position of motor:
    _odrive.RequestString("f 0 ", [](char str[], int length) {
      float pos, sp;
      _odrive.charsToFloats(str, " ", pos, sp);
      Serial.print("Motor 0 pos: ");
      Serial.println(pos);
      Serial.print("Motor 0 speed: ");
      Serial.println(sp);
    });
  }


  _odrive.ProcessInputs(); // This must be run continuously to handle messages from the odrive and calling the callbacks.

  
}

void PositionCallback(char str[], int length) {
  Serial.print("Position: ");
  float inVal = OdriveLink::CharsToFloat(str, length);
  Serial.println(inVal);
}
