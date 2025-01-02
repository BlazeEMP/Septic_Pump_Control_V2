// to establish an automatic alarm reset, wire a new output to the 555 timer reset pin that activates when all alarms are false only

void enableAlarm(bool sensor, bool relay, bool water) {
  if (sensor || relay || water) {  // any error will signal alarm to go off
    // to NOT gate then trigger on 555 (PIN 2) set to latching behaviour for alarm
    digitalWrite(alarm, HIGH);
    return;
  } else if (sensor == false && relay == false && water == false) {  // will keep the alarm set high until NO error conditions exist, even if reset button is pushed
    // currently using hardwire reset button on alarm so this will not turn off the alarm just reset the error message and on signal to buzzer
    digitalWrite(alarm, LOW);
    return;
  }
}