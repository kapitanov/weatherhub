#ifndef _ESR_IO_h
#define _ESR_IO_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "esr_conf.h"
#include "esr_errors.h"
#include <stdarg.h>

/*
* Log formatting placeholders:
* ============================
* %%	'%' literal		
* %c	const char					A character
* %s	const char*					Null-terminated string
* %ps	const __FlashStringHelper*	PROGMEM null-terminated string
* %e	const esr::error			A ESR error code name
* %b	const int8_t				Decimal 1-byte signed integer
* %ub	const uint8_t				Decimal 1-byte unsigned integer
* %d	const int16_t				Decimal 2-byte signed integer
* %ud	const uint16_t				Decimal 2-byte unsigned integer
* %l	const int32_t*				Decimal 4-byte signed integer
* %ul	const uint32_t*				Decimal 4-byte unsigned integer
* %f	const float*				Floating-point number
* %xb	const uint8_t				Hexadecimal 1-byte unsigned integer
* %xd	const uint16_t				Hexadecimal 2-byte unsigned integer
* %xl	const uint32_t*				Hexadecimal 4-byte unsigned integer
*/

namespace esr
{
	/**
	*	Log levels enumeration
	**/
	enum log_level
	{
		/**
		*	DEBUG log level
		**/
		LOG_DEBUG,

		/**
		*	INFORMATION log level
		**/
		LOG_INFO,

		/**
		*	ERROR log level
		**/
		LOG_ERROR,

		/**
		* Disables logging. Cannot be used as an argument of esr::log(), 
		* this option is to be used only with esr::log_init()
		*/
		LOG_DISABLED
	};

	/**
	* Default lowest allowed log level
	*/
	const log_level MAX_LOG_LEVEL = __ESR_MAX_LOG_LEVEL;

	/**
	* Initializes serial logging
	* @param stream a destination stream (ex. Serial)
	* @param max_level lowest allowed log level
	* @return error code
	*/
	error log_init(Print& stream, log_level max_level = MAX_LOG_LEVEL);

#ifdef __ESR_ENABLE_NON_PROGMEM_FORMATTING
	/**
	* Prints a formatted message into log
	* @param level logging level
	* @param format format string
	* @param ... format arguments
	* @return error code
	*/
	error log(log_level level, const char* format, ...);

#endif

	/**
	* Prints a formatted message into log
	* @param level logging level
	* @param format format string (flash memory)
	* @param ... format arguments
	* @return error code
	*/
	error log(log_level level, const __FlashStringHelper* format, ...);

	/**
	* An internal version of logging function. Might be disabled by defines
	* @param format format string (flash memory)
	* @param ... format arguments
	*/
	void log_d(const __FlashStringHelper* format, ...);
}

#endif