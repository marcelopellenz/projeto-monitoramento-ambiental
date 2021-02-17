#pragma once

#ifndef ARDULINKIT_RADIO

#define PJON_INCLUDE_TL
#define A0 PA0
#include <PJON.h>
#include <PJONSlave.h>
#include <PJONMaster.h>

template <typename Strategy>
class BoardRadioRoot
{
public:

private:
};

template <typename Strategy>
class BoardRadioNode : public PJONSlave<Strategy>
{
public:
	BoardRadioNode(uint8_t radioId[], uint8_t PjonId = PJON_NOT_ASSIGNED) : PJONSlave<Strategy>(radioId, PjonId)
	{
	}

	inline void begin() {
		afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // release PB3 and PB5
		afio_remap(AFIO_REMAP_SPI1); // remap SPI1

		gpio_set_mode(&gpiob, 3, GPIO_AF_OUTPUT_PP);
		gpio_set_mode(&gpiob, 4, GPIO_INPUT_FLOATING);
		gpio_set_mode(&gpiob, 5, GPIO_AF_OUTPUT_PP);

		SPI.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
		SPI.setClockDivider(SPI_BAUD_PCLK_DIV_16);
		PJONSlave<Strategy>::strategy.setPins(PA15, PB9, PB8);
		pinMode(PA15, OUTPUT);
		pinMode(PB9, OUTPUT);
		digitalWrite(PB9, LOW);
		pinMode(PB8, INPUT);
		PJONSlave<Strategy>::strategy.setFrequency(915E6);
		PJONSlave<Strategy>::strategy.setSignalBandwidth(250E3);

		PJONSlave<Strategy>::begin();
		PJONSlave<Strategy>::strategy.setSpreadingFactor(8);
		PJONSlave<Strategy>::acquire_id_master_slave();
		digitalWrite(PA15, LOW);
		Serial.println("Conf");
	}
};

template <typename Strategy>
class BoardRadio : public PJON<Strategy>
{
public:
	BoardRadio(uint8_t radioId[], uint8_t PjonId = PJON_NOT_ASSIGNED) : PJON<Strategy>(radioId, PjonId)
	{
	}

	BoardRadio(uint8_t PjonId = PJON_NOT_ASSIGNED) : PJON<Strategy>(PjonId)
	{
	}

	BoardRadio() : PJON<Strategy>()
	{
	}

	inline void begin() {
		afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // release PB3 and PB5
		afio_remap(AFIO_REMAP_SPI1); // remap SPI1

		gpio_set_mode(&gpiob, 3, GPIO_AF_OUTPUT_PP);
		gpio_set_mode(&gpiob, 4, GPIO_INPUT_FLOATING);
		gpio_set_mode(&gpiob, 5, GPIO_AF_OUTPUT_PP);

		SPI.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
		SPI.setClockDivider(SPI_BAUD_PCLK_DIV_16);
		PJONSlave<Strategy>::strategy.setPins(PA15, PB9, PB8);
		pinMode(PA15, OUTPUT);
		pinMode(PB9, OUTPUT);
		digitalWrite(PB9, LOW);
		pinMode(PB8, INPUT);
		PJONSlave<Strategy>::strategy.setFrequency(915E6);
		PJONSlave<Strategy>::strategy.setSignalBandwidth(250E3);

		PJONSlave<Strategy>::begin();
		PJONSlave<Strategy>::strategy.setSpreadingFactor(8);
		digitalWrite(PA15, LOW);
	}
};

#endif // !ARDULINKIT_RADIO