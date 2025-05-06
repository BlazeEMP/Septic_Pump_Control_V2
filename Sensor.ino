// these functions only control the ultrasonic distance sensor (and the relevant checkSensor error flag), loop for averaging the measurements of distance, and assigning the value stored to the global avgDistance variable to allow the refreshDisplay function to update at the end after leaving loop but before leaving function

int sensorError = 0;  // tracking variable for errors in sensor, keep global to adjust with one function and read the value for comparison in another

int sensorData() {
	int loops = 6;                    // set number of loops for avg measurement
	int rollingInput[loops] = { 0 };  // establish array to store sensor inputs while sorting through data
	// distance modifier is not a great indicator when bubbles can form in the pit, ultrasonic sensor inputs should have wide acceptable range
	int distanceModifier = 100;   // sets a value to compare the measurements in data sorting later, prevents errant inputs from being considered, does not add to an error count like 0s

	// active error tracking
	int zeroCount = 0;
	int faultMax = 180;  // number of loops with more than zeroMax 0 readings before throwing error

	for (int i = 0; i < loops; i++) {
		int distance = pulseSensor();

		if (distance != 0) {
      if (distance > highReadingWarning) {
        highReadingTracker ++;
      }
			if (avgDistance == 0 && i < 3) {  // this condition only runs if no avgDistance has been set yet (besides initial 0) allows the establishment of a baseline on startup
				rollingInput[i] = distance;
				Serial.print("Resetting error correction input, avgDistance was set to 0");
				Serial.print("\t");
				Serial.print("rollingInput at index [");
				Serial.print(i);
				Serial.print("] = ");
				Serial.println(rollingInput[i]);
			} else if (i < 2) {  // for first two spots in each loop check data against last measured and corrected avg measurement of the loop that was stored globally
				// [**1] if data is considered bad, remove 1 from index tracker as we do with a 0 value input, disregard the gathered data and continue on to next for() iteration
				// we want to make to make sure a bad input (decided by distanceModifier) isn't detected and stored in the array of distances, so anything greater than the int distanceModifier, as distance in cm, from the last avgDistance will be disregarded
				if (distance < avgDistance && avgDistance - distance > distanceModifier) {
					Serial.println("distance < avgDistance && avgDistance - distance > distanceModifier");
					i--;
					watchdog.reset();
				} else if (distance > avgDistance && distance - avgDistance > distanceModifier) {
					Serial.println("distance > avgDistance && distance - avgDistance > distanceModifier");
					i--;
					watchdog.reset();
				} else {
					rollingInput[i] = distance;
					Serial.print("rollingInput at index [");
					Serial.print(i);
					Serial.print("] = ");
					Serial.println(rollingInput[i]);
				}
			} else {                                                    // all values past the first 3 (for first run of loop), and 2 (for all other loop cycles) values of the array are checked here using the same distanceModifier logic
				int rollAvg = rollingInput[i - 1] + rollingInput[i - 2];  // two accepted inputs with error correction based on average (or being the only first 3 inputs)
				rollAvg = rollAvg / 2;
				// [**1]
				if (distance < rollAvg && rollAvg - distance > distanceModifier) {
					Serial.println("distance < rollAvg && rollAvg - distance > distanceModifier");
					i--;
					watchdog.reset();
				} else if (distance > rollAvg && distance - rollAvg > distanceModifier) {
					Serial.println("distance > rollAvg && distance - rollAvg > distanceModifier");
					i--;
					watchdog.reset();
				} else {
					rollingInput[i] = distance;
					Serial.print("rollingInput at index [");
					Serial.print(i);
					Serial.print("] = ");
					Serial.println(rollingInput[i]);
				}
			}
			if (i == loops - 1 && zeroCount == 0) {  // if we make it through a whole for loop reading no 0s we can reset the alarm trigger for the sensor
				sensorError = 0;
				checkSensor = false;
			}
		} else {
			zeroCount++;
			Serial.println(" 0! ");
			Serial.print("Zero count = ");
			Serial.println(zeroCount);
			bool maxZeroReached = errorCheck(zeroCount);
			i--;
			if (maxZeroReached) {
				// add a return for sensor error loops and boolean change if alarm conditions met
				if (sensorError > faultMax) {  // if the count of loops with zeroCount being too high is greater than the variable faultMax, set checkSensor error flag to true
					Serial.println("faultMax reached");
					checkSensor = true;
				}
				return 0;  // since we aren't storing 0s in the array and allowing the index to progress by 1, once we reach the maximum number of 0s permitted by the set variable zeroMax in errorCheck() we can return 0 directly to save time on the averageOfLoops() calculation
			}
		}
	}
  if (highReadingTracker > highReadingLimit) {
    autoRunPump(2500);
    highReadingTracker = 0;
  }
	return averageOfLoops(loops, rollingInput);
}

int averageOfLoops(int loops, int rollingInput[]) {
	int loopAvg = 0;  // used as return value for function, calculate avg of rollingInput at end of function, otherwise should allow to return 0
	for (int i = 0; i < loops; i++) {
		loopAvg += rollingInput[i];
	}
	loopAvg = loopAvg / loops;
	Serial.print("\n------Leaving sensor loop------\n");
	Serial.print("loopAvg = ");
	Serial.println(loopAvg);
	Serial.println("\n\n");
	return loopAvg;
}

int pulseSensor() {
	digitalWrite(trig, HIGH);  // setting trigger pin high for 10-15 microseconds (not flexible, only try up to 20 micro)
	delayMicroseconds(15);
	digitalWrite(trig, LOW);                    // keep low until new trigger
	int distance = pulseIn(echo, HIGH, 26000);  // reading ultrasonic reflections from modules output echo pin

	// algorithm invloving speed of sound in air converted to centimeters or other
	distance = (distance / 2) / 29.1;  // distance/58 for centimeters OR distance/ for inches (change displayed characters for units)

	delay(58);  // sets delay for sensing times (recommended 60ms total measurement cycle) we have spare time spent on calculations so can be reduced slightly

	Serial.print("Distance = ");
	Serial.print(distance);
	Serial.print("\t");
	return distance;
}

// this function only sets the condition to true for checking sensor errors. We reset the alarm trigger by completeing a full for loop in sensorData() reading NO zeros at all
bool errorCheck(int zeroCount) {
	int zeroMax = 10;  // total number of 0s that can be in one loop before fault condition is met

	if (zeroCount >= zeroMax) {
		sensorError++;
		return true;
	}
	return false;
}