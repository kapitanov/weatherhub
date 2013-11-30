// eeprom.h

#ifndef _EEPROM_h
#define _EEPROM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <EEPROM.h>
#include "gui.h"

namespace settings
{
	void			init();

	gui::unit		get_unit();
	void			set_unit(gui::unit unit);

	gui::sensor_id	get_sensor();
	void			set_sensor(gui::sensor_id id);

	float			get_int_calibration();
	float			get_ext_calibration();

	float			clamp_calibration(float c);

	void			set_int_calibration(float c);
	void			set_ext_calibration(float c);
}

#endif

