# SwarmBeeLE
Firmware and other resources for the Aretas SwarmBeeLE enabled products 

## Arduino Notes

The Arduino library allows you to communicate with the SwarmBeeLE V2 UART

The Library needs at least the RESET pin connected to a digital I/O pin to initiate the reset. If you're using an Aretas SwarmBee adapter module, then you'll likely also want to connect A_MODE either to a digital I/O pin or to GND 

The SwarmBeeLE is hardcoded at boot time to 115200 baud which is not typically feasible with software UART on slower 8MHZ boards. Therefore the library features a bit of a kludge to get it working with lower speed boards. In the begin() routine, the library resets the SwarmBee module and opens the port at 115200 and immediately tries to negotiate a lower baud rate. It seems to work, but YMMV. It's recommended to try and use either a higher speed board that can support SoftwareSerial baud rates of 115200 reliably or use a hardware UART. 

The library has been successfully tested with multiple SoftwareSerial ports running at different speeds connecting different sensors. 

More documentation to come. 
