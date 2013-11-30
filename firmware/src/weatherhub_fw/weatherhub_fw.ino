#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "extsensor.h"
#include "input.h"
#include "intsensor.h"
#include "backlight.h"
#include "gui.h"
#include "globals.h"
#include "settings.h"
#include <esr.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_GFX.h>
#include <DHT.h>d

using namespace esr;

SoftwareSerial	extsensor::uart(A5, A4);

void setup()
{
	// Setup logging
	Serial.begin(57600);
	log_init(Serial);
	log(LOG_INFO, F("APP\tstartup"));

	// Start threads
	begin_thread(gui::thread_func, gui::thread);
	set_thread_flag(gui::thread, THREAD_IDLE_LOOP, false);

	begin_thread(backlight::thread_func, backlight::thread);
	set_thread_flag(backlight::thread, THREAD_IDLE_LOOP, false);

	begin_thread(intsensor::thread_func, intsensor::thread);
	set_thread_flag(intsensor::thread, THREAD_IDLE_LOOP, false);

	begin_thread(input::thread_func, input::thread);
	begin_thread(extsensor::thread_func, extsensor::thread);

	settings::init();
		
	// Post initialization messages
	post_message(gui::thread, MSG_GUI_INIT);
	post_message(backlight::thread, MSG_BL_INIT);
	post_message(intsensor::thread, MSG_INTSENSOR_INIT);
	post_message(input::thread, MSG_INPUT_INIT);
	post_message(extsensor::thread, MSG_EXTSENSOR_INIT);
}

void loop()
{
	// Run scheduler loop
	run_cycle();
}

