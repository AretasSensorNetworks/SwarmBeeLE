#include "Arduino.h"
#include "AretasSwarmBeeLE.h"
#include <string.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>

#define BUF_LENGTH 30 //may need to increase for this application

#define SBEE_DEBUG1 1
//#define SBEE_DEBUG2 2

const char *atCmdSuas = "SUAS"; //get low serial number
const char *atCmdBrar = "BRAR"; //set broadcast ranging result op
const char *atCmdErrn = "ERRN"; //set enable ranging result notifications
const char *atCmdEdan = "EDAN"; //enables disables data notifications
const char *atCmdSdat = "FNIN"; //fill data into node id notification packets 
const char *atCmdEbid = "EBID"; //enable broadcast id of node id blink packets
const char *atCmdSbiv = "SBIV"; //sets the broadcast interval value (blink rate)
const char *atCmdGnid = "GNID"; //reads the configured node id
const char *atCmdSset = "SSET"; //saves all settings including node id permanently to eeprom
const char *atCmdRset = "RSET"; //restores all parameter settings from eeprom
const char *atCmdGset = "GSET";
const char *atCmdFnin = "FNIN";
const char *atCmdSpc = " ";
const char *atCmdTerminator = "\n";

AretasSwarmBeeLE::AretasSwarmBeeLE(SoftwareSerial *softSerialPort, HardwareSerial *hardSerialPort){
    sserial = softSerialPort;
    hwserial = hardSerialPort;
}

void AretasSwarmBeeLE::setPrintResponses(boolean printResponses){
    _printResponses = printResponses;
}

boolean AretasSwarmBeeLE::begin(int resetPin, int aModePin, int targetBaudRate){

    _targetBaudRate = targetBaudRate;
    
    //reset the module
    //we have to reset the module otherwise we can't seem to write to it 
    //noticed this behaviour on other tests on FTDI and other serial (so it's not just our board)
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(2000);
    digitalWrite(resetPin, HIGH);
    delay(1000);

    //negotiate a lower baud rate 
    //we cannot handle 115200 and that's the stock baud rate of the module therefore we need to 
    //try and negotiate a baud rate we can handle
    //don't mess with this code, every time I change even the smallest thing, it quits working
    //it's absolutely crucial to negotiate the new baudrate as the first thing the module does when it boots up
    //and the atmega is starting up - softwareserial @ 8MHZ , 115200 is totally impractical, 
    //so it's a small miracle that this works on our 8MHZ boards
    sserial->begin(115200);
    delay(10);
    sserial->print(atCmdSuas);
    sserial->print(atCmdSpc);
    sserial->print(targetBaudRate);
    sserial->print(atCmdTerminator);
    sserial->end();
    sserial->begin(targetBaudRate);
    delay(100);

    if(_printResponses == true)
        printResponse();

}



void AretasSwarmBeeLE::printPacketMulti(byte *stypes, float *values, byte NUM_SENSORS){

    sserial->begin(_targetBaudRate);

    char packet[3] = {0,0,0}; //placeholder for ascii byte packet

    union {
        float fval;
        byte bval[4];
    } floatToBytes;

    //print the FNIN command
    sserial->print(atCmdFnin);
    sserial->print(atCmdSpc);

    #ifdef SBEE_DEBUG1

    Serial.print("\n\n");
    #ifdef SBEE_DEBUG2
    Serial.print("NUM SENSORS:"); Serial.print(NUM_SENSORS); Serial.print('\n');

    for(int kk = 0; kk < NUM_SENSORS; kk++){
        Serial.print("TYPE:");
        Serial.print(stypes[kk]);
        Serial.print('\n');
    }
    #endif
    Serial.print(atCmdFnin);
    Serial.print(atCmdSpc);

    #endif
    
    //print a two byte packet length
    //each packet is one byte for type and 4 bytes for the serialized float32 
    byte packet_length = 0;
    packet_length = NUM_SENSORS * 5;

    sprintf(packet, "%02X", packet_length);

    sserial->print(packet);
    sserial->print(atCmdSpc);

    #ifdef SBEE_DEBUG1
    Serial.print(packet);
    Serial.print(atCmdSpc);
    #endif

    int z = 0;
    for(; z < NUM_SENSORS; z++){
        
        #ifdef SBEE_DEBUG2
            Serial.print("\n\n");
            Serial.print("TYPE"); Serial.print("["); Serial.print(z); Serial.print("]:");
            Serial.print(stypes[z]);
            Serial.print("\n\n");
        #endif

        sprintf(packet, "%02X", stypes[z]);

        sserial->print(packet);

        #ifdef SBEE_DEBUG1
        Serial.print(packet);
        #endif

        //print the float
        floatToBytes.fval = values[z];

        int j = 0;
        for(; j < 4; j++){

            sprintf(packet, "%02X", (uint16_t)floatToBytes.bval[j]);
            sserial->print(packet);

            #ifdef SBEE_DEBUG1
            Serial.print(packet);
            #endif
        }

    }

    sserial->print(atCmdTerminator);

    #ifdef SBEE_DEBUG1
    Serial.print(atCmdTerminator);
    #endif

    delay(10);
    if(_printResponses == true)
        printResponse();

}

