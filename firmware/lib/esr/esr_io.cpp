#include "esr_io.h"

Print* _log_stream = NULL;
esr::log_level _max_log_level = esr::LOG_DISABLED;

/**
* Data type for format placeholder
*/
enum datatype
{
	DATA_CHAR,
	DATA_STR,
	DATA_PGM_STR,
	DATA_INT8,
	DATA_UINT8,
	DATA_INT16,
	DATA_UINT16,
	DATA_INT32,
	DATA_UINT32,
	DATA_FLOAT,
	DATA_DOUBLE
};

/**
* Data format for format placeholder
*/
enum dataformat
{
	FORMAT_DEFAULT,
	FORMAT_BIN,
	FORMAT_DEC,
	FORMAT_HEX,
};

/**
* Format placeholder
*/
struct format_placeholder
{
	/**
	* Data type
	*/
	datatype type;

	/**
	* Data format. Must be applicable to @type
	*/
	dataformat format;
};

/**
* Initializes serial logging
* @param stream a destination stream (ex. Serial)
* @param max_level lowest allowed log level
* @return error code
*/
esr::error esr::log_init(Print& stream, esr::log_level max_level)
{
	_log_stream = &stream;
	_max_log_level = max_level;

	return esr::E_OK;
}

/**
* Prints log level-based header
* @param level log level
*/
void print_log_header(esr::log_level level)
{
	switch (level)
	{
	case esr::LOG_DEBUG:
		_log_stream->print(F("DEBUG\t"));
		break;
	case esr::LOG_INFO:
		_log_stream->print(F("INFRM\t"));
		break;
	case esr::LOG_ERROR:
		_log_stream->print(F("ERROR\t"));
		break;
	}
}

template<typename T>
const T get_argument(va_list& args)
{
	int value = va_arg(args, int);
	return reinterpret_cast<T>(value);
}

template<>
const bool get_argument(va_list& args)
{
	int value = va_arg(args, int);
	return static_cast<bool>(value);
}

template<>
const int8_t get_argument(va_list& args)
{
	int value = va_arg(args, int);
	return static_cast<int8_t>(value);
}

template<>
const int16_t get_argument(va_list& args)
{
	int value = va_arg(args, long);
	return static_cast<int16_t>(value);
}

template<>
const char get_argument(va_list& args)
{
	int value = va_arg(args, int);
	return static_cast<char>(value);
}

enum print_placeholder_result
{
	RESULT_ERROR,
	RESULT_ONE_CHAR,
	RESULT_TWO_CHARS
};

/**
* Writes a format message placeholder into log stream
* @param c  format placeholder code
* @param extra format placeholder extra code
* @params args  format arguments list
* @return result code
**/
print_placeholder_result print_format_placeholder(const char c, const char extra, va_list& args) 
{
	switch(c)
	{
	case '\0':
		// %\0
		// End of string
		break;

	case '%':
		// %%
		// '%' literal
		_log_stream->print('%');
		break;

	case 'c':
		// %c
		// Character
		{
			const char s = get_argument<char>(args);
			_log_stream->print(s);
		}
		break;

	case 's':
		// %s
		// Null-terminated string
		{
			const char* s = get_argument<char*>(args);
			_log_stream->print(s);
		}
		break;

	case 'p':
		// PROGMEM null-terminated string
		switch (extra)
		{
		case 's':
			// %ps
			{
				const __FlashStringHelper* s = get_argument<__FlashStringHelper*>(args);
				_log_stream->print(s);
			}
			break;

		default:
			return RESULT_ERROR;
		}
		return RESULT_TWO_CHARS;


	case 'e':
		// %e
		// ESR error code
		{
			const int8_t x = get_argument<int8_t>(args);
#ifdef __ESR_ENABLE_ERROR_FORMATTING
			const __FlashStringHelper* s = esr::get_error_name(static_cast<esr::error>(x));
			_log_stream->print(s);
#else
			_log_stream->print('E');
			_log_stream->print('_');
			_log_stream->print(x, HEX);
#endif
		}
		break;

	case 'b':
		// %b
		// signed 1-byte decimal integer
		{
			const int8_t x = get_argument<int8_t>(args);
			_log_stream->print(x, DEC);
		}
		break;

	case 'd':
		// %d
		// signed 2-byte decimal integer
		{
			const int16_t x = get_argument<int16_t>(args);
			_log_stream->print(x, DEC);
		}
		break;

	case 'l':
		// %l
		// signed 4-byte decimal integer
		{
			const int32_t x = *get_argument<int32_t*>(args);
			_log_stream->print(x, DEC);
		}
		break;

	case 'f':
		// %f
		// floating-point number
		{
			const float s = *get_argument<float*>(args);
			_log_stream->print(s);
		}
		break;

	case 'u':
		// Unsigned decimal integer
		switch (extra)	
		{
		case 'b':
			// %ub
			// unsigned 1-byte decimal integer
			{
				const uint8_t x = static_cast<uint8_t>(get_argument<int8_t>(args));
				_log_stream->print(x, DEC);
			}
			break;

		case 'd':
			// %ud
			// unsigned 2-byte decimal integer
			{
				const uint16_t x = static_cast<uint16_t>(get_argument<int16_t>(args));
				_log_stream->print(x, DEC);
			}
			break;

		case 'l':
			// %ul
			// unsigned 4-byte decimal integer
			{
				const uint32_t x = *get_argument<uint32_t*>(args);
				_log_stream->print(x, DEC);
			}
			break;

		default:
			return RESULT_ERROR;
		}
		return RESULT_TWO_CHARS;

	case 'x':
		// Hexadecimal integer
		switch (extra)
		{
		case 'b':
			// %xb
			// unsigned 1-byte decimal integer
			{
				const uint8_t x = static_cast<uint8_t>(get_argument<int8_t>(args));
				_log_stream->print(x, HEX);
			}
			break;

		case 'd':
			// %xd
			// unsigned 2-byte decimal integer
			{
				const uint16_t x = static_cast<uint16_t>(get_argument<int16_t>(args));
				_log_stream->print(x, HEX);
			}
			break;

		case 'l':
			// %xl
			// unsigned 4-byte decimal integer
			{
				const uint32_t x = *get_argument<uint32_t*>(args);
				_log_stream->print(x, HEX);
			}
			break;

		default:
			return RESULT_ERROR;
		}

		return RESULT_TWO_CHARS;
	}

	return RESULT_ONE_CHAR;
}

