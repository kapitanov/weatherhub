#include "intsensor.h"
#include "globals.h"
#include "gui.h"
#include "settings.h"

using namespace esr;
using namespace intsensor;
using namespace settings;

thread_id		intsensor::thread;
DHT				intsensor::sensor(2, DHT11);
sensor_reading	intsensor::reading;

void intsensor::thread_func(message msg)
{
	switch (msg)
	{
	case MSG_INTSENSOR_INIT:
		log(LOG_INFO, F("INTSNSR\tinit"));
		sensor.begin();

		set_thread_flag(THREAD_CURRENT, THREAD_IMMEDIATE_TIMER, true);
		set_thread_flag(THREAD_CURRENT, THREAD_REPEAT_TIMER, true);
		set_timer_ms(THREAD_CURRENT, 10*1000);
		break;

	case MSG_TIMER:
		log(LOG_INFO, F("INTSNSR\tupdate"));
		
		float t = sensor.readTemperature();
		float h = sensor.readHumidity();
		if(isnan(t) || isnan(h))
		{
			log(LOG_ERROR, F("INTSNSR\tfailure"));
			break;;
		}

		reading.temperature = t;
		reading.humidity = h;
		reading.status = STATUS_OK;

		float c = get_int_calibration();
		reading.temperature += c;

		log(LOG_DEBUG, F("INTSNSR\t< calibration(%f)"), &c);
		log(LOG_INFO, F("INTSNSR\tt = %f deg C, h = %f%%"), &reading.temperature, &reading.humidity);
		
		post_message(gui::thread, MSG_INTSENSOR_CHANGED);

		break;
	}
}
