void autoRunPump(int startPump) {
  int pumpTime = startPump;  // set startPump to pumpTime as pumpTime is changed iteratively, startPump is the inital value only
  int S = 1000;              // used for math to split pump time into 1 second sections by milliseconds

  if (avgDistance <= heightSetting && avgDistance != 0) {
    avgDistance = 0;  // this will reset the sensors loop to regain a measurement of the new water level after pumping, without thinking it's receiving a random signal with a large change in level. This would trigger the error correction conditions in Sensor file otherwise
    digitalWrite(runPumpRelay, HIGH);
    while (pumpTime >= 0) {
      if (pumpTime == startPump) {
        // use the following line only initially (15x0) will only need to be updated after each delay

        // we dont want to create a global variable for tracking time or have to pass in a value to refreshDisplay function. Update timer display within this function
        lcd.setCursor(10, 0);  // must be on upper row column (10x0) after vertical divider

        // use the following line only initially (15x0) will only need to be updated after each delay
        // prints "Pump...#" where # is the inital pumpTime in seconds
        lcd.print(pumping);
        lcd.write(2);
        lcd.print(pumpTime / S);

        delay(S);
        pumpTime = pumpTime - S;
      } else {
        lcd.setCursor(15, 0);
        lcd.print(pumpTime / S);
        delay(S);
        watchdog.reset();
        pumpTime = pumpTime - S;
      }
    }
    digitalWrite(runPumpRelay, LOW);
    delay(S);
    watchdog.reset();
    lcd.setCursor(10, 0);
    lcd.print("      ");  // display 6 spaces as blanking for pump timer after 3 second settling period... this allows splashing water and moving parts to not make erratic sensor reading for ultrasonic style sensors. Direct contact water sensors may not need this at all depending on setup
  }
  return;
}

// CAUTION CONTAINS A WHILE LOOP, unless other functions are added to interupt, will only leave the function once the minimum or maximum level setting is reached or no change is made (toggle switch released)
void adjustLevel() {
  int maxDistanceCM = 30;  // set upper limit for distance to trigger, higher is lower water level
  int minDistanceCM = 20;  // set lower limit for distance to trigger, lower is higher water level
  while (digitalRead(upDistance) == LOW || digitalRead(downDistance) == LOW) {
    if (digitalRead(upDistance) == LOW) {
      if (heightSetting >= maxDistanceCM) {
        return;
      } else {
        heightSetting++;
      }
    } else if (digitalRead(downDistance) == LOW) {
      if (heightSetting <= minDistanceCM) {
        return;
      } else {
        heightSetting--;
      }
    }
    refreshDisplay();  // important!!! reflects the realtime changed setting as we adjust it
    watchdog.reset();
    delay(250);
  }
  return;
}