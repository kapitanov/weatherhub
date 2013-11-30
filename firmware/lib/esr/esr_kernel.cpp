#include "esr_kernel.h"
#include "esr_io.h"

struct thread_slot
{
	esr::thread_func func;
	esr::thread_flags flags;
	esr::message queue[esr::MAX_THREAD_QUEUE];
	uint8_t queue_tail;
	esr::timer_period period;
	esr::timer_period last_invokation;

	__inline__ bool has_flag(esr::thread_flags flag) const
	{
		return (flags & flag) != 0;
	}

	__inline__ void set_flag(esr::thread_flags flag)
	{
		flags = static_cast<esr::thread_flags>(flags | flag);
	}

	__inline__ void clear_flag(esr::thread_flags flag)
	{
		flags = static_cast<esr::thread_flags>(flags & ~flag);
	}
};

thread_slot _threads[esr::MAX_THREADS];
bool _is_in_thread;
esr::thread_id _current_thread_id;

/**
* Starts new thread
* @param thread thread entry point
* @param [out] an identifier of a new thread
* @return error code
*/
esr::error esr::begin_thread(esr::thread_func thread, esr::thread_id& id)
{
	// Allocate an empty thread slot

	// Search for the first empty thread slot
	for(uint8_t i = 0; i < esr::MAX_THREADS; ++i)
	{
		thread_slot& slot = _threads[i];
		// If thread in the slot is not alive
		if(!slot.has_flag(esr::THREAD_ALIVE))
		{
			// Take the slot
			id = i;

			slot.func = thread;

			// Reset thread flags: 
			//  * thread is alive, 
			//  * idle loop enabled, 
			//  * repeat timer enabled, 
			//  * timer disabled
			slot.set_flag(esr::THREAD_ALIVE);
			slot.set_flag(esr::THREAD_IDLE_LOOP);
			slot.set_flag(esr::THREAD_REPEAT_TIMER);
			slot.clear_flag(esr::THREAD_ENABLE_TIMER);

			// Clear message queue
			for(uint8_t j = 0; j < esr::MAX_THREAD_QUEUE; ++j)
			{
				slot.queue[j] = esr::MSG_NONE;
			}

#ifdef __ESR_ENABLE_KERNEL_LOGGING
			//esr::log_d(F("begin_thread: E_OK"));
#endif
			return esr::E_OK;
		}
	}

	// All thread slots are taken, unable to create a new thread
#ifdef __ESR_ENABLE_KERNEL_LOGGING
	// esr::log_d(F("begin_thread: E_NO_FREE_THREAD_SLOTS"));
#endif
	return esr::E_NO_FREE_THREAD_SLOTS;
}

/**
* Gets a thread slot
* @param id thread identifier
* @param value [out] an output value
* @return error code
*/
esr::error get_thread_slot(esr::thread_id id, thread_slot*& value)
{
	// If id == esr::THREAD_CURRENT then take current thread's identifier
	if(id == esr::THREAD_CURRENT)
	{
		// Check if function is called from a code within scheduler loop
		// This is required only if id == esr::THREAD_CURRENT
		if(!_is_in_thread)
		{
			return esr::E_NOT_IN_SCHEDULER;
		}

		id = _current_thread_id;
	}

	// Check if thread identifier is within an allowed range
	if(id >= esr::MAX_THREADS)
	{
		return esr::E_WRONG_THREAD;
	}

	// Check if the specified thread is alive
	thread_slot& slot = _threads[id];
	if(!slot.has_flag(esr::THREAD_ALIVE))
	{
		return esr::E_WRONG_THREAD;
	}

	value = &slot;
	return esr::E_OK;
}

/**
* Gets a thread flag
* @param id thread identifier
* @param flag target thread flag
* @param value [out] a value indicating if flag is set or not
* @return error code
*/
esr::error esr::get_thread_flag(esr::thread_id id, esr::thread_flags flag, bool& value)
{
	// Retreive thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Return thread flag
	value = slot_ptr->has_flag(flag);
	return esr::E_OK;
}

