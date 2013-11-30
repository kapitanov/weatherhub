#include "settings.h"
#include <esr.h>

using namespace esr;
using namespace gui;
using namespace settings;

const int EEPROM_FIRST_RUN = 0;

void settings::init()
{
	uint8_t x = EEPROM.read(EEPROM_FIRST_RUN);
	if(x != 0xEE)
	{
		set_unit(UNIT_C);
		set_sensor(SENSOR_INT);
		set_int_calibration(0);
		set_ext_calibration(0);

		EEPROM.write(EEPROM_FIRST_RUN, 0xEE);
	}
}


const int EEPROM_UNIT = 1;

unit settings::get_unit()
{
	uint8_t x = EEPROM.read(EEPROM_UNIT);
	return static_cast<unit>(x);
}

void settings::set_unit(unit unit)
{
	EEPROM.write(EEPROM_UNIT, static_cast<uint8_t>(unit));
}


const int EEPROM_SENSOR = 2;

sensor_id settings::get_sensor()
{
	uint8_t x = EEPROM.read(EEPROM_SENSOR);
	return static_cast<sensor_id>(x);
}

void settings::set_sensor(sensor_id id)
{
	EEPROM.write(EEPROM_SENSOR, static_cast<uint8_t>(id));
}


const int EEPROM_INT_CALIBRATION = 3;
const int EEPROM_EXT_CALIBRATION = 4;

float get_calibration(int address)
{
	int8_t value = static_cast<int8_t>(EEPROM.read(address));	
	float calibration = static_cast<float>(value - 127);
	return calibration;
}

void set_calibration(int address, float calibration)
{
	int8_t value = static_cast<int8_t>(calibration);
	EEPROM.write(address, value + 127);
}


float settings::clamp_calibration(float c)
{
	if(c <= -127.0)
	{
		return -127.0;
	}

	if(c >= 127.0)
	{
		return 127.0;
	}

	return c;
}

float settings::get_int_calibration()
{
	return get_calibration(EEPROM_INT_CALIBRATION);
}

float settings::get_ext_calibration()
{
	return get_calibration(EEPROM_EXT_CALIBRATION);
}

void settings::set_int_calibration(float c)
{
	log(LOG_DEBUG, F("EEPROM\tset_int_calibration(%f)"), &c);
	set_calibration(EEPROM_INT_CALIBRATION, c);
}

void settings::set_ext_calibration(float c)
{
	log(LOG_DEBUG, F("EEPROM\tset_ext_calibration(%f)"), &c);
	set_calibration(EEPROM_EXT_CALIBRATION, c);
}