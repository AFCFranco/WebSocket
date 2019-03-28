/*
* WebSocketClient.ino
*
*  Created on: 24.05.2015
*
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
//ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

String mac;
String start = "ITS";
String stop = "DTS";
bool band = true;

#define USE_SERIAL Serial
unsigned int cont = 0;
String sendData;
byte sendData2[4984];
unsigned long tiempo = 0;
unsigned long tiempo2 = 0;
bool bandSend = false;
unsigned volatile long retardo = 1000;
unsigned volatile long retardo2 = 1500;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	unsigned int dataRecibido;
	String str2;
	
	switch (type) {
	case WStype_DISCONNECTED:
		USE_SERIAL.printf("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
	{
		USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

		// send message to server when Connected
		//webSocket.sendTXT("Connected...");
		band = false;
	}
	break;
	case WStype_TEXT:
		str2 = (char*)payload;
		USE_SERIAL.println(str2);
		//dataRecibido = str2.toInt();
		if (str2 == stop) {
			bandSend = false;
		}
		else if (str2 == start) {
			bandSend = true;
			
			//retardo = dataRecibido % 100;
			//USE_SERIAL.print("retardo");
			//USE_SERIAL.println(retardo);

		}

		// send message to server
		// webSocket.sendTXT("message here");
		break;
	case WStype_BIN:
		USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
		hexdump(payload, length);

		// send data to server
		// webSocket.sendBIN(payload, length);
		break;
	}

}

void setup() {
	// USE_SERIAL.begin(921600);
	USE_SERIAL.begin(115200);
	//Serial.setDebugOutput(true);
	
	USE_SERIAL.setDebugOutput(true);
	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();
	for (uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}
	WiFi.begin("software", "12345678");
	while (WiFi.status() != WL_CONNECTED) {
		delay(100);
	}
	//conextion setting
	String rout = "/ws/monitoring/sabanas/";
	mac = WiFi.macAddress();
	mac.remove(0, 9);
	mac.remove(2, 1);
	mac.remove(4, 1);
	mac.getBytes(sendData2, 7);
	rout = rout + mac + "/";
	webSocket.begin("10.42.0.1", 8000, rout);

	//event handler
	webSocket.onEvent(webSocketEvent);
	tiempo = millis();
}

void loop() {
	cont = 0;
	webSocket.loop();
	if ((millis() - tiempo) > (retardo) && bandSend == true) {
		while (Serial.available())Serial.read();
		USE_SERIAL.print("*");
		tiempo2 = millis();
		while (true) {
			if (USE_SERIAL.available()) {
				sendData2[cont] = USE_SERIAL.read();
				//USE_SERIAL.print(sendData2[cont]);
				if (sendData2[cont] == 255) {
					Serial.println("transmision exitosa");
					break;
				}
				cont++;
			}
			if ((millis() - tiempo2) > (retardo2)) {
				Serial.println("uC no responde");
				break;
			}
		}
		tiempo = millis();
		//mac.getBytes(sendData2, 7);
		Serial.println(cont);
		webSocket.sendBIN(sendData2, cont);
		sendData = "";
	}
}