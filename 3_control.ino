
void control () 
{
  // check if system is on
  if (systemOn) {
    
    // check if it's time to water
    for (int i = 0; i < 4; i++) {
      if (plantCheck(i)) {
        // yes -> execute water routine
        water(i);
      }
    }
  }
}


bool plantCheck (int plant) 
{
  // find elapsed time since last water
  long elapsedTime = now() - lastWater[plant];

  // check if in auto mode
  if (modes[plant] == 2 and elapsedTime > minInterval[plant] * 60 * 60 and sensor[plant] < thresh[plant]) {
    // minimun interval exceed and moisturelevels too low -> time to water!
    return true;
  }

  // check if in timer mode
  else if (modes[plant] == 3 and elapsedTime > interval[plant] * 60 * 60 * 24) {
    // time interval has been exceeded -> time to water:
    return true;
  }

  // no hit, no water
  return false;
}


void water(int plant) 
{
  Serial.print("Watering plant "); Serial.println(plant + 1);

  // set flag (used for status update)
  pumpOn[plant] = true;

  // push-update status in app and then turn pump on, and
  switch (plant) {
    case 0:
      Blynk.virtualWrite(V11, getStatus(plant));
      digitalWrite(D6, LOW);
      break;
    case 1:
      Blynk.virtualWrite(V12, getStatus(plant));
      digitalWrite(D5, LOW);
      break;
    case 2:
      Blynk.virtualWrite(V13, getStatus(plant));
      digitalWrite(D4, LOW);
      break;
    case 3:
      Blynk.virtualWrite(V14, getStatus(plant));
      digitalWrite(D7, LOW);
      break;
  }

  // delay loop
  long startTime = millis();
  while (millis() - startTime < waterdur(plant)) {
    // keep everything running in the meantime (except the control loop)
    Blynk.run();
    timer1.run();
  }

  // remove flag
  pumpOn[plant] = false;

  // turn pump off and push new status
  switch (plant) {
    case 0:
      digitalWrite(D6, HIGH);  
      Blynk.virtualWrite(V11, getStatus(plant));
      break;
    case 1:
      digitalWrite(D5, HIGH);  
      Blynk.virtualWrite(V12, getStatus(plant));
      break;
    case 2:
      digitalWrite(D4, HIGH);  
      Blynk.virtualWrite(V13, getStatus(plant));
      break;
    case 3:
      digitalWrite(D7, HIGH);  
      Blynk.virtualWrite(V14, getStatus(plant));
      break;
  }

  // Reset water button (if used)
  Blynk.virtualWrite(V8, 0);

  // update lastwater to server
  lastWater[plant] = now();
  Blynk.virtualWrite(V34, lastWater[0], lastWater[1], lastWater[2], lastWater[3]);
}


int waterdur (int plant) 
{
  // convert ml to ms for controlling pump on-time, different values depending on the chosen pump

  switch (plant) {
    case 0:
      return amount[plant] * 57 + 350; // set experimentally
    case 1:
      return amount[plant] * 50 + 320; // set experimentally
    case 2:
      return amount[plant] * 59 + 350; // set experimentally
    case 3:
      return amount[plant] * 47 + 340; // set experimentallyl
  } 
}
