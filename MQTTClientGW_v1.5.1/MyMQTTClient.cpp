/*
 The MySensors library adds a new layer on top of the RF24 library.
 It handles radio network routing, relaying and ids.

 Created by Norbert Truchsess <norbert.truchsess@t-online.de>
 Based on MyMQTT.cpp MySensors MQTT-gateway (broker) by Henrik Ekblad <henrik.ekblad@gmail.com>
 Modified by Daniel Wiegert
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "PubSubClient.h"
#include "MyMQTTClient.h"
#include <Time.h>

#include "MyLCDi2c.h"

#ifdef MQTT_TRANSLATE_TYPES
char VAR_0[] PROGMEM = "TEMP";		//V_TEMP
char VAR_1[] PROGMEM = "HUM";		//V_HUM
char VAR_2[] PROGMEM = "LIGHT";		//V_LIGHT
char VAR_3[] PROGMEM = "DIMMER";		//V_DIMMER
char VAR_4[] PROGMEM = "PRESSURE";	//V_PRESSURE
char VAR_5[] PROGMEM = "FORECAST";	//V_FORECAST
char VAR_6[] PROGMEM = "RAIN";		//V_RAIN
char VAR_7[] PROGMEM = "RAINRATE";	//V_RAINRATE
char VAR_8[] PROGMEM = "WIND";		//V_WIND
char VAR_9[] PROGMEM = "GUST";		//V_GUST
char VAR_10[] PROGMEM = "DIRECTON";	//V_DIRECTON
char VAR_11[] PROGMEM = "UV";		//V_UV
char VAR_12[] PROGMEM = "WEIGHT";		//V_WEIGHT
char VAR_13[] PROGMEM = "DISTANCE";	//V_DISTANCE
char VAR_14[] PROGMEM = "IMPEDANCE";	//V_IMPEDANCE
char VAR_15[] PROGMEM = "ARMED";		//V_ARMED
char VAR_16[] PROGMEM = "TRIPPED";	//V_TRIPPED
char VAR_17[] PROGMEM = "WATT";		//V_WATT
char VAR_18[] PROGMEM = "KWH";		//V_KWH
char VAR_19[] PROGMEM = "SCENE_ON";	//V_SCENE_ON
char VAR_20[] PROGMEM = "SCENE_OFF";	//V_SCENE_OFF
char VAR_21[] PROGMEM = "HEATER";		//V_HEATER
char VAR_22[] PROGMEM = "HEATER_SW";	//V_HEATER_SW
char VAR_23[] PROGMEM = "LIGHT_LEVEL";	//V_LIGHT_LEVEL
char VAR_24[] PROGMEM = "VAR1";		//V_VAR1
char VAR_25[] PROGMEM = "VAR2";		//V_VAR2
char VAR_26[] PROGMEM = "VAR3";		//V_VAR3
char VAR_27[] PROGMEM = "VAR4";		//V_VAR4
char VAR_28[] PROGMEM = "VAR5";		//V_VAR5
char VAR_29[] PROGMEM = "UP";		//V_UP
char VAR_30[] PROGMEM = "DOWN";		//V_DOWN
char VAR_31[] PROGMEM = "STOP";		//V_STOP
char VAR_32[] PROGMEM = "IR_SEND";	//V_IR_SEND
char VAR_33[] PROGMEM = "IR_RECEIVE";	//V_IR_RECEIVE
char VAR_34[] PROGMEM = "FLOW";		//V_FLOW
char VAR_35[] PROGMEM = "VOLUME";		//V_VOLUME
char VAR_36[] PROGMEM = "LOCK_STATUS";	//V_LOCK_STATUS
char VAR_37[] PROGMEM = "DUST_LEVEL";	//V_DUST_LEVEL
char VAR_38[] PROGMEM = "VOLTAGE";	//V_VOLTAGE
char VAR_39[] PROGMEM = "CURRENT";	//V_CURRENT
char VAR_40[] PROGMEM = "";		//
char VAR_41[] PROGMEM = "";		//
char VAR_42[] PROGMEM = "";		//
char VAR_43[] PROGMEM = "";		//
char VAR_44[] PROGMEM = "";		//
char VAR_45[] PROGMEM = "";		//
char VAR_46[] PROGMEM = "";		//
char VAR_47[] PROGMEM = "";		//
char VAR_48[] PROGMEM = "";		//
char VAR_49[] PROGMEM = "";		//
char VAR_50[] PROGMEM = "";		//
char VAR_51[] PROGMEM = "";		//
char VAR_52[] PROGMEM = "";		//
char VAR_53[] PROGMEM = "";		//
char VAR_54[] PROGMEM = "";		//
char VAR_55[] PROGMEM = "";		//
char VAR_56[] PROGMEM = "";		//
char VAR_57[] PROGMEM = "";		//
char VAR_58[] PROGMEM = "";		//
char VAR_59[] PROGMEM = "";		//
char VAR_60[] PROGMEM = "Started!\n";	//Custom for MQTTGateway
char VAR_61[] PROGMEM = "SKETCH_NAME";	//Custom for MQTTGateway
char VAR_62[] PROGMEM = "SKETCH_VERSION"; //Custom for MQTTGateway
char VAR_63[] PROGMEM = "UNKNOWN"; 	//Custom for MQTTGateway

//////////////////////////////////////////////////////////////////

PROGMEM const char *VAR_Type[] =
{ VAR_0, VAR_1, VAR_2, VAR_3, VAR_4, VAR_5, VAR_6, VAR_7, VAR_8, VAR_9, VAR_10, VAR_11, VAR_12, VAR_13,
  VAR_14, VAR_15, VAR_16, VAR_17, VAR_18, VAR_19, VAR_20, VAR_21, VAR_22, VAR_23, VAR_24, VAR_25,
  VAR_26, VAR_27, VAR_28, VAR_29, VAR_30, VAR_31, VAR_32, VAR_33, VAR_34, VAR_35, VAR_36, VAR_37,
  VAR_38, VAR_39, VAR_40, VAR_41, VAR_42, VAR_43, VAR_44, VAR_45, VAR_46, VAR_47, VAR_48, VAR_49,
  VAR_50, VAR_51, VAR_52, VAR_53, VAR_54, VAR_55, VAR_56, VAR_57, VAR_58, VAR_59, VAR_60, VAR_61,
  VAR_62, VAR_63
};

#define S_FIRSTCUSTOM 60
#define VAR_TOTAL (sizeof(VAR_Type)/sizeof(char *))-1
#endif

char mqtt_prefix[] PROGMEM = MQTT_PREFIX;
#define TYPEMAXLEN 20
#define VAR_UNKNOWN 63

extern MyLCDi2c_I2C  lcd;
extern MyTransportNRF24 transport;  
extern MyHwATMega328 hw;
static uint16_t display_cnt = 0;

MyMQTTClient::MyMQTTClient(PubSubClient inClient) : MySensor(transport, hw), buffsize(0)
{
  client = inClient;
}

inline MyMessage& 
build(MyMessage &msg, uint8_t sender, uint8_t destination, uint8_t sensor,
      uint8_t command, uint8_t type, bool enableAck)
{
  msg.destination = destination;
  msg.sender = sender;
  msg.sensor = sensor;
  msg.type = type;
  mSetCommand(msg, command);
  mSetRequestAck(msg, enableAck);
  mSetAck(msg, false);
  return msg;
}

char *
MyMQTTClient::getType(char *b, const char **index)
{
  char *q = b;
  char *p = (char *) pgm_read_word(index);
  while (*q++ = pgm_read_byte(p++))
    ;
  *q = 0;
  return b;
}

void
MyMQTTClient::begin(rf24_pa_dbm_e paLevel, uint8_t channel,
                    rf24_datarate_e dataRate)
{
  MySensor::begin(NULL, 0, true, 0);  
}

void
MyMQTTClient::processRadioMessage()
{
  if (process())
  {
    // A new message was received from one of the sensors
    MyMessage message = getLastMessage();
    // Pass along the message from sensors to serial line
    //rxBlink(1);
    SendMQTT(message);
  }

}

void
MyMQTTClient::processMQTTMessage(char* topic, byte* payload,
                                 unsigned int length)
{
  char *str, *p;
  uint8_t i = 0;
  buffer[0] = 0;
  buffsize = 0;
  //struct ts t;
 
  for (str = strtok_r(topic, "/", &p); str && i < 4;
       str = strtok_r(NULL, "/", &p))
  {
    switch (i)
    {
      case 0:
        {
          if (strcmp_P(str, mqtt_prefix) != 0)
          { //look for MQTT_PREFIX
            return;			//Message not for us or malformatted!
          }
          break;
        }
      case 1:
        {
          msg.destination = atoi(str);	//NodeID
          break;
        }
      case 2:
        {
          msg.sensor = atoi(str);		//SensorID
          break;
        }
      case 3:
        {
          char match = 0;			//SensorType
          //strcpy(str,(char*)&str[2]);  //Strip VAR_

          for (uint8_t j = 0;
               strcpy_P(convBuf, (char*) pgm_read_word(&(VAR_Type[j]))); j++)
          {
            if (strcmp((char*) &str[2], convBuf) == 0)
            { //Strip VAR_ and compare
              match = j;
              snprintf(line2, BUFF_MAX, "N:%03d S:%03d %s ", msg.destination, msg.sensor, convBuf);   
              break;
            }
            if (j >= VAR_TOTAL)
            { // No match found!
              match = VAR_UNKNOWN;	// last item.
              break;
            }
          }
          msg.type = match;
          break;
        }
    }
    i++;
  }						//Check if package has payload

   
  char* ca;
  ca = (char *)payload;
  ca += length;
  *ca = '\0';

  msg.set((const char*)payload);			//Payload
    
  //txBlink(1);
  // inject time from gateway
  if ((msg.destination == 0) && (msg.sensor == 199)) {
    unsigned long epoch = atol((char*)payload);
    if (epoch > 10000) {
#ifdef DSRTC
      //RTC.set(epoch); // this sets the RTC to the time from controller - which we do want periodically
#endif
      setTime(epoch);

    }
    // display current time
    // snprintf(line1+0, BUFF_MAX-0, "%02d:%02d:%02d", t.hour, t.min, t.sec);    
  }
  // send message from gateway to sensors through RF
  if (!sendRoute(
        build(msg, GATEWAY_ADDRESS, msg.destination, msg.sensor, C_SET, msg.type,
              0)));
    //errBlink(1);

}


// received from sensor network and transmit to controller
void
MyMQTTClient::SendMQTT(MyMessage &msg)
{

  buffsize = 0;
  if (!client.connected())
    return;			//We have no connections - return
  if (msg.isAck())
  {
    if (msg.sender == 255 && mGetCommand(msg) == C_INTERNAL
        && msg.type == I_ID_REQUEST)
    {
      // TODO: sending ACK request on id_response fucks node up. doesn't work.
      // The idea was to confirm id and save to EEPROM_LATEST_NODE_ADDRESS.
    }
  }
  else
  {
    // we have to check every message if its a newly assigned id or not.
    // Ack on I_ID_RESPONSE does not work, and checking on C_PRESENTATION isn't reliable.
    uint8_t newNodeID = loadState(EEPROM_LATEST_NODE_ADDRESS) + 1;
    if (newNodeID <= MQTT_FIRST_SENSORID) newNodeID = MQTT_FIRST_SENSORID;
    if (msg.sender == newNodeID)
    {
      saveState(EEPROM_LATEST_NODE_ADDRESS, newNodeID);
    }
    if (mGetCommand(msg) == C_INTERNAL)
    {
      if (msg.type == I_CONFIG)
      {
        //txBlink(1);
        if (!sendRoute(
              build(msg, GATEWAY_ADDRESS, msg.sender, 255, C_INTERNAL,
                    I_CONFIG, 0).set(MQTT_UNIT)));
          //errBlink(1);
      }
      else if (msg.type == I_TIME)
      {
        //txBlink(1);
        if (!sendRoute(
              build(msg, GATEWAY_ADDRESS, msg.sender, 255, C_INTERNAL,
                    I_TIME, 0).set(now())));
          //errBlink(1);
      }
      else if (msg.type == I_ID_REQUEST && msg.sender == 255)
      {
        uint8_t newNodeID = loadState(EEPROM_LATEST_NODE_ADDRESS) + 1;
        if (newNodeID <= MQTT_FIRST_SENSORID)
          newNodeID = MQTT_FIRST_SENSORID;
        if (newNodeID >= MQTT_LAST_SENSORID)
          return; // Sorry no more id's left :(
        //txBlink(1);
        if (!sendRoute(
              build(msg, GATEWAY_ADDRESS, msg.sender, 255, C_INTERNAL,
                    I_ID_RESPONSE, 0).set(newNodeID)));
          //errBlink(1);
      }
      else if (msg.type == I_BATTERY_LEVEL)
      {
        strcpy_P(buffer, mqtt_prefix);
        buffsize += strlen_P(mqtt_prefix);
        buffsize += sprintf(&buffer[buffsize], "/%i/255/BATTERY_LEVEL\0", msg.sender );
        msg.getString(convBuf);     
        //snprintf(line3, BUFF_MAX, "%s", convBuf);   
        client.publish(buffer, convBuf);
      }
      else if (msg.type == I_SKETCH_NAME)
      {
        strcpy_P(buffer, mqtt_prefix);
        buffsize += strlen_P(mqtt_prefix);
        buffsize += sprintf(&buffer[buffsize], "/%i/255/SKETCH_NAME\0", msg.sender );
        msg.getString(convBuf);
        client.publish(buffer, convBuf);
      }
      else if (msg.type == I_SKETCH_VERSION)
      {
        strcpy_P(buffer, mqtt_prefix);
        buffsize += strlen_P(mqtt_prefix);
        buffsize += sprintf(&buffer[buffsize], "/%i/255/SKETCH_VERSION\0", msg.sender );
        msg.getString(convBuf);
        client.publish(buffer, convBuf);
      }

    }
    else if (mGetCommand(msg) != 0)
    {
      if (mGetCommand(msg) == MQTTPUBLISH)
        msg.type = msg.type + (S_FIRSTCUSTOM - 10);	//Special message

      if (msg.type > VAR_TOTAL)
        msg.type = VAR_UNKNOWN;		// If type > defined types set to unknown.
        
      strcpy_P(buffer, mqtt_prefix);
      buffsize += strlen_P(mqtt_prefix);
      buffsize += sprintf(&buffer[buffsize], "/%i/%i/V_%s\0", msg.sender,
                          msg.sensor, getType(convBuf, &VAR_Type[msg.type]));
      msg.getString(convBuf);
      snprintf(line3, BUFF_MAX, "%s   ", convBuf);   
      client.publish(buffer, convBuf);
    }
  }
}


void
MyMQTTClient::display()
{
  if (display_cnt == 1000) {
    lcd.setCursor(0,0);
    lcd.print(line1);
    lcd.setCursor(0,1);
    lcd.print(line2);
    lcd.setCursor(0,2);
    lcd.print(line3);
    lcd.setCursor(0,3);
    lcd.print(line4);  
    display_cnt = 0;
  } else
  display_cnt++;
}
  
