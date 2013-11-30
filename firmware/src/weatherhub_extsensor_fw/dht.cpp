#include "dht.h"

dht_driver::dht_driver(int pin) : _pin(pin) { } 

dht_error dht_driver::update()
{
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delay(18);
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);
	pinMode(_pin, INPUT);

	// ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = 10000;
	while(digitalRead(_pin) == LOW)
		if (loopCnt-- == 0) return DHT_E_TIMEOUT;

	loopCnt = 10000;
	while(digitalRead(_pin) == HIGH)
		if (loopCnt-- == 0) return DHT_E_TIMEOUT;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (int i=0; i<40; i++)
	{
		loopCnt = 10000;
		while(digitalRead(_pin) == LOW)
			if (loopCnt-- == 0) return DHT_E_TIMEOUT;

		unsigned long t = micros();

		loopCnt = 10000;
		while(digitalRead(_pin) == HIGH)
			if (loopCnt-- == 0) return DHT_E_TIMEOUT;

		if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}

	/*
	TinySerial.print("dht_data: [ 0x");
	TinySerial.print(bits[0], 16);
	TinySerial.print(", 0x");
	TinySerial.print(bits[1], 16);
	TinySerial.print(", 0x");
	TinySerial.print(bits[2], 16);
	TinySerial.print(", 0x");
	TinySerial.print(bits[3], 16);
	TinySerial.print(", 0x");
	TinySerial.print(bits[4], 16);
	TinySerial.print(" ]");
	TinySerial.println();*/

	_temperature = bits[2] & 0x7F;
	_temperature *= 256;
	_temperature += bits[3];
	_temperature /= 10;
	if (bits[2] & 0x80) { 	
		_temperature *= -1; 
	}

	_humidity = bits[0];
	_humidity *= 256;
	_humidity += bits[1];
	_humidity /= 10;
	/*
	TinySerial.print("dht_temp: ");
	TinySerial.print(temperature);
	TinySerial.println("deg C");

	TinySerial.print("dht_humidity: ");
	TinySerial.print(humidity);
	TinySerial.println("%");
	*/

	if(bits[4] == ((bits[0] + bits[1] + bits[2] + bits[3]) & 0xFF))
	{
		return DHT_E_CHECKSUM;
	}

	return DHT_OK;
}