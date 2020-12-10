
void readSensors() 
{
  float reading[4];

  // read raw values
  reading[0] = readChannel(ADS1115_COMP_0_GND);
  reading[1] = readChannel(ADS1115_COMP_1_GND);
  reading[2] = readChannel(ADS1115_COMP_2_GND);
  reading[3] = readChannel(ADS1115_COMP_3_GND);

  // convert to percentage, filter and constrain (if we got a rouge reading)
  for (int i = 0; i<4; i++) {
    reading[i] = map(reading[i], sensorDry[i], sensorWet[i], 0, 100);
    sensor[i] = tf * reading[i] + (1 - tf) * sensor[i];
    sensor[i] = constrain(sensor[i], 0, 100);
  }

  // Write to server
  Blynk.virtualWrite(V21, sensor[0]); 
  Blynk.virtualWrite(V22, sensor[1]); 
  Blynk.virtualWrite(V23, sensor[2]); 
  Blynk.virtualWrite(V24, sensor[3]); 
}


float readChannel(ADS1115_MUX channel) 
{
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while (adc.isBusy()) {
    Blynk.run(); // nope!
  }
  voltage = adc.getResult_mV(); 
  return voltage;
}
