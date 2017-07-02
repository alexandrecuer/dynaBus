//JUNE 2017
//HARDWARE IPV4 WITH WIZNET SHIELD
//ApplicationMonitor from Megunolink
//See https://github.com/Megunolink/ArduinoCrashMonitor

#include <Arduino.h>
#include "ApplicationMonitor.h"
 
Watchdog::CApplicationMonitor ApplicationMonitor;

// number of iterations completed. 
int g_nIterations = 0;     

#include <dynaBus.h>

const uint8_t ONE_WIRE_PIN = 2;
dynaBus ds(ONE_WIRE_PIN);

#include <SPI.h>
#include <Ethernet.h>

unsigned long lastConnectionTime = 0;
boolean lastConnected = false;
const unsigned long postingInterval = 30000L;

class PacketBuffer : public Print {
public:
    PacketBuffer () : fill (0) {}
    const char* buffer() { return buf; }
    byte length() { return fill; }
    void reset()
    { 
      memset(buf,0,sizeof(buf));
      fill = 0; 
    }
    virtual size_t write (uint8_t ch)
        { if (fill < sizeof buf) buf[fill++] = ch; }
private:
    byte fill;
    char buf[200];
};
PacketBuffer str;

//arduino parameters : mac adress and IP
byte mac[6];
IPAddress ip(172,20,83,100);
//IPAddress ip(192,168,1,3);

//emoncms server parameters
//IPAddress emon_ip(192,168,1,27);
IPAddress emon_ip(172,20,83,120);
//emonCMS apikey
char apikey[] = "your_32_bits_API_Key";

EthernetClient client;

void setup()
{  
  Serial.begin(9600);
  //search for OneWire devices
  ds.begin();
  ds.find();
  Serial.print(ds.nb());Serial.println(" 1wire device(s)");
  /**
   * single wire 64-bit ROM : first byte is family
   * bytes 1 to 6 represent the unique serial code
   * byte 7 contain the CRC
   * * * * * * * * * * * 
   * in our specific case, we don't use byte 1 to generate the MAC and we use a fixed param 0x6e
   * this permits to have a local unicast MAC 
   */
  if(ds.nb()) {
    mac[0]=0x6e;mac[1]=ds[2];mac[2]=ds[3];mac[3]=ds[4];mac[4]=ds[5];mac[5]=ds[6];
  }
  Ethernet.begin(mac, ip);
  Serial.print("client is at ");
  Serial.println(Ethernet.localIP());
  // give the ethernet module time to boot up:
  delay(1000);
  
  ApplicationMonitor.Dump(Serial);
  ApplicationMonitor.EnableWatchdog(Watchdog::CApplicationMonitor::Timeout_8s);

}

void loop()
{
  ApplicationMonitor.IAmAlive();
  ApplicationMonitor.SetData(g_nIterations++);
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
  // if the server's disconnected, stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println("disconnecting.");
    client.stop();
  }
  unsigned long now = millis();
  // if you get a connection, report back via serial:
  if (!client.connected() && (now-lastConnectionTime > postingInterval))
    {
    lastConnectionTime = now;
    //good option when millis() work fine
    //lastConnectionTime += postingInterval;
    Serial.println(lastConnectionTime);
    if (client.connect(emon_ip, 80)) {
      Serial.println();Serial.println();
      Serial.println("connecting....");
      str.reset();
      str.print("GET /emoncms/input/post.json?");
      str.print("node=");
      if(ds.nb())for (byte i=0; i<7; i++){
          str.print(ds[i],HEX);
          if(i<6)str.print("_");
      }
      else str.print("No1wireBus");
      str.print("&json={");
      for(byte i=0;i<ds.nb();i++){
        switch (ds[i*8]){
        case 0x28 :
          str.print("T28_");
          str.print(i);
          str.print(":");
          str.print(ds.get28temperature(i));
          break;
        case 0x26 :
          float celsius2438 = ds.get26temperature(i);
          float vdd = ds.get26voltage(i,"vdd");
          float vad = ds.get26voltage(i,"vad");
          float rh = (vad/vdd - 0.16)/0.0062;
          float truerh = rh/(1.0546-0.00216*celsius2438);
          str.print("T26_");
          str.print(i);
          str.print(":");
          str.print(celsius2438);
          str.print(",RH26_");
          str.print(i);
          str.print(":");
          str.print(truerh);
        }
        str.print(",");
      }
      str.print("_millis:");
      str.print(millis());
      str.print("}&apikey=");
      str.print(apikey);
      Serial.print("HTTP REQ Length (bytes) : ");Serial.println(str.length());
      Serial.println(str.buffer());
      
      // Make a HTTP request:
      client.print(str.buffer());
      client.println(" HTTP/1.0");
      client.print("Host: ");client.println(emon_ip);
      client.println("User-Agent: arduino-eth");
      client.println("Connection: close");
      client.println();

      //we print full srv answer on serial port
      while (client.connected ())
        {
        if (client.available ()){
           char c = client.read();
          Serial.print(c);
          } 
        }
        
      if (!client.connected()) {
        Serial.println();
        client.stop(); 
        Serial.println("disconnecting.");
        }
      
      } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
      }
    }
  lastConnected = client.connected();
}
