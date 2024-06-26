# SwarmBeeLE

Firmware and other resources for the Aretas SwarmBeeLE enabled products

## Overview

This repository contains the Arduino library for communicating with the SwarmBeeLE V2 UART. The library provides a comprehensive set of functions to interact with the SwarmBeeLE module, enabling features like ranging, broadcasting, and sensor data transmission.

## Arduino Notes

The Arduino library allows you to communicate with the SwarmBeeLE V2 UART.

### Hardware Connections

- **RESET Pin**: The library requires at least the RESET pin to be connected to a digital I/O pin to initiate the reset.
- **A_MODE Pin**: If you're using an Aretas SwarmBee adapter module, you'll likely also want to connect A_MODE either to a digital I/O pin or to GND.

### Baud Rate Considerations

The SwarmBeeLE is hardcoded at boot time to 115200 baud, which is not typically feasible with software UART on slower 8MHz boards. Therefore, the library features a bit of a workaround to get it working with lower-speed boards. In the `begin()` routine, the library resets the SwarmBee module, opens the port at 115200, and immediately tries to negotiate a lower baud rate. It seems to work, but your mileage may vary. It's recommended to try and use either a higher-speed board that can support SoftwareSerial baud rates of 115200 reliably or use a hardware UART.

### Tested Configurations

The library has been successfully tested with multiple SoftwareSerial ports running at different speeds, connecting different sensors.

## Library Functions

### Initialization

```cpp
AretasSwarmBeeLE::AretasSwarmBeeLE(SoftwareSerial *softSerialPort, HardwareSerial *hardSerialPort);
boolean AretasSwarmBeeLE::begin(int resetPin, int aModePin, int targetBaudRate);
void AretasSwarmBeeLE::setPrintResponses(boolean printResponses);
```

- **AretasSwarmBeeLE**: Constructor to initialize the library with software and hardware serial ports.
- **begin**: Initializes the SwarmBeeLE module with the specified reset pin, A_MODE pin, and target baud rate.
- **setPrintResponses**: Sets whether the library should print responses from the module.

### Command Functions

```cpp
void AretasSwarmBeeLE::printPacketMulti(byte *stypes, float *values, byte NUM_SENSORS);
void AretasSwarmBeeLE::printPacket(byte type, float value);
unsigned long AretasSwarmBeeLE::getUniqueId();
void AretasSwarmBeeLE::setERRN(boolean setting);
void AretasSwarmBeeLE::setBRAR(boolean setting);
void AretasSwarmBeeLE::setEMSS(boolean setting);
void AretasSwarmBeeLE::setEBMS(boolean setting);
void AretasSwarmBeeLE::setEIDN(boolean setting);
void AretasSwarmBeeLE::setEDNI(boolean setting);
void AretasSwarmBeeLE::setEDAN(boolean setting);
void AretasSwarmBeeLE::setEBID(boolean setting);
void AretasSwarmBeeLE::setBlinkInterval(long blinkInterval);
void AretasSwarmBeeLE::printSettings();
void AretasSwarmBeeLE::resetSettings();
void AretasSwarmBeeLE::saveSettings();
```

- **printPacketMulti**: Sends multiple sensor data packets.
- **printPacket**: Sends a single sensor data packet.
- **getUniqueId**: Retrieves the unique node ID.
- **setERRN**: Enables or disables ranging result notifications.
- **setBRAR**: Enables or disables broadcast ranging results.
- **setEMSS**: Enables or disables the MEMS sensor.
- **setEBMS**: Enables or disables the broadcast of MEMS values.
- **setEIDN**: Enables or disables node ID broadcast notification.
- **setEDNI**: (Pending)
- **setEDAN**: Enables or disables data notifications.
- **setEBID**: Enables or disables the broadcast ID of node ID blink packets.
- **setBlinkInterval**: Sets the blink interval in milliseconds.
- **printSettings**: Prints the current settings of the module.
- **resetSettings**: Resets the module settings to default.
- **saveSettings**: Saves runtime configuration changes to permanent memory.

### Utility Functions

```cpp
void AretasSwarmBeeLE::printResponse(unsigned long timeout = 2000);
byte AretasSwarmBeeLE::getsTimeout(char *buf, uint16_t timeout, boolean filter = false);
```

- **printResponse**: Prints the response from the module.
- **getsTimeout**: Reads data from the module with a timeout.

## Example Usage

```cpp
#include <SoftwareSerial.h>
#include "AretasSwarmBeeLE.h"

// Define pins
#define RESET_PIN 2
#define A_MODE_PIN 3

// Initialize SoftwareSerial
SoftwareSerial softSerial(10, 11); // RX, TX

// Initialize AretasSwarmBeeLE
AretasSwarmBeeLE swarmBee(&softSerial, &Serial);

void setup() {
  Serial.begin(9600);
  softSerial.begin(9600);
  swarmBee.setPrintResponses(true);
  swarmBee.begin(RESET_PIN, A_MODE_PIN, 9600);
}

void loop() {
  byte sensorTypes[] = {0x01, 0x02};
  float sensorValues[] = {23.4, 56.7};
  swarmBee.printPacketMulti(sensorTypes, sensorValues, 2);
  delay(1000);
}
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to Aretas for providing the hardware and support.
- Special thanks to the contributors and community for their continuous support.

For more information, visit the [Aretas Website](https://aretas.ca).
