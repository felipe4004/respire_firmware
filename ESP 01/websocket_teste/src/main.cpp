#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <WebSocketsClient.h>

#include <Hash.h>

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected");
		}
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
      case WStype_PING:
          // pong will be send automatically
          Serial.printf("[WSc] get ping\n");
          break;
      case WStype_PONG:
          // answer to a ping we send
          Serial.printf("[WSc] get pong\n");
          break;
      default:
          Serial.print("Nenhuma das opcoes");
    }

}

void setup() {
	// Serial.begin(921600);
	Serial.begin(115200);

	//Serial.setDebugOutput(true);
	Serial.setDebugOutput(true);

	Serial.println();
	Serial.println();
	Serial.println();

	for(uint8_t t = 4; t > 0; t--) {
		Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
		Serial.flush();
		delay(1000);
	}

	WiFi.begin("Bifrost", "089083jkb139293709b1393401023fe");

	//WiFi.disconnect();
	while(WiFi.status() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("177.72.63.147", 80, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	//webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

}

void loop() {
	webSocket.loop();
}