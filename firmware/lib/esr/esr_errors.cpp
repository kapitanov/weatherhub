#include "esr_errors.h"
#include "esr_io.h"

#ifdef __ESR_ENABLE_ERROR_FORMATTING

namespace res
{
	PROGMEM char E_UNKNOWN[] = "E_UNKNOWN";
	PROGMEM char E_OK[] = "E_OK";
	PROGMEM char E_NO_FREE_THREAD_SLOTS[] = "E_NO_FREE_THREAD_SLOTS";
	PROGMEM char E_WRONG_THREAD[] = "E_WRONG_THREAD";
	PROGMEM char E_MESSAGE_QUEUE_IS_FULL[] = "E_MESSAGE_QUEUE_IS_FULL";
	PROGMEM char E_WRONG_MESSAGE[] = "E_WRONG_MESSAGE";
	PROGMEM char E_WRONG_PERIOD[] = "E_WRONG_PERIOD";
	PROGMEM char E_TIMER_NOT_DEFINED[] = "E_TIMER_NOT_DEFINED";
	PROGMEM char E_NOT_IN_SCHEDULER[] = "E_NOT_IN_SCHEDULER";
	PROGMEM char E_LOG_CONFIGURATION_IS_INCORRECT[] = "E_LOG_CONFIGURATION_IS_INCORRECT";	
	PROGMEM char E_INCORRECT_FORMAT[] = "E_INCORRECT_FORMAT";	
}

#define _CASE(name) case esr::name: message = reinterpret_cast<const __FlashStringHelper*>(res::name); break;

/**
* Returns a PROGMEM string containing name of error code @e
* @param e error code
* @return a PROGMEM string pointer
*/
const __FlashStringHelper* esr::get_error_name(esr::error e)
{
	const __FlashStringHelper* message;

	// Pick a message text from flash
	switch (e)
	{
		_CASE(E_OK);
		_CASE(E_NO_FREE_THREAD_SLOTS);		
		_CASE(E_WRONG_THREAD);
		_CASE(E_MESSAGE_QUEUE_IS_FULL);
		_CASE(E_WRONG_MESSAGE);
		_CASE(E_WRONG_PERIOD);
		_CASE(E_TIMER_NOT_DEFINED);
		_CASE(E_NOT_IN_SCHEDULER);		
		_CASE(E_LOG_CONFIGURATION_IS_INCORRECT);
		_CASE(E_INCORRECT_FORMAT);

	default:
		message = reinterpret_cast<const __FlashStringHelper*>(res::E_UNKNOWN);
		break;
	}

	return message;
}

/**
* Copies an error message corresponding to error code info buffer
* @param buffer text buffer (expected at least 32 bytes of memory)
* @param e error code
*/
void esr::format_error(char* buffer, esr::error e)
{
	uint16_t message = reinterpret_cast<uint16_t>(esr::get_error_name(e));

	// Copy message into buffer
	uint8_t i = 0;
	while(true)
	{
		char c = pgm_read_byte(message + i);
		buffer[i] = c;
		++i;

		if(c == 0)
		{
			return;
		}
	}
}

const __FlashStringHelper* get_function_name(esr::function_name function)
{
	switch(function)
	{
	case esr::FUNC_LOG_INIT:
		return F("log_init");
	case esr::FUNC_LOG:
		return F("log");
	case esr::FUNC_BEGIN_THREAD:
		return F("begin_thread");
	case esr::FUNC_GET_THREAD_FLAG:
		return F("get_thread_flag");
	case esr::FUNC_SET_THREAD_FLAG:
		return F("set_thread_flag");
	case esr::FUNC_KILL_THREAD:
		return F("kill_thread");
	case esr::FUNC_POST_MESSAGE:
		return F("post_message");
	case esr::FUNC_SET_TIMER_MS:
		return F("set_timer_ms");
	case esr::FUNC_CHANGE_TIMER_MS:
		return F("change_timer_ms");
	case esr::FUNC_CLEAR_TIMER:
		return F("clear_timer");
	case esr::FUNC_GET_CURRENT_THREAD_ID:
		return F("get_current_thread_id");
	default:
		return F("<none>");
	}
}

/**
* Verifies ESR function call result and writes a ERROR log message
* @param e error code
* @param function name of function invoked
* @return true if no error happened, false otherwise
*/
bool esr::verify(esr::error e, esr::function_name function)
{
	if(e != esr::E_OK)
	{
		const __FlashStringHelper* function_name = get_function_name(function);
		esr::log(esr::LOG_ERROR, F("esr::%ps() failed with %e"), function_name, e);
		return false;
	}

	return true;
}

#endif