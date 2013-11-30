#ifndef _BACKLIGHT_h
#define _BACKLIGHT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <esr.h>

namespace backlight
{
	extern esr::thread_id thread;

	const int BL_PIN = 9;

	void thread_func(esr::message msg);
}

#endif

