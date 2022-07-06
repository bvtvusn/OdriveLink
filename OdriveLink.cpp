/*
  OdriveLink.cpp - Library for Communicating with the Odrive BLDC motor controller.
  Created by Bjørn Vegard Tveraaen, July 25, 2022.
  Released into the public domain.
*/

#include "Arduino.h"
#include "OdriveLink.h"


void OdriveLink::setSerial(Stream *stream){
	OdriveStream = stream;
}


void OdriveLink::setTimeout(long ms){
    commandTimeout = ms;
}

// ################################### Requests: #####################################//


void OdriveLink::RequestString(String command, charParamFuncPtr callback){
  AttachCallback(callback);
  SendWithChecksum(command);
}

void OdriveLink::AttachCallback(charParamFuncPtr callback ) {
  // Try to remove the oldest callback (if it is too old)
  if (millis() - callbackAttachedTime[callbackIndex] > commandTimeout) {
    RemoveFirstCallback();
    //Serial.println("Removed Old Callback");
  }
  if (callbacksWaiting < commandBufferLength) {
    int insertIndex = (callbackIndex + callbacksWaiting ) % commandBufferLength;
    methodArray[insertIndex] = callback;
    callbacksWaiting = min(callbacksWaiting + 1, commandBufferLength);
    callbackAttachedTime[insertIndex] = millis();
    //Serial.print("insertIndex: "); Serial.println(insertIndex);
    //Serial.print("AttachTime: "); Serial.println(millis());
  } else {
    // Send warning because the callback buffer is full.
    //Serial.println("Warning, bufferis full");
  }
}


void OdriveLink::SendWithChecksum(String command){
  command.toCharArray(transmitBuffer, bufferLength);  // Fill the transmit buffer
  int lenOut = AppendChecksum(transmitBuffer, command.length() ); // Append checksum to transmit buffer
  OdriveStream->write(transmitBuffer, lenOut);
}

int OdriveLink::AppendChecksum(char command[], int len) {
  byte sum = 0;
  for (int i = 0; i < len; i++) {
    sum ^= command[i];
  }
  String checksumString = String(sum);
  int checksumChars = checksumString.length();
  checksumString.toCharArray(&transmitBuffer[len + 1], bufferLength);
  transmitBuffer[len] = '*';
  transmitBuffer[checksumChars + len + 1] = '\n';
  return checksumChars + len + 2; // Should be transmit length
}

void OdriveLink::RemoveFirstCallback() {
  callbacksWaiting = max(0, callbacksWaiting - 1); // Reduce by one but not below zero.
  callbackIndex = (callbackIndex + 1) % commandBufferLength;
}

// ################################### Responses: #####################################//

void OdriveLink::ProcessInputs() {
  while (OdriveStream->available()) {
    char character = OdriveStream->read();
    if (character == '\n') {
      HandleLine(receiveBuffer, lineLength);
      lineLength = 0;
    }
    else {
      receiveBuffer[lineLength] = character;
      lineLength += 1;
    }
  }
}

void OdriveLink::HandleLine(char line[], int length) {
  // Check Checksum; if bad, increment the error count.
  int asterixPos = -1;
  bool isOK = CheckChecksum(line, length, &asterixPos); // Asterix pos is an "Out" parameter
  line[asterixPos] = '\0'; // inserting null character to end the string before the checksum starts
  if (asterixPos != -1) length = asterixPos;
  if (isOK) {
    ActivateCallback(line, length);
    //else // Increment the error counter
  }
}

bool OdriveLink::CheckChecksum(char testStr[], int length, int* asterixIndex) {
  int i = 0;
  byte sum = 0;
  *asterixIndex = -1;
  while (*asterixIndex == -1 && i < length ) {
    if (testStr[i] == '*') {
      *asterixIndex = i;
    }
    else {
      sum ^= testStr[i]; // Calc checksum;
    }
    i++;
  }
  if (*asterixIndex == -1) return true; // No checksum is supplied, so just say "OK".
  //Serial.println(sum);
  String strNumber = "";
  for (int j = 0; j < min(3, length - 1 - *asterixIndex); j++) {
    strNumber.concat(testStr[*asterixIndex + 1 + j]);
  }
  byte suppliedChecksum = strNumber.toInt();
  return (suppliedChecksum == sum);
}

void OdriveLink::ActivateCallback(char line[], int length) {
  while (callbacksWaiting > 0) {
    if (millis() - callbackAttachedTime[callbackIndex] > commandTimeout) {
      // The waiting callback is too old.
      RemoveFirstCallback();
    }
    else {
      // Call the callback function and return.
      methodArray[callbackIndex](line,length); // CALL CALLBACK
      RemoveFirstCallback();
      return; // RETURN
    }
  }
}

// ### Other functions ###
float OdriveLink::CharsToFloat(char line[], int length) { // A static function (even though it does not use the static keyword here. Only in the header file.)
  String strNumber = "";
  for (int j = 0; j < length ; j++) {
    strNumber.concat(line[j]);
  }
  return strNumber.toFloat();
}

void OdriveLink::charsToFloats(char str[], char* delim, float& float1, float& float2 ){ 
      char *token = strtok(str, delim);
      float1 = OdriveLink::CharsToFloat(token, strlen(token));
      token = strtok(NULL, delim);
      float2 = OdriveLink::CharsToFloat(token, strlen(token));
}