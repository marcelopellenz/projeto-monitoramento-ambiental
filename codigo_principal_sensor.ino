#include <SPI.h>
#include "Radio.hpp"
#include "Sensors.h"
#include "QueryManager.h"
#include <ArduinoJson.h>

//-------- Message Name ---------
#define Name	'N'
//-------- Message Query ID -----
#define QueryID	'Q'
//-------- Message Data ---------
#define Data	'D'
//-------- Message Options ------
#define Options 'O'
//-------- Data Filtering -------
#define Filter	'F'
#define Filter_Data	'D'
#define Filter_Include	'I'
#define Filter_Exclude	'E'

uint8_t radioId[] = { 0, 0, 0, 1 };

BoardRadioNode<ThroughLora> Radio(radioId, PJON_NOT_ASSIGNED);

String NodeName = "pucpr/bloco8";

uint32_t refreshSensors;
uint16_t RSSI;
float SNR;
volatile uint8_t connected = 0;

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
	/*for (uint8_t i = 0; i < length; i++) {
		Serial.print(payload[i], HEX);
		Serial.write(' ');
	}
	Serial.println();*/
	if (connected) {
		StaticJsonBuffer<1024> jsonBuffer;
		JsonObject &root = jsonBuffer.parseBinaryObject((char *)payload);
		char message[128];
		uint16_t queryID = 0;
		const char* interestName;
		RSSI = abs((int16_t)Radio.strategy.packetRssi());
		SNR = Radio.strategy.packetSnr();

		if (root.success()) {
			uint8_t sendData = 1;
			uint8_t excludeData = 0;
			uint8_t includeData = 0;
			uint8_t order = 0;
			uint8_t registerQuery = 0;

			if (!root.containsKey("Type")) return; // Check if Type is existent in message
			if (!root.containsKey("Name")) return; // Check if Name is existent in message
			if (!root.containsKey("Query")) return; // Check if Name is existent in message

			if ((int)root["Type"] == 0) { // Check if message type is a interest (Type = 0)
				interestName = root["Name"];
				if (NodeName.equals(interestName)) { // Check if message name is addressed to this node
					Serial.println("Received Interest:");

					// 1) Read QueryID number and add it to LRU cache
					if (root.containsKey("Query")) { // Check Query ID of message
						queryID = (int)root["Query"];
						root["Q"] = queryID;
						if (root.containsKey("D")) {
							JsonObject &QueryPeriod = root["D"];
							if (QueryPeriod.containsKey("S")) {
								uint16_t sampleTime = QueryPeriod["S"];

								if (sampleTime > 0) {
									registerQuery = 1;
									if (QueryPeriod.containsKey("L")) {
										uint16_t lifeTime = QueryPeriod["L"];
										QueryManager.addQuery(queryID, sampleTime, lifeTime);
									}
									else
									{
										QueryManager.addQuery(queryID, sampleTime);
									}
								}
								else {
									QueryManager.removeQuery(queryID);
									sendData = 0;
									Serial.print("Canceled Query ");
									Serial.println(queryID);
								}
							}

							root.remove("D");
						}

						root.remove("Query");
					}
					else // Insert blank QueryID if message do not contain QueryID (Single Query request)
					{
						root["Q"] = 0;
					}

					// 2) Read Filter object
					if (root.containsKey("Filter")) // Check if message contain data filtering
					{
						JsonObject &filter = root["Filter"];
						if (filter.containsKey("Data")) sendData = filter["Data"];
						if (filter.containsKey("Order")) order = filter["Order"];

						if (filter.containsKey("Include")) includeData = 1; // Include has priotity over exclude
						else if (filter.containsKey("Exclude")) excludeData = 1;
					}

					// 4) Read Option object
					// TODO
					// 5) Mount Data structure with/without filter

					if (sendData == 1) {
						JsonObject &data = root.createNestedObject("D");

						if (includeData == 1)
						{
							JsonObject &filter = root["Filter"];
							JsonArray &include = filter["Include"];

							for (auto rule : include)
							{
								const char keyChar = rule.as<const char *>()[0];
								Serial.println(keyChar);
								populateJsonData(data, keyChar); // Get first character of rule
								if (registerQuery) QueryManager.addQueryRule(queryID, keyChar);
							}
						}
						else if (excludeData == 1)
						{
							JsonObject &filter = root["Filter"];
							JsonArray &exclude = filter["Exclude"];

							populateJsonData(data, NULL); // Add all sensor values first

							for (auto rule : exclude) // Loop through all rules, removing sensor values
							{
								const char *keyChar = rule; // Get sensor rule name

								if (data.containsKey(keyChar))
									data.remove(keyChar);
							}
							if (registerQuery) {
								for (auto rule : data) // Loop through all rules, removing sensor values
								{
									const char *keyChar = rule.key; // Get sensor rule name

									QueryManager.addQueryRule(queryID, keyChar[0]);
								}
							}
						}
						else
						{
							populateJsonData(data, NULL); // Add all sensor values

							if (registerQuery) {
								for (auto rule : data) // Loop through all rules, removing sensor values
								{
									const char *keyChar = rule.key; // Get sensor rule name

									QueryManager.addQueryRule(queryID, keyChar[0]);
								}
							}
						}

						// 5) Close Packet and send message with data structure
						if (!registerQuery) {
							if (root.containsKey("Filter")) root.remove("Filter");
							if (root.containsKey("Opt")) root.remove("Opt");

							root.remove("Type");
							root["T"] = 1; // Change message type to response (Type = 1)
							root.remove("Name");

							int size = root.binaryPrintTo(message, 128); // Serialize data to MessagePack

							delay(Radio.device_id() * 200);
							Radio.send(PJON_MASTER_ID, message, size);  // Send packet in broadcast mode
							//unsigned int response = Radio.send_packet(PJON_MASTER_ID, message, size);
							Serial.println("Bytes sent: " + String(size));

							// Blink to show status
							digitalWrite(LED_BUILTIN, LOW);
							delay(30);
							digitalWrite(LED_BUILTIN, HIGH);
						}
					}
				}
			}
		}
	}
}

