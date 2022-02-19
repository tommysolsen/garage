#include <Arduino.h>

#define POWER_LED 2
#define EXTERNAL_LED 27
#define GARAGE_PORT 22

#define MANUAL_TRIGGER 21

void initiatedPing(int port)
{
  digitalWrite(port, HIGH);
  delay(1000);
  digitalWrite(port, LOW);
  delay(1000);

  digitalWrite(port, HIGH);
  delay(500);
  digitalWrite(port, LOW);
  delay(500);

  digitalWrite(port, HIGH);
  delay(250);
  digitalWrite(port, LOW);
  delay(250);
}

/**
 *
 * Function that determines what happends when the garage port shall open
 *
 */
void toggleGarage()
{
  digitalWrite(EXTERNAL_LED, HIGH);
  digitalWrite(GARAGE_PORT, HIGH);
  delay(1000);
  digitalWrite(EXTERNAL_LED, LOW);
  digitalWrite(GARAGE_PORT, HIGH);
}

/**
 *
 * This function is run when someone tries to authenticate with the opener but fails.
 * For debugging purposes we currently blink some lights on the device.
 *
 */
void signalAuthenticationFailure()
{
  digitalWrite(EXTERNAL_LED, HIGH);
  delay(100);
  digitalWrite(EXTERNAL_LED, LOW);
  delay(100);
  digitalWrite(EXTERNAL_LED, HIGH);
  delay(100);
  digitalWrite(EXTERNAL_LED, LOW);
  delay(100);
}

void enableManualOpenClose()
{
  pinMode(MANUAL_TRIGGER, INPUT_PULLUP);
  attachInterrupt(MANUAL_TRIGGER, toggleGarage, RISING);
}
