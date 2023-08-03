

#define PIN_BOTON_1 34
#define PIN_BOTON_2 35
#define PIN_BOTON_3 32
#define PIN_BOTON_4 33
#define PIN_BOTON_5 25




int estadoBoton1;
int estadoBoton2;
int estadoBoton3;
int estadoBoton4;
int estadoBoton5;



void setup() {

  Serial.begin(9600);

  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT);
  pinMode(PIN_BOTON_4, INPUT);
  pinMode(PIN_BOTON_5, INPUT);
  

}



void loop() {

  estadoBoton1 = digitalRead(PIN_BOTON_1);
  estadoBoton2 = digitalRead(PIN_BOTON_2);
  estadoBoton3 = digitalRead(PIN_BOTON_3);
  estadoBoton4 = digitalRead(PIN_BOTON_4);
  estadoBoton5 = digitalRead(PIN_BOTON_5);

  Serial.print("Boton 1: ");
  Serial.println(estadoBoton1);
  Serial.print("Boton 2: ");
  Serial.println(estadoBoton2);
  Serial.print("Boton 3: ");
  Serial.println(estadoBoton3);
  Serial.print("Boton 4: ");
  Serial.println(estadoBoton4);
  Serial.print("Boton 5: ");
  Serial.println(estadoBoton5);

}
