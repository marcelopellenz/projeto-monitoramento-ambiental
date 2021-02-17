#pragma once

#ifndef ARDULINKIT_SENSORS
#define ARDULINKIT_SENSORS

#include <HDC1080JS.h>
#include <Adafruit_BMP280.h>
#include "BH1620.h"

class BoardSensors
{
public:

	void begin() {
		MoisureSensor.begin();
		//PressureSensor.begin();
		LightSensor.begin(PA0, BH1620::BH1620_MEDIUM_GAIN);
	}

	void TestMoisureSensor() {
		Serial.println("------------------HDC1080------------------");

		Serial.print("T=");
		Serial.print(getTemperature());
		Serial.print("C, RH=");
		Serial.print(getRelativeHumidity());
		Serial.println("%");
	}

	void TestPressureSensor() {
		Serial.println("------------------BMP280-------------------");
		Serial.print("T=");
		Serial.print(getPressureTemperature());
		Serial.print("C, P=");
		Serial.print(getPressurePa() / 1000.0);
		Serial.println("KPa");
	}

	void TestLightSensor() {
		Serial.println("------------------BH1620-------------------");
		Serial.print("E=");
		Serial.print((uint16_t)getLightMeasurement());
		Serial.println("Lux");
	}

	float getTemperature() {
		MoisureSensor.readTempHumid();
		return MoisureSensor.getTemp();
		//return 25.0;
	}

	float getRelativeHumidity() {
		MoisureSensor.readTempHumid();
		return  MoisureSensor.getRelativeHumidity();
		//return 60.0;
	}

	float getPressurePa() {
		//return PressureSensor.readPressure();
		return 0;
	}

	float getPressureTemperature() {
		return PressureSensor.readTemperature();
	}

	float getLightMeasurement() {
		return LightSensor.getLux();
	}

	float getAverageTemperature() {
		MoisureSensor.readTempHumid();
		return (MoisureSensor.getTemp() + PressureSensor.readTemperature()) / 2.0;
	}

private:
	HDC1080JS MoisureSensor;
	BMP280 PressureSensor;
	BH1620 LightSensor;
};

BoardSensors Sensors;
#endif
