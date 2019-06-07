

//JUNE 2019
//HARDWARE IPV4 WITH WIZNET SHIELD
//a simple MQTT publication to an emonCMS server
//uses Nicky O'Leary arduino MQTT library https://github.com/knolleary/pubsubclient 
//API doc on https://pubsubclient.knolleary.net/api.html
//additional resources on MQTT
//https://www.arduinolab.net/mqtt-node-with-arduino-uno-and-w5100-ethernet-shield/


#include <Arduino.h>
#include <dynaBus.h>

const uint8_t ONE_WIRE_PIN = 2;
dynaBus ds(ONE_WIRE_PIN);

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10000L;

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
    char buf[50];
};
PacketBuffer basetopic;
PacketBuffer topic;

//arduino parameters : mac adress and IP
//*********ip not needed when working in dhcp*****************
byte mac[6];
IPAddress ip(192,168,1,149);


//***********************SERVER PARAMETERS********************
IPAddress emon_ip(192,168,1,2);

EthernetClient ethclient;
PubSubClient client(ethclient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // user/pass values by default
    if (client.connect("arduinoClient","emonpi","emonpimqtt2016")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

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
    //for( uint8_t i = 1; i < 7; i++) mac[i]=ds[i+1];
  }
  client.setServer(emon_ip, 1883);
  //on peut fonctionner en dhcp
  Ethernet.begin(mac);
  Serial.print("client is at ");
  Serial.println(Ethernet.localIP());
  // give the ethernet module time to boot up:
  delay(1000);
  
}

//output the topic
PacketBuffer prepareTopic(PacketBuffer base, byte i, char* s){
   PacketBuffer str;
   str.reset();
   str.print(base.buffer());
   str.print("/");
   str.print(String(i));
   str.print(s);
   return str;
}

void loop()
{
  
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    reconnect();
  }
  
  unsigned long now = millis();
  
  // if you get a connection, report back via serial:
  if (client.connected() && (now-lastConnectionTime > postingInterval))
    {
      lastConnectionTime = now;
      //good option when millis() work fine
      //lastConnectionTime += postingInterval;
      Serial.println(lastConnectionTime);
      Serial.println();Serial.println();
      char tmpBuffer[20];

      //just create the base topic with the node name (ROM number of the sensor 0
      if(ds.nb())
        {
          basetopic.reset();
          basetopic.print("emon/");
          ds.ROMtochar(0,"",basetopic);
        }
        else 
        {
          basetopic.print("emon/No1wireBus");
        }
      Serial.print("we are publishing on ");
      Serial.println(basetopic.buffer());
      
      for(byte i=0;i<ds.nb();i++){
        switch (ds[i*8]){
        case 0x26 :
          {
          float celsius2438 = ds.get26temperature(i);
          float vdd = ds.get26voltage(i,"vdd");
          float vad = ds.get26voltage(i,"vad");
          float rh = (vad/vdd - 0.16)/0.0062;
          float truerh = rh/(1.0546-0.00216*celsius2438);
          //Serial.println(celsius2438);
          //Serial.println(truerh);
          topic.reset();
          topic=prepareTopic(basetopic,i,"T26");
          
          client.publish(topic.buffer(),dtostrf(celsius2438,6,2,tmpBuffer));
          topic.reset();
          topic=prepareTopic(basetopic,i,"RH26");
          client.publish(topic.buffer(),dtostrf(truerh,6,2,tmpBuffer));
          }
          break;
        case 0x28 :
          {
          float celsius1820=ds.get28temperature(i);
          topic.reset();
          topic=prepareTopic(basetopic,i,"T28");
          client.publish(topic.buffer(),dtostrf(celsius1820,6,2,tmpBuffer));
          }
          break;
        }
      }
      topic.reset();
      topic=prepareTopic(basetopic,0,"millis");
      client.publish(topic.buffer(),dtostrf(millis(),10,2,tmpBuffer));
    }

    client.loop();
}
