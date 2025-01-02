// add error display triggers and an input pin sensing if the alarm is triggered. Only once we push the button to reset the alarm should the error message be blanked over !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!-----------------------------------------------

// section for display formatting
char cm[] = "cm";
char noData3[] = "???";
char set[] = "Set:";      // for displaying set trigger level
char water[] = "Wtr:";    // for displaying current water distance
char error[] = "Err";     // only show when displaying error messages
char pumping[] = "Pump";  // for displaying second of pumping

byte periodsChar[8] = {  // ellipses character (custom char 2)
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101
};
byte linesChar[8] = {  // 2 vertical lines seperation character (custom char 1)
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010
};

void setupDisplay() {
  // start display and write the charaters that will not be written over
  lcd.init();                      // initialize the lcd
  lcd.createChar(1, linesChar);    // vertical line divider custom char
  lcd.createChar(2, periodsChar);  // elipses custom char
  lcd.backlight();                 // enable backlight
  lcd.setCursor(0, 0);             // begin 0,0 writing
  lcd.print(water);                // display "Wtr:"
  lcd.print(noData3);              // display no data during startup for water level (allows visual inidcator of code hangup if things don't load far enough to get senor data)
  lcd.print(cm);                   // display "cm"
  lcd.write(1);                    // create vertical line divier (currnetly 9x0 coord on LCD)
  lcd.setCursor(0, 1);             // go to line 2 on LCD
  lcd.print(set);                  // display "Set:"
  lcd.print(" ");                  // empty spot next to "Set:" since trigger level should be 2 digits in CM (<10 is too high for sump pit, >100 would run too often and pump for too long with 5 second pumping config and volume of our sump pit)
  lcd.print(heightSetting);        // display default height setting
  lcd.print(cm);                   // display "cm"
  lcd.write(1);                    // create vertical line divier (currnetly 9x0 coord on LCD)
}

int lastSetting;  // used to store a value between loops in refresh display function, saves time overwriting values that aren't normally updated (I2C commm. takes multiple clock cycles compared to quick if() statement check)

void refreshDisplay() {
  // run through on each refresh to display new reading for water level from sensor
  if (avgDistance == 0) {
    lcd.setCursor(4, 0);  // draw no data on water level measurement for visual indication of sensor loop reading 0s, clearly indicates an error rather than just displaying a 0
    lcd.print(noData3);
  } else if (avgDistance < 10) { // in my setup you may notice that we can never reach a ditance less than 10, by design this is more flexible than needed so unexpected inputs don't cause errors
    lcd.setCursor(4, 0);
    lcd.print("  ");
    lcd.print(avgDistance);
  } else if (avgDistance < 100) { // in my setup we should only ever recieve 2 digit or greater measurements without triggering any mechanisms
    lcd.setCursor(4, 0);
    lcd.print(" ");
    lcd.print(avgDistance);
  } else {
    lcd.setCursor(4, 0);
    lcd.print(avgDistance);
  }

  // prevents LCD commands from running when not making adjustments, compare to global variable lastSetting
  if (lastSetting != heightSetting) {
    lcd.setCursor(5, 1);
    lcd.print(heightSetting);
    lastSetting = heightSetting;
  }

  return;
}