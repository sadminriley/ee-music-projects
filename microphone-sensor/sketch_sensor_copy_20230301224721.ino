int SENSOR = 3;              // Digital/PWM pin number for
int LED = 7;                // Define pin number


void setup(){

  pinMode (SENSOR, INPUT);
  pinMode (LED, OUTPUT);
}


void loop(){

  int sensorvalue = digitalRead (SENSOR);            // If the intensity is higher than the set threshhold, return sensor value as 1

  if (sensorvalue == 1)
  {
    digitalWrite(LED, HIGH);
  }

  else
  {
    digitalWrite(LED, LOW);
  }

}
