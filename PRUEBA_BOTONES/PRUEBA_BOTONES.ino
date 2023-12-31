

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

unsigned long milisActuales;
unsigned long milisPrevios;


void setup() {

  Serial.begin(9600);

  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT_PULLUP);
  pinMode(PIN_BOTON_4, INPUT_PULLUP);
  pinMode(PIN_BOTON_5, INPUT_PULLUP);


}



void loop() {

  milisActuales = millis();

  estadoBoton1 = digitalRead(PIN_BOTON_1);
  estadoBoton2 = digitalRead(PIN_BOTON_2);
  estadoBoton3 = digitalRead(PIN_BOTON_3);
  estadoBoton4 = digitalRead(PIN_BOTON_4);
  estadoBoton5 = digitalRead(PIN_BOTON_5);

  if ((milisActuales - milisPrevios) > 1000) {


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
    Serial.println("");
    Serial.println("");
    Serial.println("");

    milisPrevios = milisActuales;

  }

}
