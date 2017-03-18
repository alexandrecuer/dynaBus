# dynaBus
1wire customized library for DS2438 HIH4000 and DS18B20
Bus

Single Wire Bus - a subclass of the OneWire library

OneWire library is maintained by Paul Stoffregen
http://www.pjrc.com/teensy/td_libs_OneWire.html
or
https://github.com/PaulStoffregen/OneWire

Bus class scans the 1 wire Bus connected to an arduino UNO analog pin and stores the ROMs in an array
1-wire components are commercialized by Maxim Integrated Products, Inc.
https://www.maximintegrated.com/en/products/digital/one-wire.html

It has been tested with two configurations, corresponding to my needs :

    one DS2438
    one DS2438 and one DS18B20

In all cases, an analog humidity sensor HIH4000 (Honeywell International Inc.) was connected to the DS2438 VAD input

Anyway it should work with more than 2 devices on the bus

Please note that you need to install the OneWire library first in your arduino library directory

You can find various PCB designs in the PCB subdirectory in order to construct your own single wire Bus

# Usage

### dynaBus bus (uint8_t pin)
Create the Bus object, using a specific pin

### bus.begin()
Initialize the bus with a number of devices equal to zero

### bus.void find()
scan the bus and stores the ROMs in an the ROM array

### bus.nb()
return the number of devices on the bus

### bus[i]
return the byte i stored in the ROM array

### bus.ROMtochar(byte j, const char* separator="", Print &print=Serial)
print the ROM number j to char

### bus.get28temperature(byte j)
return temperature from the device j which is assumed to be a DS1820 sensor

### bus.get26temperature(byte j)
return temperature from the device j which is assumed to be a DS2438 sensor

### bus.get26voltage(byte j, char *mode)
return the vdd of vad voltage on the device j which is assumed to be a DS2438 sensor
mode can be "vdd" or "vad"

### bus.read26PageZero(byte j, uint8_t *data)
read page zero on the device j which is assumed to be a DS2438 sensor, and stores the result in data
could be private only - used by get26voltage and get26temperature

### bus.write26PageZero(byte j, uint8_t *data)
write page zero on the device j which is assumed to be a DS2438 sensor
could be private only - used by get26voltage
