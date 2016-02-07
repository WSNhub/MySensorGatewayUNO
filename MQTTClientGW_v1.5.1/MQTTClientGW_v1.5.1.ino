/* 				MyMQTT Client Gateway 1.0

 Created by Peter Van Aken <Peter.Van_Aken@skynet.be>
 
 Based on MyMQTT-client with Mysensors lib 1.4 by Norbert Truchsess <norbert.truchsess@t-online.de>
 Based on MyMQTT-broker gateway created by Daniel Wiegert <daniel.wiegert@gmail.com>
 Based on MySensors Ethernet Gateway by Henrik Ekblad <henrik.ekblad@gmail.com>
 http://www.mysensors.org

 Requires MySensors lib 1.5
 
 DON'T FORGET to DISABLE DEBUG and ENABLE SOFTSPI in MyConfig.h

 * Change below; TCP_IP, TCP_PORT, TCP_MAC
 This will listen on your selected TCP_IP:TCP_PORT below, Please change TCP_MAC your liking also.
 *1 -> NOTE: Keep first byte at x2, x6, xA or xE (replace x with any hex value) for using Local Ranges.

 *2 You can use standard pin set-up as MySensors recommends or if you own a IBOARD you may change
 the radio-pins below if you hardware mod your iBoard. see [URL BELOW] for more details.
 http://forum.mysensors.org/topic/224/iboard-cheap-single-board-ethernet-arduino-with-radio/5

 * Don't forget to look at the definitions in libraries\MySensors\MyMQTT.h!

 define TCPDUMP and connect serial interface if you have problems, please write on
 http://forum.mysensors.org/ and explain your problem, include serial output. Don't forget to
 turn on DEBUG in libraries\MySensors\MyConfig.h also.

 MQTT_FIRST_SENSORID is for 'DHCP' server in MyMQTT. You may limit the ID's with FIRST and LAST definition.
 If you want your manually configured below 20 set MQTT_FIRST_SENSORID to 20.
 To disable: set MQTT_FIRST_SENSORID to 255.

 MQTT_BROKER_PREFIX is the leading prefix for your nodes. This can be only one char if like.

 MQTT_SEND_SUBSCRIPTION is if you want the MyMQTT to send a empty payload message to your nodes.
 This can be useful if you want to send latest state back to the MQTT client. Just check if incoming
 message has any length or not.
 Example: if (msg.type==V_LIGHT && strlen(msg.getString())>0) otherwise the code might do strange things.

 * Address-layout is : [MQTT_BROKER_PREFIX]/[NodeID]/[SensorID]/V_[SensorType]
 NodeID and SensorID is uint8 (0-255) number.
 Last segment is translation of the sensor type, look inside MyMQTT.cpp for the definitions.
 User can change this to their needs. We have also left some space for custom types.

 Special: (sensor 255 reserved for special commands)
 You can receive a node sketch name with MyMQTT/20/255/V_Sketch_name (or version with _version)

 To-do:
 Special commands : clear or set EEPROM Values, Send REBOOT and Receive reboot for MyMQTT itself.
 Be able to send ACK so client returns the data being sent.
 ... Please come with ideas!
 What to do with publish messages.

 Test in more MQTT clients, So far tested in openhab and MyMQTT for Android (Not my creation)
 - http://www.openhab.org/
 - https://play.google.com/store/apps/details?id=at.tripwire.mqtt.client&hl=en
 ... Please notify me if you use this broker with other software.


 * How to set-up Openhab and MQTTGateway:
 http://forum.mysensors.org/topic/303/mqtt-broker-gateway

 */
 
#include <DigitalIO.h>
#include <SPI.h>
#include <Wire.h>

#include <MySigningNone.h>
#include <MyTransportNRF24.h>
#include <MyHwATMega328.h>
#include <MySensor.h>

#include <MsTimer2.h>
#include <Time.h>
#include <Ethernet.h>

#include "MyMQTTClient.h"
#include "PubSubClient.h"

#include "MyLCDi2c.h"
#include "ds3231.h"

// ip of MQTT broker (e.g. Mosquitto)
uint8_t remote_ip[] = { 192, 168, 0, 107 };
// port that your server is listening on
#define remote_port 1883
// gateway ip-address. 
uint8_t local_ip[] = {192, 168, 0, 225};
// It's mandatory every device is assigned a unique mac.
uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x07, 0x02 };

MyLCDi2c_I2C  lcd(I2C_ADDR);

EthernetClient ethClient;
MyTransportNRF24 transport(RADIO_CE_PIN, RADIO_SPI_SS_PIN, RF24_PA_LEVEL_GW);  
// Hardware profile 
MyHwATMega328 hw;

//////////////////////////////////////////////////////////////////
void processMQTTMessages(char* topic, byte* payload, unsigned int length);
PubSubClient client(remote_ip, remote_port, processMQTTMessages, ethClient);
MyMQTTClient gw(client);

void setup()
{
  pinMode(ETH_RST_PIN, OUTPUT);
  digitalWrite(ETH_RST_PIN, HIGH);
  
  lcd.begin (20,4,LCD_5x8DOTS);  // initialize the lcd 
  lcd.clear();         
  
  //DS3231_init(DS3231_INTCN);
  
  Ethernet.begin(mac, local_ip);
  delay(1000);   // Wait for Ethernet to get configured.
  gw.begin(RF24_PA_LEVEL_GW, RF24_CHANNEL, RF24_DATARATE);
}

void loop()
{
  if (!client.connected())
  {
    client.connect("MySensor");
    client.subscribe("MyMQTT/#");
  }
  client.loop();
  gw.processRadioMessage();
  gw.display();
}

void processMQTTMessages(char* topic, byte* payload, unsigned int length)
{
  gw.processMQTTMessage(topic, payload, length);
}
