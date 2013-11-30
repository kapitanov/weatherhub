#include <esr.h>

const esr::message MSG_LED_ON = esr::MSG_USER + 1;
const esr::message MSG_LED_OFF = esr::MSG_USER + 2;

bool is_on;
uint32_t interval = 500;
esr::thread_id ctrl_thread_id, led_thread_id, uart_thread_id;

void led_thread(esr::message msg)
{
	switch (msg)
	{
	case MSG_LED_ON:
		digitalWrite(13, HIGH);
		break;

	case MSG_LED_OFF:
		digitalWrite(13, LOW);
		break;
	}
}

void ctrl_thread(esr::message msg)
{
	switch (msg)
	{
	case esr::MSG_TIMER:
		esr::verify(esr::post_message(led_thread_id, is_on ? MSG_LED_OFF : MSG_LED_ON), esr::FUNC_POST_MESSAGE);
		is_on = !is_on;

		if(!is_on)
		{
			switch (interval)
			{
			case 500:
				interval = 250;
				break;
			case 250:
				interval = 125;
				break;
			case 125:
				interval = 62;
				break;
			case 62:
				interval = 500;
				break;
			}

			esr::verify(esr::change_timer_ms(esr::THREAD_CURRENT, interval), esr::FUNC_CHANGE_TIMER_MS);
		}
		break;
	}
}

void uart_thread(esr::message msg)
{
	switch (msg)
	{
	case esr::MSG_IDLE:
		if(Serial.available() > 0)
		{
			char c = Serial.read();
			switch(c)
			{
			case '+':
				Serial.println(F("esr::set_timer_ms(ctrl_thread_id, 500)"));
				esr::verify(esr::set_timer_ms(ctrl_thread_id, 500), esr::FUNC_POST_MESSAGE);
				break;
			case '-':
				Serial.println(F("esr::clear_timer(ctrl_thread_id)"));
				esr::verify(esr::clear_timer(ctrl_thread_id), esr::FUNC_POST_MESSAGE);
				break;
			case 'L':
				Serial.println(F("esr::post_message(led_thread_id, MSG_LED_ON)"));
				esr::verify(esr::post_message(led_thread_id, MSG_LED_ON), esr::FUNC_POST_MESSAGE);
				break;
			case 'D':
				Serial.println(F("esr::post_message(led_thread_id, MSG_LED_OFF)"));
				esr::verify(esr::post_message(led_thread_id, MSG_LED_OFF), esr::FUNC_POST_MESSAGE);
				break;
			default:
				Serial.println(F("WTF?"));
				break;
			}
		}
		break;
	}
}

void setup()
{
	Serial.begin(57600);

	esr::log_init(Serial);

	int32_t s32 = 127;
	uint32_t u32 = 127;
	float f = 1.25;

#ifdef __ESR_ENABLE_NON_PROGMEM_FORMATTING 
	{
		esr::log(esr::LOG_DEBUG, "=== esr::log() - inmem version ===");

		esr::log(esr::LOG_DEBUG, "No args");
		esr::log(esr::LOG_DEBUG, "uint8 <%ub>", (uint8_t)127);
		esr::log(esr::LOG_DEBUG, "int8 <%b>", (int8_t)127);

		esr::log(esr::LOG_DEBUG, "uint16 <%ud>", (uint16_t)127);
		esr::log(esr::LOG_DEBUG, "int16 <%d>", (int16_t)127);


		esr::log(esr::LOG_DEBUG, "uint32 <%l>", &u32);
		esr::log(esr::LOG_DEBUG, "int32 <%ul>", &s32);

		esr::log(esr::LOG_DEBUG, "char <%c>", 'z');
		esr::log(esr::LOG_DEBUG, "str <%s>", "name");
		esr::log(esr::LOG_DEBUG, "pstr <%ps>", F("name"));


		esr::log(esr::LOG_DEBUG, "float <%f>", &f);
	}
#endif

	{
		esr::log(esr::LOG_DEBUG, F("=== esr::log() - progmem version ==="));

		esr::log(esr::LOG_DEBUG, F("No args"));
		esr::log(esr::LOG_DEBUG, F("uint8 <%ub>"), (uint8_t)127);
		esr::log(esr::LOG_DEBUG, F("int8 <%b>"), (int8_t)127);

		esr::log(esr::LOG_DEBUG, F("uint16 <%ud>"), (uint16_t)127);
		esr::log(esr::LOG_DEBUG, F("int16 <%d>"), (int16_t)127);

		esr::log(esr::LOG_DEBUG, F("uint32 <%ul>"), &u32);
		esr::log(esr::LOG_DEBUG, F("int32 <%l>"), &s32);

		esr::log(esr::LOG_DEBUG, F("char <%c>"), 'z');
		esr::log(esr::LOG_DEBUG, F("str <%s>"), "name");
		esr::log(esr::LOG_DEBUG, F("pstr <$ps>"), F("name"));


		esr::log(esr::LOG_DEBUG, F("float <%f>"), &f);

		esr::log_d(F("Internal logging example"));
	}

	pinMode(13, OUTPUT);

	esr::verify(esr::begin_thread(ctrl_thread, ctrl_thread_id), esr::FUNC_BEGIN_THREAD);
	esr::verify(esr::set_thread_flag(ctrl_thread_id, esr::THREAD_IDLE_LOOP, false), esr::FUNC_SET_THREAD_FLAG);
	esr::verify(esr::set_timer_ms(ctrl_thread_id, 500), esr::FUNC_SET_TIMER_MS);

	esr::verify(esr::begin_thread(led_thread, led_thread_id), esr::FUNC_BEGIN_THREAD);
	esr::verify(esr::set_thread_flag(led_thread_id, esr::THREAD_IDLE_LOOP, false), esr::FUNC_SET_THREAD_FLAG);

	esr::verify(esr::begin_thread(uart_thread, uart_thread_id), esr::FUNC_BEGIN_THREAD);
}

void loop()
{
	esr::run_cycle();
}
