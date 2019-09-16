#ifndef AretasSwarmBeeLE_h
#define AretasSwarmBeeLE_h

#include <Arduino.h>
#include <SoftwareSerial.h>

/**
 * I think for now we'll only support SoftwareSerial and then if necessary 
 * we'll implement the SerialProxy class I've been thinking about to abstract
 * the HardwareSerial vs SoftwareSerial usage
 */

class AretasSwarmBeeLE {
	
	public:
	
    AretasSwarmBeeLE(SoftwareSerial *softSerialPort, HardwareSerial *hardSerialPort);
    unsigned long getUniqueId();
    
    void printPacket(byte type, float value);
    boolean begin(int resetPin, int aModePin, int targetBaudRate);
    //void printPacket(long mac, byte type, int value);
    //void printPacket(long mac, byte type, long value);
    //void printPacket(long mac, byte type, unsigned long value);
    //void printPacketArr(long mac, byte type, float arr[], int sz);
    //void printAudioSamplePacket(long mac, byte type, unsigned long SAMPLE_RATE, uint16_t ADC_MAX_VAL, int arr[], int sz);
    //void printPacketArr(long mac, byte type, int arr[], int sz);
    //prints all settings using GSET to the passed in Serial port
    void printPacketMulti(byte types[], float values[], byte len);
    void printSettings();
    void printResponse(unsigned long timeout = 2000);
    void resetSettings();
    void AretasSwarmBeeLE::saveSettings();
    void setPrintResponses(boolean printResponses);

    void setERRN(boolean setting);
    void setBRAR(boolean setting);

	private:
	byte getsTimeout(char *buf, uint16_t timeout, boolean filter = false);
	SoftwareSerial *sserial;
    HardwareSerial *hwserial;
	int _targetBaudRate = 38400;
    boolean began = false;
    int _aModePin = -1;
    int _resetPin = -1;
    boolean _printResponses = false;

};

#endif