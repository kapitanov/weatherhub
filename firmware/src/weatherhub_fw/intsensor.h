#ifndef _INTSENSOR_h
#define _INTSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <esr.h>
#include <DHT.h>
#include "globals.h"

namespace intsensor
{
	extern esr::thread_id thread;
	extern DHT sensor;

	extern sensor_reading reading;

	void thread_func(esr::message msg);
}

#endif

