#ifndef BH1620_SENSOR
#define BH1620_SENSOR

class BH1620
{
public:

	void begin(uint8_t pin, uint8_t sensitivityType = BH1620_MEDIUM_GAIN) {
		_sensitivityType = sensitivityType;
		_resistor = 10E3; // Default resistor

		switch (sensitivityType)
		{
		case BH1620_LOW_GAIN:
			_constant = 0.0057e-6;
			break;

		case BH1620_MEDIUM_GAIN:
			_constant = 0.057e-6;
			break;

		case BH1620_HIGH_GAIN:
			_constant = 0.57e-6;
			break;
		}

		pinMode(pin, INPUT_ANALOG);
	}

	float getLux() {
		float voltage = analogRead(PA0);
		voltage = mapfloat(voltage, 0, 4096.0, 0, 3.3);
		return  voltage / (_constant * _resistor);
	}

	enum SENSITIVITY
	{
		BH1620_LOW_GAIN = 0, // 0 - 100000 lux
		BH1620_MEDIUM_GAIN,  // 0 - 10000 lux
		BH1620_HIGH_GAIN     // 0 - 1000 lux
	};

private:
	double _constant;
	uint8_t _sensitivityType;
	uint16_t _resistor; // Current resistor

	float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}
};

#endif
