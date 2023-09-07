
void setup() {

  Serial.begin(9600);

  pinMode(15, OUTPUT);

}

void loop() {

  digitalWrite(15, HIGH);
  Serial.println(digitalRead(15));

  delay(1000);

  digitalWrite(15, LOW);
  Serial.println(digitalRead(15));

  delay(1000);
  
}

void HighYLow() {

  digitalWrite(15, HIGH);
  Serial.println(digitalRead(15));

  delay(1000);

  digitalWrite(15, LOW);
  Serial.println(digitalRead(15));

  delay(1000);

}

void High() {

  digitalWrite(15, HIGH);
  Serial.println(digitalRead(15));

  delay(1000);

}
