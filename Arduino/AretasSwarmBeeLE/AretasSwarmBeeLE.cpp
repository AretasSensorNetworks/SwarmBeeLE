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
const char *atCmdGset = "GSET"; //get all settings
const char *atCmdFnin = "FNIN"; //insert payload data
const char *atCmdEmss = "EMSS"; //enables and disables mems sensor on the module
const char *atCmdEbms = "EBMS"; //enables and disables broadcast of MEMS values (including temperature)
const char *atCmdEidn = "EIDN"; //enables and disables node id broadcast notification
const char *atCmdEdni = "EDNI"; //pending
const char *atCmdSdmd = "SDMD"; //pending
const char *atCmdSrob = "SROB"; //Selects Ranging Operation Blinks; sets which classes of devices the node will initiate a ranging operation with upon reception of a node blink ID packet.
const char *atCmdGpio = "GPIO"; //GPIO configurations //GPIO 0 2 //GPIO 0 as wake - up with rising edge (internally in this case a pulldown is switched, i.e. open input is LOW).
const char *atCmdIcfg = "ICFG"; //interrupt configuration  ICFG 0001 //only INT for GPIO 0 with rising edge**

const char *atCmdSpc = " ";
const char *atCmdTerminator = "\n";

AretasSwarmBeeLE::AretasSwarmBeeLE(SoftwareSerial *softSerialPort, HardwareSerial *hardSerialPort){
    sserial = softSerialPort;
    hwserial = hardSerialPort;
}

void AretasSwarmBeeLE::setPrintResponses(boolean printResponses){
    _printResponses = printResponses;
}
/**
 * we need to call this begin function to setup baud rate negotiation and reset the module
 * the reset is necessary in certain circumstances so we can be sure the baud rate is reset to default
 * plus it's good to trigger the reset so we know the module is configured to defaults 
 * and we've had a few issues with the modules not starting up cleanly as VCC comes up
 * on low speed 8MHZ boards with SoftwareSerial, we need a kludge to get the module into a lower target baud rate
 */
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

/**
 * implementation of a multiple sensor data broadcast 
 * we can serialize up to ~11 readings - the payload is ~60 bytes max and we need 5 bytes per sensor reading
 * one byte for the type and 4 bytes for the serialized float
 * @param *stypes the array of sensor types
 * @param *values the array of float values
 * @param NUM_SENSORS the size of the arrays
 */

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
/**
 * implementation of the Aretas printpacket command to send sensor data over the 
 * SwarmBee module in a format compatible for middleware decoding
 * 
 * @param type the sensor type
 * @param value the sensor reading
 */
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

/**
 * this gets us the NID for the module for provisioning purposes
 */
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

/**
 * Represents a type of boolean (0 or 1) type of command
 * such as BRAR 0 (which enabled or disabled broadcast ranging results)
 * we could print the decimal but I prefer being pedantic for clarity
 *  and switching and printing a char
 * @param setting the boolean 0 or 1 setting to enable / disable the setting
 * @param cmd the command (such as BRAR)
 */
void AretasSwarmBeeLE::boolCmd(boolean setting, const char *cmd){

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

    sserial->print(cmd);
    sserial->print(atCmdSpc);
    sserial->print(s);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses){
        printResponse();
    }

}

void AretasSwarmBeeLE::setERRN(boolean setting){
    boolCmd(setting, atCmdErrn);
}

void AretasSwarmBeeLE::setBRAR(boolean setting){
    boolCmd(setting, atCmdBrar);
}

void AretasSwarmBeeLE::setEMSS(boolean setting){
    boolCmd(setting, atCmdEmss);
}
void AretasSwarmBeeLE::setEBMS(boolean setting){
    boolCmd(setting, atCmdEbms); 
}

void AretasSwarmBeeLE::setEIDN(boolean setting){
    boolCmd(setting, atCmdEidn);
}
void AretasSwarmBeeLE::setEDNI(boolean setting){
    boolCmd(setting, atCmdEdni);
}
void AretasSwarmBeeLE::setEDAN(boolean setting){
    boolCmd(setting, atCmdEdan);
}
void AretasSwarmBeeLE::setEBID(boolean setting){
    boolCmd(setting, atCmdEbid);
}

void AretasSwarmBeeLE::setSROB(int setting){}
void AretasSwarmBeeLE::setICFG(int setting){}
void AretasSwarmBeeLE::setGPIO(int gpio, int setting){}

/**
 * an accessible function for setting the blink rate 
 * @param blinkInterval the blink interval in milliseconds
 * standard range is 0 - 65000 however for practical purposes
 * 100-200ms are the general minimums we've tested
 */
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

/**
 * print the settings stored in the module to the hardware serial hardSerialPort
 */
void AretasSwarmBeeLE::printSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdGset);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses == true)
        printResponse();
    
}

/**
 * reset the module settings to the default
 */
void AretasSwarmBeeLE::resetSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdRset);
    sserial->print(atCmdTerminator);
    //arbitrary delay to wait for the settings to persist
    delay(30);
    if(_printResponses == true)
        printResponse();

}

/** 
 * save any runtime config changes to permanent memory
 */
void AretasSwarmBeeLE::saveSettings(){

    sserial->begin(_targetBaudRate);
    sserial->print(atCmdSset);
    sserial->print(atCmdTerminator);
    delay(10);
    if(_printResponses){
        printResponse();
    }
}

/**
 * utility function for printing the module response 
 * (note that we don't parse cmnd responses right now)
 */
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

