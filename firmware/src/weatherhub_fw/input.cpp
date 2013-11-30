#include "input.h"
#include "gui.h"
#include "globals.h"

using namespace esr;
using namespace input;

const long KEYPAD_PERIOD  = 250;

thread_id input::thread;
long last_update_time = -1;

enum button
{
	BTN_NONE	= 0,
	BTN_MODE	= MSG_BNTPRESS_MODE,
	BTN_SENSOR  = MSG_BNTPRESS_SENSOR,
	BTN_UNIT	= MSG_BNTPRESS_UNIT
};

button read_button()
{
	uint8_t value1 = digitalRead(BTN1_PIN);
	uint8_t value2 = digitalRead(BTN2_PIN);
	uint8_t value3 = digitalRead(BTN3_PIN);

	if(value1 != 0 ||
		value2 != 0 ||
		value3 != 0)
	{
		log(LOG_DEBUG, F("INPUT\tbtn state [ %c %c %c ]"), 
			value1 == HIGH ? '1': '0',
			value2 == HIGH ? '1': '0',
			value3 == HIGH ? '1': '0');
	}

	if(value1 == HIGH)
	{
		log(LOG_INFO, F("INPUT\tpressed <UNIT>"));
		return BTN_UNIT;
	}

	if(value2 == HIGH)
	{
		log(LOG_INFO, F("INPUT\tpressed <MODE>"));
		return BTN_MODE;
	}

	if(value3 == HIGH)
	{
		log(LOG_INFO, F("INPUT\tpressed <SENSOR>"));
		return BTN_SENSOR;
	}

	return BTN_NONE;
}

void input::thread_func(esr::message msg)
{
	switch (msg)
	{
	case MSG_INPUT_INIT:
		log(LOG_INFO, F("INPUT\tinit"));
		pinMode(BTN1_PIN, INPUT);
		pinMode(BTN2_PIN, INPUT);
		pinMode(BTN3_PIN, INPUT);
		break;

	case MSG_IDLE:
		{
			long time = millis();
			if(time - last_update_time < KEYPAD_PERIOD)
			{
				break;
			}

			button btn = read_button();
			if(btn != BTN_NONE)
			{
				last_update_time = time;
				post_message(gui::thread, static_cast<message>(btn));
			}
		}
		break;
	}
}
