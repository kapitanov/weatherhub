#ifndef _EXTSENSOR_h
#define _EXTSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <esr.h>
#include <SoftwareSerial.h>
#include "globals.h"

namespace extsensor
{
	extern esr::thread_id thread;
	extern SoftwareSerial uart;

	extern sensor_reading reading;

	void thread_func(esr::message msg);

	const char CMD_NONE				= 'N';
	const char CMD_IDENTIFY			= 'I';
	const char CMD_UPDATE			= 'U';
	const char CMD_GET_TEMPERATURE	= 'T';
	const char CMD_GET_HUMIDITY		= 'H';


	const char RESP_NONE		= 'N';
	const char RESP_OK		= 'O';
	const char RESP_IDENTITY		= 'I';
	const char RESP_WRONG_COMMAND		= 'W';
	const char RESP_DEVICE_ERROR		= 'E';
	const char RESP_UP_TO_DATE		= 'D';
	const char RESP_TEMPERATURE		= 'T';
	const char RESP_HUMIDITY		= 'H';

	namespace fsm
	{
		typedef void (*state)();

		extern state handler;

		void state_initial();
		void state_wait_for_update();
		void state_wait_for_t();
		void state_wait_for_h();
	}
}

#endif

