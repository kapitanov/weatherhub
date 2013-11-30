#include "gui.h"
#include "globals.h"
#include "intsensor.h"
#include "extsensor.h"
#include "settings.h"

using namespace esr;
using namespace gui;
using namespace gui::fsm;
using namespace settings;

thread_id			gui::thread;
Adafruit_PCD8544	gui::lcd  = Adafruit_PCD8544(7, 6, 5, 4, 3);

unit				active_unit			= UNIT_C;
sensor_id			active_sensor		= SENSOR_INT;

bool				enable_progress_bar = false;
uint8_t				progress_bar_state  = 0;

float				calibration_base;
float				calibration_offset;
float				calibration_out;

void gui_bootscreen()
{
	lcd.clearDisplay();
	lcd.setTextColor(BLACK);

	lcd.setTextSize(2);	
	lcd.setCursor(25, 15);
	lcd.print(F("ESR"));

	lcd.setTextSize(1);	
	lcd.setCursor(22, 40);
	lcd.print(F("boot up"));

	lcd.display();
}

void gui_frame(const __FlashStringHelper* title)
{
	lcd.clearDisplay();
	lcd.fillRect(0, 0, LCDWIDTH, 10, BLACK);
	lcd.setTextSize(1);	
	lcd.setCursor(2, 2);
	lcd.setTextColor(WHITE);
	lcd.print(title);
	lcd.drawFastHLine(0, 10, LCDWIDTH - 1, BLACK);
	lcd.drawFastHLine(0, LCDHEIGHT - 1, LCDWIDTH - 1, BLACK);
	lcd.drawFastVLine(0, 10, LCDHEIGHT - 1, BLACK);
	lcd.drawFastVLine(LCDWIDTH - 1, 10, LCDHEIGHT - 1, BLACK);

	if(enable_progress_bar)
	{
		if(progress_bar_state > 4)
		{
			progress_bar_state = 0;
		}

		lcd.drawFastHLine(74, 1, 9, WHITE);
		lcd.drawFastHLine(74, 9, 9, WHITE);
		lcd.drawFastVLine(74, 1, 9, WHITE);
		lcd.drawFastVLine(82, 1, 9, WHITE);

		lcd.drawFastHLine(76, 3 + progress_bar_state, 5, WHITE);

		++progress_bar_state;
	}
}

uint16_t power_of_10(uint8_t p)
{
	uint16_t r = 1;
	for(uint8_t i = 0; i < p; ++i)
	{
		r *= 10;
	}
	return r;
}

char gui_get_digit(uint16_t value, uint8_t index)
{
	/*
	123155
	^ 2
	% 1000 = 10 ^ (i + 1) 
	155
	/ 100 = 10 ^ i
	1
	*/

	uint8_t d = static_cast<uint8_t>(
		value % power_of_10(index + 1)
		/  power_of_10(index)
		);

	return d + '0';
}

void gui_print_float(float& f, char* buffer)
{
	uint16_t value = static_cast<uint16_t>(abs( f * 10.0));

	buffer[0] = f >= 0 ? '+' : '-';
	buffer[1] = gui_get_digit(value, 3);
	buffer[2] = gui_get_digit(value, 2);
	buffer[3] = gui_get_digit(value, 1);
	buffer[4] = '.';
	buffer[5] = gui_get_digit(value, 0);
	buffer[6] = 0;

	if(buffer[1] == '0')
	{
		buffer[1] = ' ';

		if(buffer[2] == '0')
		{
			buffer[2] = ' ';
		}
	}
}

