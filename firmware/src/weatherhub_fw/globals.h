#ifndef _GLOBALS_h
#define _GLOBALS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <esr.h>

const esr::message MSG_GUI_INIT				= esr::MSG_USER + 0;
const esr::message MSG_BL_INIT				= esr::MSG_USER + 1;
const esr::message MSG_INTSENSOR_INIT		= esr::MSG_USER + 2;
const esr::message MSG_INTSENSOR_CHANGED	= esr::MSG_USER + 3;
const esr::message MSG_INPUT_INIT			= esr::MSG_USER + 4;
const esr::message MSG_BNTPRESS_MODE		= esr::MSG_USER + 5;
const esr::message MSG_BNTPRESS_SENSOR		= esr::MSG_USER + 6;
const esr::message MSG_BNTPRESS_UNIT		= esr::MSG_USER + 7;
const esr::message MSG_EXTSENSOR_INIT		= esr::MSG_USER + 8;
const esr::message MSG_EXTSENSOR_CHANGED	= esr::MSG_USER + 9;
const esr::message MSG_SENSOR_UPDATE_BEGIN	= esr::MSG_USER + 10;
const esr::message MSG_SENSOR_UPDATE_END	= esr::MSG_USER + 11;

enum sensor_status
{
	STATUS_OK,
	STATUS_NO_DATA,
	STATUS_ERROR
};

struct sensor_reading
{
	sensor_status status;
	float temperature;
	float humidity;
};

#endif

