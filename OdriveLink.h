#ifndef ODRIVELINK_H
#define ODRIVELINK_H
#include "Arduino.h"

#define commandBufferLength (10)
#define bufferLength (100)

// Future work: Count errors (timeout errors, invalid checksum, full callbackBuffer)
// Make Callback with error messages.

class OdriveLink {
  
public:
	typedef void (*charParamFuncPtr)(char str[], int length); // typedef for function pointer to method with float input parameter.


	// Static functions:
	static float CharsToFloat(char line[], int length);

	// ################################### Requests: #####################################//
	void RequestString(String command, charParamFuncPtr callback);
	void SendWithChecksum(String command);
	// ################################### Responses: #####################################//
	void ProcessInputs();
	// Other Functions:
	void setTimeout(long ms);
	void setSerial(Stream * stream);
	void charsToFloats(char str[], char* delim, float& float1, float& float2 );
 

  private:
	Stream * OdriveStream; // Serial conection

	charParamFuncPtr methodArray[commandBufferLength]; // An array containing all the method pointers.
	unsigned long callbackAttachedTime[commandBufferLength];
	unsigned long commandTimeout = 200; // If no response comes within x milliseconds, it will not be answered.
	int callbackIndex = 0;
	int callbacksWaiting = 0;
	char receiveBuffer[bufferLength];
	char transmitBuffer[bufferLength];
	int lineLength = 0;

	// ################################### Requests: #####################################//
	void AttachCallback(charParamFuncPtr callback );
	int AppendChecksum(char command[], int len);
	void RemoveFirstCallback();
	// ################################### Responses: #####################################//
	void HandleLine(char line[], int length);
	bool CheckChecksum(char testStr[], int length, int* asterixIndex);
	void ActivateCallback(char line[], int length);
};

#endif