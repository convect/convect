/*
 * Convect v1.0
 * (c) 2013 D.M.
 */

#include <Bridge.h>
#include <Console.h>
#include <Arduino.h>
#include "convectors.h"

Convectors *convs;
TempSensors *temps;

void setup() {
  // put your setup code here, to run once:
  Bridge.begin();
  Console.begin();
  temps = new TempSensors(1, 3.95);
  convs = new Convectors(1);

  while (!Console) ;
}

int count = 0;

void loop() {

  temps->cycle();
  convs->cycle();
  count++;
  if (!(count % 5)) {
    Console.println(temps->getInstantTemp(BEDROOM));
  }
  if (count == 60) {
    float temp = temps->getTemp(BEDROOM);
    Console.print("Average Temperature: ");
    Console.println(temp);
    if (temp <= 19.) {
      convs->setMode(BEDROOM, Convectors::MODE_COMFORT);
      Console.println("Convector set to COMFORT");
    }
    if (temp >= 20. && temp < 21.) {
      convs->setMode(BEDROOM, Convectors::MODE_ECO);
      Console.println("Convector set to ECO");
    }
    if (temp >= 21.) {
      convs->setMode(BEDROOM, Convectors::MODE_OFF);
      Console.println("Convector set to OFF");
    }
    count = 0;
  }
  delay(1000);

  /*} else if (ft != 0) {
    uint8_t cmd[] = {'X','X','X','X','X'};
    uint8_t res[1];
    uint16_t l = Bridge.transfer(cmd, 5, res, 1);
    if (l == BridgeClass::TRANSFER_TIMEOUT) {
      Console.println("BLAH");
    }
    ft = 0;

  }*/
}
