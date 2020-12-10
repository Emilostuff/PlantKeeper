
void setup()
{

  // Serial
  Serial.begin(9600);

  // Blynk 
  Blynk.begin(auth, ssid, pass);

  // fetch stored data from server
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V30);
  Blynk.syncVirtual(V31);
  Blynk.syncVirtual(V32);
  Blynk.syncVirtual(V33);
  Blynk.syncVirtual(V34);
  Blynk.syncVirtual(V35);

  // Sensors
  Wire.begin(D1, D2);  // SDA, SCL
  if (!adc.init()) {
    Serial.println("ADS1115 not connected!");
  }
  adc.setVoltageRange_mV(ADS1115_RANGE_6144);
  
  // timers
  timer.setInterval(30000L, control);     // for control loop, run every 30 secs
  timer1.setInterval(5000L, readSensors); // for sensor read loop, run every 5 sec

  // RTC
  setSyncInterval(10 * 60);               // Sync interval in seconds (10 minutes)
  
  // pin assignments
  pinMode(D7, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);

  // set pumps to OFF (active-low)
  digitalWrite(D7, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);

  // Set a reasonable start value for sensors (a little above the triggering threshold)
  for (int i = 0; i < 4; i++) {
    sensor[i] = thresh[i] + 10;
  }

  Serial.println("Setup Complete");
}
