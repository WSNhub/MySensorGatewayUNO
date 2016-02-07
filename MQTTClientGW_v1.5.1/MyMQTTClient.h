/*
The MySensors library adds a new layer on top of the RF24 library.
It handles radio network routing, relaying and ids.

Created by Henrik Ekblad <henrik.ekblad@gmail.com>
Modified by Daniel Wiegert
Modified by Norbert Truchsess <norbert.truchsess@t-online.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
*/

#ifndef MyMQTTClient_h
#define MyMQTTClient_h

#include "MySensor.h"
#include "PubSubClient.h"

#define MQTT_FIRST_SENSORID	255  		// If you want manually configured nodes below this value. 255 = Disable
#define MQTT_LAST_SENSORID	254 		// 254 is max! 255 reserved.
#define MQTT_PREFIX	"MyMQTT"	        // First prefix in MQTT tree, keep short!
#define MQTT_SEND_SUBSCRIPTION 1		// Send empty payload (request) to node upon MQTT client subscribe request.
// NOTE above : Beware to check if there is any length on payload in your incommingMessage code:
// Example: if (msg.type==V_LIGHT && strlen(msg.getString())>0) otherwise the code might do strange things.
#define MQTT_UNIT		"M"		// Select M for metric or I for imperial.
#define MQTT_TRANSLATE_TYPES			// V_TYPE in address, Comment if you want all numbers (MyMQTT/01/01/01)
 
#define RADIO_CE_PIN        5		// radio chip enable
#define RADIO_SPI_SS_PIN    4		// radio SPI serial select
#define ETH_RST_PIN         6           // ethernet module reset
#define BUZZER_PIN          3           // buzzer : TODO !!!

#define BUFF_MAX            20
#define I2C_ADDR            0x27  // Define I2C Address for the PCF8574A 

//////////////////////////////////////////////////////////////////

#define EEPROM_LATEST_NODE_ADDRESS ((uint8_t)EEPROM_LOCAL_CONFIG_ADDRESS)
#define MQTT_MAX_PACKET_SIZE 100

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1  // Client request to connect to Server
#define MQTTCONNACK     2  // Connect Acknowledgment
#define MQTTPUBLISH     3  // Publish message
#define MQTTPUBACK      4  // Publish Acknowledgment
#define MQTTPUBREC      5  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8  // Client Subscribe request
#define MQTTSUBACK      9  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 // Client Unsubscribe request
#define MQTTUNSUBACK    11 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 // PING Request
#define MQTTPINGRESP    13 // PING Response
#define MQTTDISCONNECT  14 // Client is Disconnecting
#define MQTTReserved    15 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)


class MyMQTTClient :
public MySensor {

public:
	MyMQTTClient(PubSubClient client); // constructor
	void begin(rf24_pa_dbm_e paLevel=RF24_PA_LEVEL_GW, uint8_t channel=RF24_CHANNEL, rf24_datarate_e dataRate=RF24_DATARATE);
	void processRadioMessage();
	void processMQTTMessage(char* topic, byte* payload, unsigned int length);
        void display();
        
private:
	void SendMQTT(MyMessage &msg);
	PubSubClient client;
	char buffer[MQTT_MAX_PACKET_SIZE];
	char convBuf[MAX_PAYLOAD*2+1];
	uint8_t buffsize;
	char *getType(char *b, const char **index);
        char line1[BUFF_MAX],line2[BUFF_MAX],line3[BUFF_MAX],line4[BUFF_MAX];
};

#endif