/**
* Sets a thread flag
* @param id thread identifier
* @param flag target thread flag
* @param value true to set the flag, false to clear it
* @return error code
*/
esr::error esr::set_thread_flag(esr::thread_id id, esr::thread_flags flag, bool value)
{
	// Check if flag is not readonly
	if(flag == esr::THREAD_ALIVE)
	{
		return esr::E_WRONG_FLAG;
	}

	// Retreive thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Set or clear the thread flag according to value parameter
	if(value)
	{
		slot_ptr->set_flag(flag);
	}
	else
	{
		slot_ptr->clear_flag(flag);
	}
	return esr::E_OK;
}

/**
* Terminates thread
* @param id thread identifier
* @return error code
*/
esr::error esr::kill_thread(esr::thread_id id)
{
	// Retreive thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Let the thread to finalize by invoking it with MSG_FINALIZE message
	slot_ptr->func(esr::MSG_FINALIZE);

	// Mark the thread as a dead one	
	slot_ptr->clear_flag(esr::THREAD_ALIVE);

#ifdef __ESR_ENABLE_KERNEL_LOGGING
	// esr::log_d(F("kill_thread 0x%xd E_OK"), id);
#endif
	return esr::E_OK;
}

/**
* Puts a message into thread's message queue
* @param id thread identifier
* @param msg message code
* @return error code
*/
esr::error esr::post_message(esr::thread_id id, esr::message msg)
{
	// Check if message can be posted by client code
	if(msg ==  esr::MSG_NONE || 
		msg ==  esr::MSG_IDLE || 
		msg ==  esr::MSG_TIMER)
	{
		// MSG_NONE, MSG_IDLE, MSG_TIMER are system defined messages, 
		// they cannot be posted into message queue
		return esr::E_WRONG_MESSAGE;
	}

	// Retreive thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// If queue element at queue_tail is taken then move tail
	if(slot_ptr->queue[slot_ptr->queue_tail] != esr::MSG_NONE)
	{
		// If tail is at the last possible position then message queue is full
		if(slot_ptr->queue_tail >= esr::MAX_THREAD_QUEUE)
		{
			return esr::E_MESSAGE_QUEUE_IS_FULL;
		}

		++slot_ptr->queue_tail;
	}

	// Put message into the slot
	slot_ptr->queue[slot_ptr->queue_tail] = msg;
	return esr::E_OK;
}

/**
* Fires timer for the thread slot and updates thread flags
* @param slot thread slot
* @param time current time
*/
void fire_timer(thread_slot& slot, esr::timer_period time)
{
	// Invoke thread with MSG_TIMER message
	slot.func(esr::MSG_TIMER);

	// Update last_invokation time
	slot.last_invokation = time;

	// If THREAD_REPEAT_TIMER flag is not set then clear the THREAD_ENABLE_TIMER flag
	if(!slot.has_flag(esr::THREAD_REPEAT_TIMER))
	{
		slot.clear_flag(esr::THREAD_ENABLE_TIMER);
	}
}

/**
* Starts timer for the thread. Timer's behavior depends on THREAD_REPEAT_TIMER flag.
* @param id thread identifier
* @param period timer period in milliseconds
* @return error code
*/
esr::error esr::set_timer_ms(esr::thread_id id, esr::timer_period period)
{
	// Validate timer period	
	if(period <= 0)
	{
		return esr::E_WRONG_PERIOD;
	}

	// Retrieve thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Enable timer for the thread
	slot_ptr->set_flag(esr::THREAD_ENABLE_TIMER);
	slot_ptr->period = period;
	// The timer will fire at current_time + period
	esr::timer_period time = millis();
	slot_ptr->last_invokation = time;

	// If THREAD_IMMEDIATE_TIMER flag is set the fire timer immediately
	if(slot_ptr->has_flag(esr::THREAD_IMMEDIATE_TIMER))
	{
		fire_timer(*slot_ptr, time);
	}

	return esr::E_OK;
}

