#ifndef _ESR_CONF_h
#define _ESR_CONF_h

/*
* Define default max logging level
*/
#ifndef __ESR_MAX_LOG_LEVEL
#define __ESR_MAX_LOG_LEVEL esr::LOG_DEBUG
#endif

/*
* Define max thread slots count
*/
#ifndef __ESR_MAX_THREADS
#define __ESR_MAX_THREADS 8
#endif

/*
* Define max thread message queue size
*/
#ifndef __ESR_MAX_THREAD_QUEUE
#define __ESR_MAX_THREAD_QUEUE 4
#endif

/**
* Enable non-PROGMEM version of esr::log()
*/
#define __ESR_ENABLE_NON_PROGMEM_FORMATTING

/**
* Enable kernel logging via esr::log_d()
*/
#define __ESR_ENABLE_KERNEL_LOGGING

/**
* Enable esr::verify(), esr::get_error_name() and esr::format_error()
*/
// #define __ESR_ENABLE_ERROR_FORMATTING

#endif