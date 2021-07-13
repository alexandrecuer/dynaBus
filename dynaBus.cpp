/*
Copyright (c) 2016, Alexandre CUER (alexandre.cuer@wanadoo.fr)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------
*/

#include "Arduino.h"
#include "OneWire.h"
#include "dynaBus.h"

void dynaBus::begin(){
    _nb=0;
}

int dynaBus::nb(){
    return _nb;
}
 
uint8_t dynaBus::operator[](int index) const
{
    return _addrt[index];
}

//FIX ME - verify if LCD print is OK
//print the ROM j - default is Serial
//we want to print all zero even non significative ones
//the core of this fonction comes from ethersia by Nicholas Humfrey
void dynaBus::ROMtochar(byte j, const char* separator, Print &p){
    char str[2];
    for(int i=0;i<8;i++){
        str[0] = (_addrt[8*j+i] >> 4) & 0x0f;
        str[1] = _addrt[8*j+i] & 0x0f;
        for (int j=0; j<2; j++) {
            // base for converting single digit numbers to ASCII is 48
            // base for 10-16 to become lower-case characters a-f is 87
            if (str[j] > 9) str[j] += 39;
            str[j] += 48;
            p.print(str[j]);
        }
    if(i<7)p.print(separator);
    }
}

void dynaBus::find(){
    byte addr[8];
    byte i;
    int n=0;
    while(search(addr))_nb++;
    _addrt=(uint8_t *)malloc(_nb*8);
    //filling _addrt with the single wire ROM byte
    while(search(addr)){for(i=0;i<8;i++)_addrt[n*8+i]=addr[i];n++;}
}

float dynaBus::get28temperature(byte j){
    byte i;
    byte data[9];
    uint8_t addr[8];
    for(i=0;i<8;i++)addr[i]=_addrt[j*8+i];
    reset();
    select(addr);
    //temperature conversion
    write(0x44, 1);
    delay(1000);
    reset();
    select(addr);
    //read Scratchpad
    write(0xBE);
    //we read 9 bytes - byte 8 is a CRC error code
    for ( i = 0; i < 9; i++)data[i] = read();
    /*
    for ( i = 0; i < 9; i++) {
      Serial.print(data[i], HEX); 
      Serial.print(" ");
    }
    Serial.print(" CRC="); 
    Serial.print(crc8(data, 8), HEX);
    Serial.println();
    */
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    if(crc8(data, 8) == data[8]) return (float)raw / 16.0;
    else return 10000000000;
}

float dynaBus::get26temperature(byte j){
    byte i;
    uint8_t data[9];
    uint8_t addr[8];
    for(i=0;i<8;i++)addr[i]=_addrt[j*8+i];
    reset();
    select(addr);
    //temperature conversion 
    write(0x44, 0);
    //temperature delay 
    delay(10);
    if (read26PageZero(j,data)) return (double)(((((int16_t)data[2]) << 8) | (data[1] & 0x0ff)) >> 3) * 0.03125;
    else return 10000000000;
}

float dynaBus::get26voltage(byte j, const char mode[3]){
    byte i;
    uint8_t data[9];
    uint8_t addr[8];
    for(i=0;i<8;i++)addr[i]=_addrt[j*8+i];
    
    read26PageZero(j,data);
    
    //data[0] | 0x08 > bit 5 set to 1 in byte 0 page 0 > VDD selection 
    //data[0] & 0xF7 > bit 5 set to 0 in byte 0 page 0 > VAD selection
    if(mode=="vdd")data[0] = data[0] | 0x08;
    if(mode=="vad")data[0] = data[0] & 0xF7;
    
    /*
    for(int k=0;k<9;k++){Serial.print(data[k],HEX);Serial.print(" ");}
    Serial.println();
    */
    write26PageZero(j,data);
    
    reset();
    select(addr);
    //voltage conversion
    write(0xb4, 0);
    //voltage delay
    delay(8);
    
    if(read26PageZero(j,data))
      {
        /*
        for(int k=0;k<9;k++){Serial.print(data[k],HEX);Serial.print(" ");}
        Serial.println();
        */
        return (((data[4] << 8) & 0x00300) | (data[3] & 0x0ff)) / 100.0;
      }
    else return 10000000000;
}

bool dynaBus::read26PageZero(byte j, uint8_t *data) {
    byte i;
    uint8_t addr[8];
    for(i=0;i<8;i++)addr[i]=_addrt[j*8+i];
    reset();
    select(addr);
    //recall memory command
    write(0xb8, 0);
    write(0x00, 0);
    reset();
    select(addr);
   //read scratchpad 
    write(0xbe, 0);
    write(0x00, 0);
    //we read 9 bytes - byte 8 is a CRC error code
    for (int i = 0; i < 9; i++)data[i] = read();
    return crc8(data, 8) == data[8];
}

void dynaBus::write26PageZero(byte j, uint8_t *data) {
    byte i;
    uint8_t addr[8];
    for(i=0;i<8;i++)addr[i]=_addrt[j*8+i];
    reset();
    select(addr);
    //write scratchpad 
    write(0x4e, 0);
    write(0x00, 0);
    for (int i = 0; i < 8; i++)write(data[i], 0);
    reset();
    select(addr);
    //copy scratchpad 
    write(0x48, 0);
    write(0x00, 0);
}
