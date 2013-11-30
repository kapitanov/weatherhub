#ifndef _GUI_h
#define _GUI_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <esr.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

namespace gui
{
	enum unit
	{
		UNIT_NONE,
		UNIT_PERCENT,
		UNIT_C,
		UNIT_F,
		UNIT_K		
	};

	enum sensor_id
	{
		SENSOR_INT,
		SENSOR_EXT,
	};

	extern esr::thread_id thread;
	extern Adafruit_PCD8544 lcd;

	void thread_func(esr::message msg);

	namespace fsm
	{
		typedef void (*fsm_state)(esr::message msg);

		extern fsm_state handler;

		void state_initial(esr::message msg);
		void state_indicator(esr::message msg);
		void state_calibration(esr::message msg);
	}
}

#endif

