#include "extsensor.h"
#include "gui.h"
#include "globals.h"
#include "settings.h"

using namespace esr;
using namespace extsensor;
using namespace extsensor::fsm;
using namespace settings;

thread_id		extsensor::thread;
sensor_reading	extsensor::reading;
state			extsensor::fsm::handler = state_initial;

const uint8_t	BUFFER_SIZE = 32;
char			buffer[BUFFER_SIZE];
uint8_t			buffer_index;

void buffer_reset()
{
	for (uint8_t i = 0; i < BUFFER_SIZE; i++)
	{
		buffer[i] = 0;
	}

	buffer_index = 0;
}

void extsensor::fsm::state_initial()
{
	log(LOG_DEBUG, F("EXTSNSR\tstate_initial"));
	reading.status = STATUS_NO_DATA;

	// Send 'U' to extsensor
	log(LOG_INFO, F("EXTSNSR\tbegin identify"));
	log(LOG_DEBUG, F("EXTSNSR\t> %c"), CMD_IDENTIFY);
	uart.print(CMD_IDENTIFY);

	while(uart.available() <= 0) { }

	char c = uart.read();
	log(LOG_DEBUG, F("EXTSNSR\t< %c"), c);
	switch (c)
	{
	case RESP_IDENTITY:
		{
			buffer_reset();
			while(true)
			{
				while(uart.available() <= 0) { }

				char c = uart.read();
				if(c == '\n')
				{
					break;
				}

				buffer[buffer_index] = c;
				++buffer_index;
			}
			log(LOG_DEBUG, F("EXTSNSR\t< %s"), buffer);
			log(LOG_DEBUG, F("EXTSNSR\tdevice identified"));
		}
		break;
	default:
		log(LOG_ERROR, F("EXTSNSR\tunknown device"));
		
		while(uart.available() > 0)
		{
			uart.read();
		}
		break;
	}

	// Send 'U' to extsensor
	log(LOG_INFO, F("EXTSNSR\tbegin update"));
	log(LOG_DEBUG, F("EXTSNSR\t> %c"), CMD_UPDATE);
	uart.print(CMD_UPDATE);

	// Enable idle loop
	set_thread_flag(THREAD_CURRENT, THREAD_IDLE_LOOP, true);
	extsensor::fsm::handler = state_wait_for_update;

	// Send message to GUI
	post_message(gui::thread, MSG_SENSOR_UPDATE_BEGIN);
}

void extsensor::fsm::state_wait_for_update()
{
	if(uart.available() >= 2)
	{
		char c = uart.read();
		uart.read();
		log(LOG_DEBUG, F("EXTSNSR\t< %c"), c);
		switch (c)
		{
		case RESP_OK:
			reading.status = STATUS_OK;
			break;

		default:
			reading.status = STATUS_ERROR;
			break;
		}
		
		// Send 'T' to extsensor
		log(LOG_DEBUG, F("EXTSNSR\t> %c"), CMD_GET_TEMPERATURE);
		uart.print(CMD_GET_TEMPERATURE);
		buffer_reset();
		extsensor::fsm::handler = state_wait_for_t;
	}
}

uint8_t parse_digit(char c)
{
	switch (c)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	default:  return 0;
	}
}

bool receive_float(float& value)
{
	while(uart.available() > 0) 
	{
		char c = uart.read();
		if(c == '\n')
		{
			log(LOG_DEBUG, F("EXTSNSR\t< %s"), buffer);
			// Parse float:
			// Format: T+000.0

			float f = 0;
			f += parse_digit(buffer[2]) * 100.0;
			f += parse_digit(buffer[3]) * 10.0;
			f += parse_digit(buffer[4]) * 1.0;
			f += parse_digit(buffer[6]) * 0.1;

			if(buffer[1] == '-')
			{
				f *= -1.0;
			}

			value = f;
			return true;
		}

		buffer[buffer_index] = c;
		++buffer_index;
	}

	return false;
}

void extsensor::fsm::state_wait_for_t()
{
	if(receive_float(reading.temperature))
	{
		log(LOG_DEBUG, F("EXTSNSR\t< temperature(%f)"), &reading.temperature);
		float c = get_ext_calibration();
		reading.temperature += c;

		log(LOG_DEBUG, F("EXTSNSR\t< calibration(%f)"), &c);
		log(LOG_DEBUG, F("EXTSNSR\t< temperature_c(%f)"), &reading.temperature);

		// Send 'H' to extsensor
		log(LOG_DEBUG, F("EXTSNSR\t> %c"), CMD_GET_HUMIDITY);
		uart.print(CMD_GET_HUMIDITY);
		buffer_reset();

		extsensor::fsm::handler = state_wait_for_h;
	}
}

void extsensor::fsm::state_wait_for_h()
{
	if(receive_float(reading.humidity))
	{
		log(LOG_DEBUG, F("EXTSNSR\t< humidity(%f)"), &reading.humidity);
		
		set_thread_flag(THREAD_CURRENT, THREAD_IDLE_LOOP, false);
		set_thread_flag(THREAD_CURRENT, THREAD_IMMEDIATE_TIMER, false);
		set_timer_ms(THREAD_CURRENT, 10*1000);

		post_message(gui::thread, MSG_EXTSENSOR_CHANGED);
		extsensor::fsm::handler = state_initial;

		// Send message to GUI
		post_message(gui::thread, MSG_SENSOR_UPDATE_END);
	}
}

void extsensor::thread_func(message msg)
{
	switch (msg)
	{
	case MSG_EXTSENSOR_INIT:
		log(LOG_INFO, F("EXTSNSR\tinit"));
		uart.begin(57600);

		set_thread_flag(THREAD_CURRENT, THREAD_IMMEDIATE_TIMER, true);
		set_thread_flag(THREAD_CURRENT, THREAD_REPEAT_TIMER, false);
		set_thread_flag(THREAD_CURRENT, THREAD_IDLE_LOOP, false);
		set_timer_ms(THREAD_CURRENT, 10*1000);
		break;

	case MSG_TIMER:
	case MSG_IDLE:
		extsensor::fsm::handler();
		break;
	}
}

