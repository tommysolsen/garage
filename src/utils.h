
#define POWER_LED 2
#define EXTERNAL_LED 27
#define GARAGE_PORT 22

#define MANUAL_TRIGGER 21

void initiatedPing(int port);

void toggleGarage();

void signalAuthenticationFailure();

void enableManualOpenClose();