//Serial.write(payload, length);

inline void populateJsonData(JsonObject &data, char sensorChar) {
	switch (sensorChar)
	{
	case NULL: // Add all sensor if NULL is received
	case 'T': data["T"] = Sensors.getTemperature();
		if (sensorChar)
			break;
	case 'H': data["H"] = Sensors.getRelativeHumidity();
		if (sensorChar)
			break;
	case 'P': data["P"] = (int)(Sensors.getPressurePa() / 1000);
		if (sensorChar)
			break;
	case 'L': data["L"] = (int)Sensors.getLightMeasurement();
		if (sensorChar)
			break;
	case 'R': data["R"] = RSSI;
		if (sensorChar)
			break;
	}
}

void error_handler(uint8_t code, uint16_t data, void * ptr) {
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(30);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(30);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(30);
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	Serial.begin(115200);  // start serial for output
	delay(2000);
	Sensors.begin();
	// TODO: Error handler code
	Radio.set_error(error_handler);
	Radio.begin();

	Radio.set_receiver(receiver_function);
	Radio.set_synchronous_acknowledge(true); // Enable Unicast ACK

	QueryManager.begin(Radio);

	Serial.println("Node ready");
	delay(1000);
}

void loop() {
	static uint8_t acquired = Radio.device_id();
	static uint32_t disconnectedTimeout = 0;

	/*if (millis() - refreshSensors >= 5000) {
		refreshSensors = millis();

		Sensors.TestMoisureSensor();
		Sensors.TestPressureSensor();
		Sensors.TestLightSensor();

		Serial.println("------------------LoRa---------------------");
		Serial.print("RSSI=");
		Serial.print(RSSI);
		Serial.print(", SNR=");
		Serial.println(SNR);
	}*/

	Radio.receive();
	Radio.update();
	QueryManager.update();

	/*static uint32 t;
	if (millis() - t > 1000) {
		t = millis();
		Serial.println(Radio.device_id());
	}*/

	if (!connected) {
		if (millis() - disconnectedTimeout > 5000) {
			Serial.println("Retrying connection...");
			delay(PJON_RANDOM(10000));
			Radio.acquire_id_master_slave();
			disconnectedTimeout = millis();
		}
	}

	if ((Radio.device_id() != acquired && Radio.device_id() != 255)) {
		acquired = Radio.device_id();
		connected = 1;
		Serial.print("Acquired device id: ");
		Serial.println(Radio.device_id());
	}
}