void gui_value(int16_t y, float& v, unit u)
{
	char text[7] = "";
	gui_print_float(v, text);
	text[4] = 0;
	text[5] = 0;
	text[6] = 0;

	if(u == UNIT_K || u == UNIT_PERCENT)
	{
		text[0] = ' ';
	}

	lcd.setTextSize(2);	
	lcd.setTextColor(BLACK);
	lcd.setCursor(5, y);
	lcd.print(text);

	switch(u)	
	{
	case UNIT_C:
		lcd.setTextSize(1);	
		lcd.setCursor(60, y);
		lcd.print('O');
		lcd.setTextSize(2);	
		lcd.setCursor(70, y);
		lcd.print('C');
		break;

	case UNIT_F:
		lcd.setTextSize(1);	
		lcd.setCursor(60, y);
		lcd.print('O');
		lcd.setTextSize(2);	
		lcd.setCursor(70, y);
		lcd.print('F');
		break;

	case UNIT_K:
		lcd.setTextSize(2);	
		lcd.setCursor(60, y);
		lcd.print('K');
		break;

	case UNIT_PERCENT:
		lcd.setTextSize(1);	
		lcd.setCursor(60, y);
		lcd.print('%');
		break;
	}
}

void gui_convert_unit(float& t, unit u)
{
	switch (u)
	{
	case gui::UNIT_F:		
		// [°F] = [°C] x 9/5 + 32
		t = t * 9.0 / 5.0 + 32.0;
		break;
	case gui::UNIT_K:
		// [K] = [°C] + 273.15
		t += 273.15;
		break;
	}
}

void gui_scroll_unit()
{
	const __FlashStringHelper* name = F("?");
	switch (active_unit)
	{
	case UNIT_C:
		active_unit = UNIT_F;
		name = F("UNIT_F");
		break;
	case UNIT_F:
		active_unit = UNIT_K;
		name = F("UNIT_K");
		break;
	case UNIT_K:
	default:
		active_unit = UNIT_C;
		name = F("UNIT_C");
		break;
	}

	set_unit(active_unit);
	log(LOG_DEBUG, F("GUI\tactive_sensor = %ps"), name);
}

void gui_scroll_sensor()
{
	const __FlashStringHelper* name = F("?");
	switch (active_sensor)
	{
	case SENSOR_INT:
		active_sensor = SENSOR_EXT;
		name = F("SENSOR_EXT");
		break;
	case SENSOR_EXT:
	default:
		active_sensor = SENSOR_INT;
		name = F("SENSOR_INT");
		break;
	}

	set_sensor(active_sensor);
	log(LOG_DEBUG, F("GUI\tactive_sensor = %ps"), name);
}

void gui_device_error()
{
	lcd.setTextSize(2);	
	lcd.setTextColor(BLACK);
	lcd.setCursor(12, 20);
	lcd.print(F("ERROR"));
}

void gui_no_data()
{
	lcd.setTextSize(1);	
	lcd.setTextColor(BLACK);
	lcd.setCursor(10, 23);
	lcd.print(F("Updating..."));
}

void gui_reading(sensor_reading* reading)
{	
	float t = reading->temperature;
	float h = reading->humidity;

	gui_convert_unit(t, active_unit);
	gui_value(13, t, active_unit);
	gui_value(30, h, UNIT_PERCENT);
}

void gui_indicator()
{
	sensor_reading* reading;

	switch (active_sensor)
	{
	case gui::SENSOR_INT:
		gui_frame(F("Room"));
		reading = &intsensor::reading;
		break;

	case gui::SENSOR_EXT:
		gui_frame(F("Street"));
		reading = &extsensor::reading;
		break;
	}

	switch (reading->status)
	{
	case STATUS_OK:
		gui_reading(reading);
		break;
	case STATUS_NO_DATA:
		gui_no_data();
		break;
	case STATUS_ERROR:
		gui_device_error();
		break;
	}
}

void gui_calibration()
{
	calibration_out = calibration_base + calibration_offset;
	const __FlashStringHelper* name;

	switch (active_sensor)
	{
	case SENSOR_INT:
		name = F("Calibrate (INT)");
		break;
	case SENSOR_EXT:
		name = F("Calibrate (EXT)");
		break;
	}

	gui_frame(name);
	float f = calibration_out;
	gui_convert_unit(f, active_unit);
	gui_value(13, f, active_unit);
	gui_value(30, calibration_offset, UNIT_NONE);
	lcd.display();
}

fsm_state gui::fsm::handler = state_initial;

