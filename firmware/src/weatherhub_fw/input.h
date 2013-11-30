#ifndef _INPUT_h
#define _INPUT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <esr.h>

namespace input
{
	extern esr::thread_id thread;

	const int BTN1_PIN = A0;
	const int BTN2_PIN = A1;
	const int BTN3_PIN = A2;

	void thread_func(esr::message msg);
}

#endif

