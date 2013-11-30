#ifndef _ESR_KERNEL_h
#define _ESR_KERNEL_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "esr_conf.h"
#include "esr_errors.h"

namespace esr
{
	/**
	* Defines maximum allowed amount of threads
	*/
	const uint8_t MAX_THREADS = __ESR_MAX_THREADS;

	/**
	* Defines maximum allowed message queue size of a thread
	*/
	const uint8_t MAX_THREAD_QUEUE = __ESR_MAX_THREAD_QUEUE;


	/**
	* Message code
	*/
	typedef uint8_t message;

	/**
	* Empty message code
	*/
	const message MSG_NONE  = 0;

	/**
	* Idle loop message code
	*/
	const message MSG_IDLE  = 1;

	/**
	* Timer message code
	*/
	const message MSG_TIMER = 2;

	/**
	* Thread termination message
	*/
	const message MSG_FINALIZE = 3;

	/**
	* Base message code for user-defined messages
	*/
	const message MSG_USER = 4;


	/**
	* Thread identifier
	*/
	typedef uint8_t thread_id;

	/**
	* Current thread identifier
	*/
	const thread_id THREAD_CURRENT = 255;

	/**
	* Thread option flags
	*/
	enum thread_flags
	{
		/**
		* Indicates if a thread is alive. 
		* If a thread is not alive it might be reused by upcoming esr::begin_thread() calls.
		*/
		THREAD_ALIVE = 1 << 0,

		/**
		* Indicates if a thread will receive esr::MSG_IDLE messages during scheduler loop.
		*/
		THREAD_IDLE_LOOP = 1 << 1,

		/**
		* Indicates if a thread will be invoked on a timer.
		* If THREAD_ENABLE_TIMER is set and THREAD_REPEAT_TIMER is not set 
		* then THREAD_ENABLE_TIMER will be cleared after one timer call.
		*/
		THREAD_ENABLE_TIMER = 1 << 2,

		/**
		* Indicates if a thread will be invoked on a timer more than once.
		* If THREAD_ENABLE_TIMER is set and THREAD_REPEAT_TIMER is not set 
		* then THREAD_ENABLE_TIMER will be cleared after one timer call.
		* If THREAD_ENABLE_TIMER is not set then this flag has no effect.
		*/
		THREAD_REPEAT_TIMER = 1 << 3,

		/**
		* Indicates if a timer will fire at the moment of start.
		* If this flag is set then timer will fire the first time at the moment of esr::set_timer() call.
		* Otherwise, timer will fire the first time after one timer period counting from the moment
		* of esr::set_timer() call.
		* If THREAD_ENABLE_TIMER is not set then this flag has no effect.
		*/
		THREAD_IMMEDIATE_TIMER = 1 << 4
	};

	/**
	* Timer period value
	*/
	typedef uint32_t timer_period;

	/**
	* Thread worker function type
	*/
	typedef void (*thread_func)(message msg);


	/**
	* Starts new thread
	* @param thread thread entry point
	* @param [out] an identifier of a new thread
	* @return error code
	*/
	error begin_thread(thread_func thread, thread_id& id);

	/**
	* Gets a thread flag
	* @param id thread identifier
	* @param flag target thread flag
	* @param value [out] a value indicating if flag is set or not
	* @return error code
	*/
	error get_thread_flag(thread_id id, thread_flags flag, bool& value);

	/**
	* Sets a thread flag
	* @param id thread identifier
	* @param flag target thread flag
	* @param value true to set the flag, false to clear it
	* @return error code
	*/
	error set_thread_flag(thread_id id, thread_flags flag, bool value);

	/**
	* Terminates thread
	* @param id thread identifier
	* @return error code
	*/
	error kill_thread(thread_id id);	

	/**
	* Puts a message into thread's message queue
	* @param id thread identifier
	* @param msg message code
	* @return error code
	*/
	error post_message(thread_id id, message msg);

	/**
	* Starts timer for the thread. Timer's behavior depends on THREAD_REPEAT_TIMER flag.
	* @param id thread identifier
	* @param period timer period in milliseconds
	* @return error code
	*/
	error set_timer_ms(thread_id id, timer_period period);	

	/**
	* Changes timer period for the thread. 
	* If THREAD_REPEAT_TIMER flag is not set and timer has already fired then this function will have no effect 
	* (since THREAD_ENABLE_TIMER flag will be cleared at that moment).
	* @param id thread identifier
	* @param period timer period in milliseconds
	* @return error code
	*/
	error change_timer_ms(thread_id id, timer_period period);	

	/**
	* Resets thread timer
	* @param id thread identifier
	* @return error code
	*/
	error clear_timer(thread_id id);

	/**
	* Gets an identifier of the current thread
	* @param id [out] current thread identifier
	* @return error code
	*/
	error get_current_thread_id(thread_id& id);

	/**
	* Runs one iteration of scheduler loop
	*/
	void run_cycle();
}

#endif