#ifdef __ESR_ENABLE_NON_PROGMEM_FORMATTING

/**
* Prints a formatted message into log
* @param level logging level
* @param format format string
* @param ... format arguments
* @return error code
*/
esr::error esr::log(esr::log_level level, const char* format, ...)
{
	// Check if logging is set up
	if(_log_stream == NULL)
	{
		return esr::E_LOG_CONFIGURATION_IS_INCORRECT;
	}

	// Skip if log level is disabled
	if(_max_log_level > level)
	{
		return esr::E_OK;
	}

	va_list args;
	va_start(args, format);

	print_log_header(level);

	// Print formatted message
	--format;
	while(true)
	{
		++format;
		char c = *format;
		if(c == '\0')
		{
			break;
		}

		if (c == '%') 
		{
			++format;
			c = *format;
			char e = c != '\0' 
				? *(format + 1)
				: '\0';
			switch(print_format_placeholder(c, e, args))
			{
			case RESULT_ERROR:
				return false;

			case RESULT_ONE_CHAR:
				break;

			case RESULT_TWO_CHARS:
				++format;
				break;
			}
		}
		else
		{
			_log_stream->print(c);
		}
	}

	_log_stream->println();

	va_end(args);
	return esr::E_OK;
}

#endif

/**
* Prints formatted message into log without appending log message header
* @param format format string (flash memory)
* @param ... format arguments
*/
bool try_print(const __FlashStringHelper* format, va_list& args)
{
	// Print formatted message
	uint16_t address = reinterpret_cast<uint16_t>(format);
	--address;
	while(true)
	{
		++address;
		char c = pgm_read_byte(address);
		if(c == '\0')
		{
			break;
		}

		if (c == '%') 
		{
			++address;
			c = pgm_read_byte(address);
			char e = c != '\0' 
				? pgm_read_byte(address + 1)
				: '\0';
			switch(print_format_placeholder(c, e, args))
			{
			case RESULT_ERROR:
				return false;

			case RESULT_ONE_CHAR:
				break;

			case RESULT_TWO_CHARS:
				++address;
				break;
			}
		}
		else
		{
			_log_stream->print(c);
		}
	}

	_log_stream->println();
	return true;
}

/**
* Prints a formatted message into log
* @param level logging level
* @param format format string (flash memory)
* @param ... format arguments
* @return error code
*/
esr::error esr::log(esr::log_level level, const __FlashStringHelper* format, ...)
{
	// Check if logging is set up
	if(_log_stream == NULL)
	{
		return esr::E_LOG_CONFIGURATION_IS_INCORRECT;
	}

	// Skip if log level is disabled
	if(_max_log_level > level)
	{
		return esr::E_OK;
	}

	va_list args;
	va_start(args, format);

	print_log_header(level);

	bool success = try_print(format, args);

	va_end(args);
	return success ? esr::E_OK : esr::E_INCORRECT_FORMAT;
}

/**
* An internal version of logging function. Might be disabled by defines
* @param format format string (flash memory)
* @param ... format arguments
*/
void esr::log_d(const __FlashStringHelper* format, ...)
{
#ifdef __ESR_ENABLE_KERNEL_LOGGING

	// Check if logging is set up
	if(_log_stream == NULL)
	{
		return;
	}

	// Print message

	_log_stream->print(F("KERNL\t"));

	va_list args;
	va_start(args, format);

	try_print(format, args);

	va_end(args);

#endif
}