#define UART_RX 0
#define UART_TX 2
#define DHT_PORT 1

#define TEXT_PROTOCOL

#include <SoftwareSerial.h>
#include "dht.h"
#include <stdlib.h>

SoftwareSerial Serial(UART_RX, UART_TX);
dht_driver dht(DHT_PORT);
uint64_t lastUpdateTime;

const uint8_t CMD_NONE				= 'N';
const uint8_t CMD_IDENTIFY			= 'I';
const uint8_t CMD_UPDATE			= 'U';
const uint8_t CMD_GET_TEMPERATURE	= 'T';
const uint8_t CMD_GET_HUMIDITY		= 'H';

uint8_t cmd_read() 
{
	while(Serial.available() < 1) { }
	// 1 byte - command code
	uint8_t command = Serial.read();
	return command;
}

const uint8_t RESP_NONE		= 'N';
const uint8_t RESP_OK		= 'O';
const uint8_t RESP_IDENTITY		= 'I';
const uint8_t RESP_WRONG_COMMAND		= 'W';
const uint8_t RESP_DEVICE_ERROR		= 'E';
const uint8_t RESP_UP_TO_DATE		= 'D';
const uint8_t RESP_TEMPERATURE		= 'T';
const uint8_t RESP_HUMIDITY		= 'H';

void resp_write(uint8_t value)
{
	Serial.write(value);
}

void resp_end()
{
	Serial.print('\n');
}

void resp_write(const __FlashStringHelper* text)
{
	Serial.print(text);
}

#ifdef TEXT_PROTOCOL
	uint16_t power_of_10(uint8_t p)
{
	uint16_t r = 1;
	for(uint8_t i = 0; i < p; ++i)
	{
		r *= 10;
	}
	return r;
}

char get_digit(uint16_t value, uint8_t index)
{
	uint8_t d = static_cast<uint8_t>(
		value % power_of_10(index + 1)
		/  power_of_10(index)
		);

	return d + '0';
}
#endif

void resp_write(float value)
{
#ifdef TEXT_PROTOCOL
	uint16_t int_value = static_cast<uint16_t>(abs( value * 10.0));
	
	Serial.print(value >= 0 ? '+' : '-');
	Serial.print(get_digit(int_value, 3));
	Serial.print(get_digit(int_value, 2));
	Serial.print(get_digit(int_value, 1));
	Serial.print('.');
	Serial.print(get_digit(int_value, 0));
#else
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t x = p[i];
		Serial.write(x);
	}
#endif
}

const uint8_t FAST =  50;

void blink(uint8_t times, uint8_t period)
{
	pinMode(DHT_PORT, OUTPUT);

	for(uint8_t i = 0; i < times; ++i)
	{
		digitalWrite(DHT_PORT, LOW);
		delay(period/2);
		digitalWrite(DHT_PORT, HIGH);
		delay(period/2);
	}  
}


void handle_command(uint8_t command)
{
	uint64_t time;

	switch (command)
	{
	case CMD_NONE:
		// Empty command (may be used as PING)
		resp_write(RESP_NONE);
		break;

	case CMD_IDENTIFY:
		// Identify device command
		// 1 byte response code
		// n bytes null-terminated string containing device name
		resp_write(RESP_IDENTITY);
		resp_write(F("SENSOR/EXT_DHT"));
		resp_end();
		break;

	case CMD_UPDATE:
		// Update data command
		time = millis();
		if(lastUpdateTime - time < 2000)
		{
			// Can't update readings more often than 2 sec			
			// 1 byte response code
			resp_write(RESP_UP_TO_DATE);
			resp_end();
		}
		else
		{
			lastUpdateTime = time;

			int errorCode = dht.update();
			if(errorCode == DHT_E_TIMEOUT)
			{
				// DHT22 doesn't answer

				// 1 byte response code
				resp_write(RESP_DEVICE_ERROR);
				resp_end();
			}
			else
			{
				// Ignore checksum errors

				// 1 byte response code
				resp_write(RESP_OK);
				resp_end();

				blink(5, 1000);
			}
		}
		break;

	case CMD_GET_TEMPERATURE:
		// Get last temperature command

		// 1 byte response code
		// 4 bytes - temperature (float) 
		resp_write(RESP_TEMPERATURE);
		resp_write(dht.temperature());
		resp_end();
		break;


	case CMD_GET_HUMIDITY:
		// Get last humidity command

		// 1 byte response code
		// 4 bytes - humidity (float) 
		resp_write(RESP_HUMIDITY);
		resp_write(dht.humidity());
		resp_end();
		break;

	default:
		// Unknown command
		// 1 byte response code
		resp_write(RESP_WRONG_COMMAND);
		resp_end();
		break;
	}
}

void setup () 
{
	Serial.begin(57600);
}

void loop() 
{
	uint8_t command = cmd_read();
	handle_command(command);
}
