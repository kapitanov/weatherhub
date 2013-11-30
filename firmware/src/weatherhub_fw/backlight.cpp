#include "backlight.h"
#include "globals.h"

using namespace esr;
using namespace backlight;

thread_id backlight::thread;

void backlight::thread_func(message msg)
{
	switch (msg)
	{
	case MSG_BL_INIT:
		log(LOG_INFO, F("BL\tinit"));
		pinMode(BL_PIN, OUTPUT);
		analogWrite(BL_PIN, 255);
		break;
	}
}
