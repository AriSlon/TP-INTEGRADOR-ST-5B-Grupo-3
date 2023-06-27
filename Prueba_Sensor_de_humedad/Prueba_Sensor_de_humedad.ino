#define PIN_SENSOR 36

float humedad;

void setup() {

  Serial.begin(9600);

}

void loop() {

  humedad = analogRead(PIN_SENSOR);
  Serial.println(humedad);

}
