#ifndef _ESR_ERRORS_h
#define _ESR_ERRORS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

namespace esr
{
	/**
	 * Error codes
	 */
	enum
	{
		/**
		* No error happened
		*/
		E_OK = 0,

		/**
		* Unable to create a new thread. All thread slots are taken.
		*/
		E_NO_FREE_THREAD_SLOTS,

		/**
		* Wrong thread identifier has been specified.
		*/
		E_WRONG_THREAD,

		/**
		* Unable to post message into the message queue. Message queue is full.
		*/
		E_MESSAGE_QUEUE_IS_FULL,

		/**
		* Wrong message code has been specified.
		*/
		E_WRONG_MESSAGE,

		/**
		* Wrong timer period has been specified.
		*/
		E_WRONG_PERIOD,

		/**
		* Wrong flag has been specified.
		*/
		E_WRONG_FLAG,

		/**
		* The specified thread has no active timer defined.
		*/
		E_TIMER_NOT_DEFINED,

		/**
		* The function has been called from outside of scheduler threads but it requires scheduler context.
		*/
		E_NOT_IN_SCHEDULER,

		/**
		* esr::log_init() has not been called before calling esr::log()
		*/
		E_LOG_CONFIGURATION_IS_INCORRECT,

		/**
		* Incorrect format string
		*/
		E_INCORRECT_FORMAT
	};

	/**
	* Error code
	*/
	typedef uint8_t error;

#ifdef __ESR_ENABLE_ERROR_FORMATTING

	/**
	* Returns a PROGMEM string containing name of error code @e
	* @param e error code
	* @return a PROGMEM string pointer
	*/
	const __FlashStringHelper* get_error_name(esr::error e);

	/**
	* Copies an error message corresponding to error code info buffer
	* @param buffer text buffer (expected at least 32 bytes of memory)
	* @param e error code
	*/
	void format_error(char* buffer, esr::error e);

	/**
	* Function name's identifier
	*/
	enum function_name
	{
		FUNC_NONE,
		FUNC_LOG_INIT,
		FUNC_LOG,
		FUNC_BEGIN_THREAD,
		FUNC_GET_THREAD_FLAG,
		FUNC_SET_THREAD_FLAG,
		FUNC_KILL_THREAD,
		FUNC_POST_MESSAGE,
		FUNC_SET_TIMER_MS,
		FUNC_CHANGE_TIMER_MS,
		FUNC_CLEAR_TIMER,
		FUNC_GET_CURRENT_THREAD_ID
	};

	/**
	* Verifies ESR function call result and writes a ERROR log message
	* @param e error code
	* @param function name of function invoked
	* @return true if no error happened, false otherwise
	*/
	bool verify(esr::error e, esr::function_name function);

#endif
}

#endif