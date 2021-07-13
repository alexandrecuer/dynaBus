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

#ifndef dynaBus_h
#define dynaBus_h

#include "Arduino.h"
#include "OneWire.h"

class dynaBus : public OneWire {
public:
  dynaBus(uint8_t pin) : OneWire(pin) {};
  void begin();
  int nb();
  uint8_t operator[](int index) const;
  void ROMtochar(byte j, const char* separator="", Print &print=Serial);
  void find();
  float get28temperature(byte j);
  float get26temperature(byte j);
  //mode can be "vdd" or "vad"
  float get26voltage(byte j, const char mode[3]);
  bool read26PageZero(byte j, uint8_t *data);
  void write26PageZero(byte j, uint8_t *data);
private:
  byte _nb;
  //single wire devices have 8 bytes ROM
  //byte i of ROM number j will be _addrt[8*j+i]
  byte *_addrt;
};

#endif