/**
* Changes timer period for the thread. 
* If THREAD_REPEAT_TIMER flag is not set and timer has already fired then this function will have no effect.
* @param id thread identifier
* @param period timer period in milliseconds
* @return error code
*/
esr::error esr::change_timer_ms(esr::thread_id id, esr::timer_period period)
{
	// Validate timer period	
	if(period <= 0)
	{
		return esr::E_WRONG_PERIOD;
	}

	// Retrieve thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Ensure thread has an active timer
	if(!slot_ptr->has_flag(esr::THREAD_ENABLE_TIMER))
	{
		return esr::E_TIMER_NOT_DEFINED;
	}

	// Change timer period
	slot_ptr->period = period;
	return esr::E_OK;
}

/**
* Resets thread timer
* @param id thread identifier
* @return error code
*/
esr::error esr::clear_timer(esr::thread_id id)
{
	// Retrieve thread slot if possible
	thread_slot* slot_ptr = NULL;
	esr::error e = get_thread_slot(id, slot_ptr);
	if(e != esr::E_OK)
	{
		return e;
	}

	// Ensure thread has an active timer
	if(!slot_ptr->has_flag(esr::THREAD_ENABLE_TIMER))
	{
		return esr::E_TIMER_NOT_DEFINED;
	}

	// Clear the THREAD_ENABLE_TIMER flag
	slot_ptr->clear_flag(esr::THREAD_ENABLE_TIMER);
	return esr::E_OK;
}

/**
* Gets an identifier of the current thread
* @param id [out] current thread identifier
* @return error code
*/
esr::error esr::get_current_thread_id(esr::thread_id& id)
{
	if(!_is_in_thread)
	{
		// Not in scheduler thread
		return esr::E_NOT_IN_SCHEDULER;
	}

	id = _current_thread_id;
	return esr::E_OK;
}

/**
* Tries to peek a message from the message queue and process it
* @param thread target thread
* @return true if a message has been processed, false if the message queue is empty
*/
bool try_process_message(thread_slot& thread)
{
	// Pick a message
	if(thread.queue[0] == esr::MSG_NONE)
	{
		return false;
	}

	// Process the message
	thread.func(thread.queue[0]);

	// Move elements in the queue
	for(uint8_t i = thread.queue_tail; i > 0; --i)
	{
		thread.queue[i - 1] = thread.queue[i];
	}

	thread.queue[thread.queue_tail] = esr::MSG_NONE;

	// Move queue_tail if possible
	if(thread.queue_tail > 0)
	{
		--thread.queue_tail;
	}

	return true;
}

/**
* Runs one iteration of scheduler loop
*/
void esr::run_cycle()
{
	_is_in_thread = true;

	// Loop thought thread slots and run each of them
	for(uint8_t i = 0; i < esr::MAX_THREADS; ++i)
	{
		thread_slot& thread = _threads[i];

		// Skip dead threads
		if(!thread.has_flag(esr::THREAD_ALIVE))
		{
			continue;
		}

		_current_thread_id = i;

		// Try peek a message from the queue and invoke thread
		try_process_message(thread);

		// If THREAD_IDLE_LOOP flag is set then populate a MSG_IDLE message
		if(thread.has_flag(esr::THREAD_IDLE_LOOP))
		{
			thread.func(esr::MSG_IDLE);
		}

		// If thread has a timer defined then try fire it
		if(thread.has_flag(esr::THREAD_ENABLE_TIMER))
		{
			esr::timer_period time = millis();
			esr::timer_period interval = time - thread.last_invokation;

			// Fire timer if at least thread.period has elapsed
			if(interval >= thread.period)
			{
				fire_timer(thread, time);
			}
		}
	}

	_is_in_thread = false;
}