void gui::fsm::state_initial(message msg)
{
	switch (msg)
	{
	case MSG_GUI_INIT:
		log(LOG_INFO, F("GUI\tinit"));

		lcd.begin();
		lcd.setContrast(45);

		gui_bootscreen();
		handler = state_indicator;

		active_unit = get_unit();
		set_unit(active_unit);

		active_sensor = get_sensor();
		set_sensor(active_sensor);
		break;
	}
}

void gui::fsm::state_indicator(message msg)
{
	switch (msg)
	{
	case MSG_INTSENSOR_CHANGED:
	case MSG_EXTSENSOR_CHANGED:
		log(LOG_INFO, F("GUI\tindicator"));
		gui_indicator();
		lcd.display();
		break;

	case MSG_BNTPRESS_UNIT:
		gui_scroll_unit();
		switch (active_sensor)
		{
		case gui::SENSOR_INT:
			post_message(THREAD_CURRENT, MSG_INTSENSOR_CHANGED);
			break;

		case gui::SENSOR_EXT:
			post_message(THREAD_CURRENT, MSG_EXTSENSOR_CHANGED);
			break;
		}		
		break;

	case MSG_BNTPRESS_SENSOR:
		gui_scroll_sensor();
		switch (active_sensor)
		{
		case gui::SENSOR_INT:
			post_message(THREAD_CURRENT, MSG_INTSENSOR_CHANGED);
			break;

		case gui::SENSOR_EXT:
			post_message(THREAD_CURRENT, MSG_EXTSENSOR_CHANGED);
			break;
		}
		break;


	case MSG_BNTPRESS_MODE:
		log(LOG_INFO, F("GUI\tindicator calibrate"));
		handler = state_calibration;
		post_message(THREAD_CURRENT, MSG_GUI_INIT);	
		break;


	case MSG_SENSOR_UPDATE_BEGIN:
		enable_progress_bar = true;
		progress_bar_state = 0;
		set_timer_ms(THREAD_CURRENT, 100);
		gui_indicator();
		lcd.display();
		break;

	case MSG_SENSOR_UPDATE_END:
		enable_progress_bar = false;
		progress_bar_state = 0;
		clear_timer(THREAD_CURRENT);
		gui_indicator();
		lcd.display();
		break;

	case MSG_TIMER:
		gui_indicator();
		lcd.display();
		break;
	}
}

void gui::fsm::state_calibration(message msg)
{
	switch (msg)
	{
	case MSG_GUI_INIT:
		// init calibration mode
		log(LOG_INFO, F("GUI\tcalibration init"));
		switch (active_sensor)
		{
		case gui::SENSOR_INT:
			calibration_base = intsensor::reading.temperature;
			calibration_offset = get_int_calibration();
			break;
		case gui::SENSOR_EXT:
			calibration_base = extsensor::reading.temperature;
			calibration_offset = get_ext_calibration();
			break;
		default:
			calibration_base = 0;
			break;
		}
		gui_calibration();
		break;

	case MSG_BNTPRESS_MODE:
		// commit		
		log(LOG_INFO, F("GUI\tcalibration commit"));
		handler = state_indicator;
		switch (active_sensor)
		{
		case SENSOR_INT:
			set_int_calibration(calibration_offset);
			post_message(THREAD_CURRENT, MSG_INTSENSOR_CHANGED);
			break;
		case SENSOR_EXT:
			set_ext_calibration(calibration_offset);
			post_message(THREAD_CURRENT, MSG_EXTSENSOR_CHANGED);
			break;
		}
		break;

	case MSG_BNTPRESS_UNIT:
		// increment calibration
		calibration_offset = clamp_calibration(calibration_offset + 1);
		log(LOG_INFO, F("GUI\tcalibration inc %f"), &calibration_offset);
		gui_calibration();
		break;

	case MSG_BNTPRESS_SENSOR:
		// decrement calibration
		calibration_offset = clamp_calibration(calibration_offset - 1);
		log(LOG_INFO, F("GUI\tcalibration dec %f"), &calibration_offset);
		gui_calibration();
		break;
	}
}

void gui::thread_func(message msg)
{
	handler(msg);
}
