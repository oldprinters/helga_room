/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>
#include "Timer.h"
#include "FilterES.h"

VL53L0X sensor;
Timer tVL53(50);  //замер расстояния
Timer tVL53out(500);  //замер расстояния
FilterES fes(5, 0.85);
uint16_t range{};

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();
}

void loop()
{
  if (tVL53.getTimer()){
    tVL53.setTimer();
    range = sensor.readRangeContinuousMillimeters();
    fes.FilterAnalog(static_cast<double>(range));
    if (sensor.timeoutOccurred()) { Serial.println(" TIMEOUT"); }
  }
  if(tVL53out.getTimer()){
    tVL53out.setTimer();
    Serial.println(fes.getValue());
  }
}