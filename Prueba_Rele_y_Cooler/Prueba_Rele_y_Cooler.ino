
void setup() {

pinMode(16, OUTPUT);
pinMode(13, OUTPUT);

}

void loop() {

digitalWrite(16, HIGH);
digitalWrite(13, HIGH);

delay(5000);

digitalWrite(16, LOW);
digitalWrite(13, LOW);

delay(5000);

}
