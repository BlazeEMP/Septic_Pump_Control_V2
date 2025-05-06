// Best changes to make would be to create a flag for if the pump has ran, and also a digital input to see if the button to run the pump has been pressed (this is the button going to the OR gate of 2 transistors) and throw the flag up as well. Handle differently from resetting avgDistance to 0 ideally

// Add digital input to read if the alarm is on, until the alarm is turned off there should be an error message maintained !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!_---------------------------------------------------------------

#include <LiquidCrystal_I2C.h>
#include <Watchdog.h>

Watchdog watchdog;  // bring watchdog class object in

// section good for cross platform work when using print and write functions or want them to be changed
// #if defined(ARDUINO) && ARDUINO >= 100 this if is enabled for Arduino UNO
// #define printByte(args)  write(args); // keep if wanting to use lcd.printByte instead of lcd.write
// #else
// #define printByte(args)  print(args,BYTE);
// #endif

#define upDistance 3      // raise the number of cm to trigger (trigger with lower water level)
#define downDistance 4    // lower the number of cm to trigger (trigger with higher water level)
#define echo 7            // input signal from ultrasonic sensor
#define trig 8            // output pulses for ultrasonic sensor
#define alarm 11          // pin for alarm to sound (go to 555 flip flop configuration for TRIGGER pin)
#define runPumpRelay 12   // output signal pin to digital pin 2 for setting pump on
#define watchdogRelay 13  // output signal pin to indicate reset (stays high going into inverter until reset)
// do not use analog 4 and 5, used for SDA SCL on I2C display

LiquidCrystal_I2C lcd(0x27, 16, 2);

int avgDistance = 0;          // set average distance to global for use in multiple functions
int heightSetting = 25;       // set default trigger height for water level, keep global to adjust with one function (adjustLevel) and read the value for comparison in another (setupDisplay, refreshDisplay, and autoRunPump)

int highReadingWarning = 40;  // if the sensor reads over 32cm it is getting reflections from bubbles that slow down the speed of sound, if this lasts too long (track while looping through sensor readings) the alarm should sound
int highReadingTracker = 0;
int highReadingLimit = 400;
int goodReadingTracker = 0;

// alarm will also set off if arduino resets, also triggering relay reset for modules, does not turn off alarm when no longer true, manual button to reset latch must be pushed and all bools false
bool checkSensor = false;  // fails on reporting too many zeros
bool checkRelay = false;   // current draw sense required
bool checkWater = false;   // water sensor input required

void setup() {
	// enable watchdog with timer variable in seconds at the end, 1,2, or 4 works well for the application. If water levels can change quickly in the system consider evaluating all functions for new operating speed (less loops, more optimization) and watchdog reset positions
	watchdog.enable(Watchdog::TIMEOUT_4S);

	pinMode(watchdogRelay, OUTPUT);     // initialize "watchdog" pin to high
	digitalWrite(watchdogRelay, HIGH);  // do not overwrite unless using custom code to reset on program hangups for power relay on board, may be set low in this project to reset the power manually but right now is tracking power-on state of arduino, watchdog handles the reset itself

	pinMode(alarm, OUTPUT);  // alarm trigger signal initialization, routes to NOT gate and 555 as a latch circuit
	digitalWrite(alarm, LOW);

	pinMode(runPumpRelay, OUTPUT);  // pin for relay to run pump

	// pins for adjusting distance to trigger relay
	pinMode(upDistance, INPUT_PULLUP);
	pinMode(downDistance, INPUT_PULLUP);

	// pins for ultrasonic sensor
	pinMode(echo, INPUT);
	pinMode(trig, OUTPUT);
	digitalWrite(trig, LOW);  // default low for ultrasonic sensor trigger pin
	// digitalWrite(echo, HIGH);  // this could be intentional to set a pullup resistor, check functionality with sensor plugged in
	setupDisplay();
	Serial.begin(19200);
	watchdog.reset();
	Serial.println("\n\n\nRESTART COMPLETE ENTERING VOID LOOP()\n");
}

void loop() {
	delay(800);
	adjustLevel();
	Serial.print(avgDistance);
	Serial.println(" is the average distance before sensorData()");
	avgDistance = sensorData();
	refreshDisplay();                                  // useful if debugging sensorData since it loops back around very quickly when not running the pump
	enableAlarm(checkSensor, checkRelay, checkWater);  // checks all three global booleans that are modified by above functions and checks based on external sensor input only
	watchdog.reset();
	autoRunPump(8000);  // pass in the length of time you want to run pump in milliseconds, auto refers to self check in function for conditions
	                    // IMPORTANT INTERGER FOR PUMP TIME, NOT CURRENTLY VARIABLE WHILE INSTALLED, TIME SHOULD EQUAL NO MORE THAN THE TIME TO PUMP FROM MAX OR MIN HEIGHT TO A SAFE LEVEL FOR PUMP
	                    // DO NOT SET TO LONGER THAN IT WOULD TAKE TO PUMP THE SYSTEM DRY AT LOWEST WATER LEVEL TRIGGER
}