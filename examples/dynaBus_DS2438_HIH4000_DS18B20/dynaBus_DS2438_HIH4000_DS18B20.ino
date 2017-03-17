#include <dynaBus.h>

const uint8_t ONE_WIRE_PIN = 2;
dynaBus ds(ONE_WIRE_PIN);

void setup() {
Serial.begin(9600);
int i;
ds.begin();
ds.find();
Serial.print(ds.nb());Serial.println(" 1wire device(s)");
for(i=0;i<ds.nb();i++){
  ds.ROMtochar(i,":");
  Serial.println();
  }
}

void loop() {
  for(int i=0;i<ds.nb();i++){
    ds.ROMtochar(i);
    if(ds[i*8]==0x28) {
      Serial.print(" : DS18B20 sensor : temperature in celsius : ");
      Serial.println(ds.get28temperature(i));
    }
    if(ds[i*8]==0x26) {
      Serial.print(" : DS2436 sensor : temperature in celsius : ");
      float celsius2438 = ds.get26temperature(i);
      float vdd = ds.get26voltage(i,"vdd");
      float vad = ds.get26voltage(i,"vad");
      float rh = (vad/vdd - 0.16)/0.0062;
      float truerh = rh/(1.0546-0.00216*celsius2438);
      Serial.print(celsius2438);
      Serial.print(" - RH in % : ");
      Serial.println(truerh);
    }
  }
  Serial.println();
  delay(5000);

}
