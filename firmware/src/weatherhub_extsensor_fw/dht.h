#ifndef _DHT_h
#define _DHT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

enum dht_error
{
	DHT_OK,
	DHT_E_CHECKSUM,
	DHT_E_TIMEOUT
};

class dht_driver
{
public:
	dht_driver(int pin);

	float temperature() { return _temperature; }
	float humidity() { return _humidity; }

	dht_error update();
	
private:
	int _pin;
	float _temperature;
	float _humidity;
};

#endif