void AretasSwarmBeeLE::printPacket(byte type, float value){

    sserial->begin(_targetBaudRate);
    //FNIN command looks like: FNIN 0f f800008042f600009841b50000eb43
    //above example has 3 sensor readings

    char packet[2]; //placeholder for ascii byte packet

    union {
        float fval;
        byte bval[4];
    } floatAsBytes;

    //print the FNIN command
    sserial->print(atCmdFnin);
    
    //print a two byte packet length 
    byte packet_length = 5;

    sprintf(packet, "%02X", packet_length);

    sserial->print(packet);
    sserial->print(atCmdSpc);

    sprintf(packet, "%02X", type);
    sserial->print(packet);

    //print the float
    floatAsBytes.fval = value;

    int i = 0;
    for(i = 0; i < 4; i++){
        sprintf(packet, "%02X", floatAsBytes.bval[i]);
        sserial->print(packet);
    }

    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses == true)
        printResponse();
}

unsigned long AretasSwarmBeeLE::getUniqueId(){
    
    sserial->begin(_targetBaudRate);
    char buf[BUF_LENGTH];

	sserial->print(atCmdGnid);
    sserial->print(atCmdTerminator);

	unsigned long ul = 0;

    //filter out the '=' response
    byte len = getsTimeout(buf, 1000, true);
    if(len > 0){
        ul = strtoul(buf, NULL, 16);
    }

	return ul;

}

void AretasSwarmBeeLE::setERRN(boolean setting){

    sserial->begin(_targetBaudRate);
    char s;
    switch(setting){
        case false:
            s = '0';
            break;
        case true:
            s = '1';
            break;
    }

    sserial->print(atCmdErrn);
    sserial->print(atCmdSpc);
    sserial->print(s);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses){
        printResponse();
    }
}

void AretasSwarmBeeLE::setBRAR(boolean setting){

    sserial->begin(_targetBaudRate);
    char s;
    switch(setting){
        case false:
            s = '0';
            break;
        case true:
            s = '1';
            break;
    }

    sserial->print(atCmdBrar);
    sserial->print(atCmdSpc);
    sserial->print(s);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses){
        printResponse();
    }
}

void AretasSwarmBeeLE::setBlinkInterval(long blinkInterval){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdSbiv);
    sserial->print(atCmdSpc);
    sserial->print(blinkInterval);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses == true)
        printResponse();

}

void AretasSwarmBeeLE::printSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdGset);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses == true)
        printResponse();
    
}

void AretasSwarmBeeLE::resetSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdRset);
    sserial->print(atCmdTerminator);
    //arbitrary delay to wait for the settings to persist
    delay(30);
    if(_printResponses == true)
        printResponse();

}

void AretasSwarmBeeLE::saveSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdSset);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses){
        printResponse();
    }
}

void AretasSwarmBeeLE::printResponse(unsigned long timeout = 2000){

    sserial->listen();

    unsigned long start = millis();
    while(sserial->available() > 0){
        hwserial->print((char)sserial->read());
        if((millis() - start) > timeout){
            break;
        }
    }

}

byte AretasSwarmBeeLE::getsTimeout(char *buf, uint16_t timeout, boolean filter = false) {
    
    byte count = 0;
    long timeIsOut = 0;
    byte c;
    *buf = 0;
    timeIsOut = millis() + timeout;

    sserial->listen();
    
    while (timeIsOut > millis() && count < (BUF_LENGTH - 1)) {
        
        if (sserial->available() > 0) {
            count++;
            c = sserial->read();
            if(filter == true)
                if(c == '=') continue;
            *buf++ = c;
            timeIsOut = millis() + timeout;
        }
    }
    
    if (count != 0) {
        *buf = 0;
        count++;
    }
    
    return count;
}

