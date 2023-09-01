#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>


#define PIN_BOTON_1 34
#define PIN_BOTON_2 35
#define PIN_BOTON_3 32
#define PIN_BOTON_4 33
#define PIN_BOTON_5 25

#define PIN_BOTON_IZQUIERDA 34
#define PIN_BOTON_DERECHA 35
#define PIN_BOTON_ARRIBA 32
#define PIN_BOTON_ABAJO 33
#define PIN_BOTON_ENTER 25

#define PIN_SENSOR_HUMEDAD 2
#define PIN_RELE_COOLER 15
#define PIN_LED_ROJO 12
#define PIN_LED_AMARILLO 14
#define PIN_LED_VERDE 27

#define BUZZER_PIN 4    // Pin del buzzer
#define BUZZER_CHANNEL 0 // Canal PWM del buzzer

#define PRESIONADO 0
#define SUELTO 1

#define ARRIBA 0
#define ABAJO 1

#define TEMPERATURA 0
#define HUMEDAD 1

#define PANTALLA_GENERAL 0
#define ESPERA_GENERAL_UMBRALTEMP 1
#define PANTALLA_UMBRAL_TEMPERATURA 2
#define SUMA_UMBRAL_TEMPERATURA 3
#define RESTA_UMBRAL_TEMPERATURA 4
#define ESPERA_GENERAL_UMBRALHUM 5
#define PANTALLA_UMBRAL_HUMEDAD 6
#define SUMA_UMBRAL_HUMEDAD 7
#define RESTA_UMBRAL_HUMEDAD 8
#define ESPERA_VUELTA_GENERAL 9
#define MOVIMIENTOS_CURSOR 10
#define ESPERA_2 7
#define ESPERA_2 8
#define ESPERA_2 9
#define ESPERA_2 10



Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferencesTemp;
Preferences preferencesHum;

int estadoBotonIzquierda;
int estadoBotonDerecha;
int estadoBotonArriba;
int estadoBotonAbajo;
int estadoBotonEnter;

int estadoMaquinaGeneral;

unsigned long milisActuales;
unsigned long milisPrevios;

bool estadoCooler;

int luzPorcentaje;
int luz;

int humedad;
int humedadPorcentaje;

int valorUmbralTemp;
int valorUmbralHum;

int cursorPantalla;

bool chequeoCursor;
bool chequeoPantallaUmbral;

void setup() {

  Serial.begin(9600);

  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT_PULLUP);
  pinMode(PIN_BOTON_4, INPUT_PULLUP);
  pinMode(PIN_BOTON_5, INPUT_PULLUP);
  pinMode(PIN_RELE_COOLER, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);


  // /*while ( !Serial ) delay(100);

  Serial.println(F("BMP280 test"));

  unsigned status;

  status = bmp.begin(0x76);

  if (!status) {

    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  // Default settings from datasheet.

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode.
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering.
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time.

  //*/

  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

  preferencesTemp.begin("memoria", valorUmbralTemp);
  preferencesHum.begin("memoria2", valorUmbralHum);


  valorUmbralTemp = preferencesTemp.getInt("memoria", 0);
  valorUmbralHum = preferencesHum.getInt("memoria2", 0);


  ledcSetup(BUZZER_CHANNEL, 2000, 8); // Configurar el canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // Asociar el pin al canal PWM
  ledcWrite(BUZZER_CHANNEL, 0);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  pantallaMenuGeneral();

  lcd.setCursor(0, 19);
  lcd.print("*");

}



void loop() {

  milisActuales = millis();

  estadoBotonIzquierda = !digitalRead(PIN_BOTON_1);
  estadoBotonDerecha = digitalRead(PIN_BOTON_2);
  estadoBotonArriba = digitalRead(PIN_BOTON_3);
  estadoBotonAbajo = digitalRead(PIN_BOTON_4);
  estadoBotonEnter = digitalRead(PIN_BOTON_5);

  /*Serial.print("Boton 1: ");
    Serial.println(estadoBotonIzquierda);
    Serial.print("Boton 2: ");
    Serial.println(estadoBotonDerecha);
    Serial.print("Boton 3: ");
    Serial.println(estadoBotonArriba);
    Serial.print("Boton 4: ");
    Serial.println(estadoBotonAbajo);
    Serial.print("Boton 5: ");
    Serial.println(estadoBotonEnter);
  */

  Serial.println(cursorPantalla);

  digitalWrite(PIN_LED_VERDE, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_ROJO, HIGH);

  humedad = analogRead(PIN_SENSOR_HUMEDAD);
  humedadPorcentaje = map(humedad, 0, 2950, 0, 100);

  luz = lightMeter.readLightLevel();

  luzPorcentaje = map(luz, 0, 65535, 0, 100);

  if (PIN_RELE_COOLER == HIGH) {

    estadoCooler = 1;
  }

  if (PIN_RELE_COOLER == LOW) {

    estadoCooler = 0;
  }

  if (cursorPantalla < 0) {
    cursorPantalla = 0;
  }

  if (cursorPantalla > 3) {
    cursorPantalla = 3;
  }

  maquinaDeEstadosGeneral();

}


void maquinaDeEstadosGeneral () {

  switch (estadoMaquinaGeneral) {

    case PANTALLA_GENERAL:

      movimientosCursor();

      if (estadoBotonAbajo == PRESIONADO) {
        chequeoCursor = ABAJO;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (estadoBotonArriba == PRESIONADO) {
        chequeoCursor = ARRIBA;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (cursorPantalla == 0 && estadoBotonEnter == PRESIONADO) {
        estadoMaquinaGeneral = ESPERA_GENERAL_UMBRALTEMP;
      }

      if (cursorPantalla == 1 && estadoBotonEnter == PRESIONADO) {
        estadoMaquinaGeneral = ESPERA_GENERAL_UMBRALHUM;
      }

      if ((milisActuales - milisPrevios) > 1000) {

        pantallaMenuGeneral();

        milisPrevios = milisActuales;

      }


      break;

    case ESPERA_GENERAL_UMBRALTEMP:

      pantallaMenuGeneral();

      if (estadoBotonEnter == SUELTO) {
        lcd.clear();
        estadoMaquinaGeneral = PANTALLA_UMBRAL_TEMPERATURA;
      }


      break;

    case PANTALLA_UMBRAL_TEMPERATURA:

      pantallaUmbralTemp();

      if (estadoBotonDerecha == PRESIONADO) {
        estadoMaquinaGeneral = SUMA_UMBRAL_TEMPERATURA;
      }

      if (estadoBotonIzquierda == PRESIONADO) {
        estadoMaquinaGeneral = RESTA_UMBRAL_TEMPERATURA;
      }

      if (estadoBotonAbajo == PRESIONADO && estadoBotonArriba == PRESIONADO) {
        chequeoPantallaUmbral = TEMPERATURA;
        estadoMaquinaGeneral = ESPERA_VUELTA_GENERAL;
      }

      break;

    case SUMA_UMBRAL_TEMPERATURA:

      pantallaUmbralTemp();

      if (estadoBotonDerecha == SUELTO ) {
        valorUmbralTemp += 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_TEMPERATURA;
      }

      break;

    case RESTA_UMBRAL_TEMPERATURA:

      pantallaUmbralTemp();

      if (estadoBotonIzquierda == SUELTO ) {
        valorUmbralTemp -= 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_TEMPERATURA;
      }

      break;

    case ESPERA_GENERAL_UMBRALHUM:

      pantallaMenuGeneral();

      if (estadoBotonEnter == SUELTO) {
        lcd.clear();
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }


      break;

    case PANTALLA_UMBRAL_HUMEDAD:

      pantallaUmbralHum();

      if (estadoBotonDerecha == PRESIONADO) {
        estadoMaquinaGeneral = SUMA_UMBRAL_HUMEDAD;
      }

      if (estadoBotonIzquierda == PRESIONADO) {
        estadoMaquinaGeneral = RESTA_UMBRAL_HUMEDAD;
      }

      if (estadoBotonAbajo == PRESIONADO && estadoBotonArriba == PRESIONADO) {
        chequeoPantallaUmbral = HUMEDAD;
        estadoMaquinaGeneral = ESPERA_VUELTA_GENERAL;
      }

      break;

    case SUMA_UMBRAL_HUMEDAD:

      pantallaUmbralHum();

      if (estadoBotonDerecha == SUELTO ) {
        valorUmbralHum += 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }

      break;

    case RESTA_UMBRAL_HUMEDAD:

      pantallaUmbralHum();

      if (estadoBotonIzquierda == SUELTO ) {
        valorUmbralHum -= 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }

      break;

    case ESPERA_VUELTA_GENERAL:

      if (chequeoPantallaUmbral == TEMPERATURA) {
        pantallaUmbralTemp();
      }

      if (chequeoPantallaUmbral == HUMEDAD) {
        pantallaUmbralHum();
      }

      if (estadoBotonAbajo == SUELTO && estadoBotonEnter == SUELTO) {
        preferencesTemp.putInt("memoria", valorUmbralTemp);
        preferencesHum.putInt("memoria2", valorUmbralHum);
        lcd.clear();
        estadoMaquinaGeneral = PANTALLA_GENERAL;
      }

      break;

    case MOVIMIENTOS_CURSOR:

      pantallaMenuGeneral();

      if (estadoBotonAbajo == SUELTO && chequeoCursor == ABAJO ) {
        cursorPantalla += 1;
        estadoMaquinaGeneral = PANTALLA_GENERAL;
      }

      if (estadoBotonArriba == SUELTO && chequeoCursor == ARRIBA ) {
        cursorPantalla -= 1;
        estadoMaquinaGeneral = PANTALLA_GENERAL;
      }

      break;

  }
}

void pantallaMenuGeneral() {

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(bmp.readTemperature());

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humedadPorcentaje);

  lcd.setCursor(0, 2);
  lcd.print("Luz: ");
  lcd.print(luzPorcentaje);


  lcd.setCursor(0, 3);
  lcd.print("Cooler: ");

  if (estadoCooler == 1) {
    lcd.setCursor(8, 3);
    lcd.print("On");
  }

  if (estadoCooler == 0) {
    lcd.setCursor(8, 3);
    lcd.print("Off");
  }



}


void pantallaUmbralTemp() {

  lcd.setCursor(0, 0);
  lcd.print("Umbral Temp: ");
  lcd.print(valorUmbralTemp);

}


void pantallaUmbralHum() {

  lcd.setCursor(0, 0);
  lcd.print("Umbral Hum: ");
  lcd.print(valorUmbralHum);

}


void movimientosCursor() {

  if (cursorPantalla == 0) {
    lcd.setCursor(19, 0);
    lcd.print("*");
    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(19, 2);
    lcd.print(" ");
    lcd.setCursor(19, 3);
    lcd.print(" ");

  }

  if (cursorPantalla == 1) {
    lcd.setCursor(19, 0);
    lcd.print(" ");
    lcd.setCursor(19, 1);
    lcd.print("*");
    lcd.setCursor(19, 2);
    lcd.print(" ");
    lcd.setCursor(19, 3);
    lcd.print(" ");

  }

  if (cursorPantalla == 2) {
    lcd.setCursor(19, 0);
    lcd.print(" ");
    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(19, 2);
    lcd.print("*");
    lcd.setCursor(19, 3);
    lcd.print(" ");

  }

  if (cursorPantalla == 3) {
    lcd.setCursor(19, 0);
    lcd.print(" ");
    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(19, 2);
    lcd.print(" ");
    lcd.setCursor(19, 3);
    lcd.print("*");

  }

}

/*

  void loop() {

    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

  }

  void loop() {

  uint16_t lux = lightMeter.readLightLevel();

  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  }

  void loop() {

  humedad = analogRead(PIN_SENSOR_HUMEDAD);
  Serial.println(humedad);

  }

  void loop() {

  milisActuales = millis();

  switch (estadoMaquina) {

    case 0:

      lcd.setCursor(0, 0);
      lcd.print("Test Funcionamiento");
      lcd.setCursor(0, 1);
      lcd.print("Pantalla LCD");
      lcd.setCursor(0, 2);
      lcd.print("20x4 I2C");


      if ((milisActuales - milisPrevios) > 6000) {


        estadoMaquina = 1;
        milisPrevios = milisActuales;

      }

      break;

    case 1:

      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Hola Veckiardo");

      estadoMaquina = 2;

      break;

      case 2:

      break;

  }

  }

  void loop() {

  Serial.print(digitalRead(4));
  ledcWrite(BUZZER_CHANNEL, 128);

  delay(2000);

  ledcWrite(BUZZER_CHANNEL, 0);

  delay(2000);

  }

  void loop() {

  digitalWrite(PIN_RELE_COOLER, HIGH);

  delay(5000);

  digitalWrite(PIN_RELE_COOLER, LOW);

  delay(5000);

  }

  void loop() {

  digitalWrite(PIN_LED_ROJO, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_VERDE, HIGH);


  #define PIN_LED_ROJO 12
  #define PIN_LED_AMARILLO 14
  #define PIN_LED_VERDE 27

  void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);


  }

  void loop() {

  digitalWrite(PIN_LED_VERDE, HIGH);
  digitalWrite(PIN_LED_AMARILLO, HIGH);
  digitalWrite(PIN_LED_ROJO, HIGH);

  }

  void buzzerTercerUmbral() {

  milisActualesBuzzerTercerUmbral = millis();

  switch (estadoMaquinaBuzzerTercerUmbral) {

    case 0:

      ledcWrite(BUZZER_CHANNEL, 128);

      if ((milisActualesBuzzerTercerUmbral - milisPreviosBuzzerTercerUmbral) > 150) {

        milisPreviosBuzzerTercerUmbral = milisActualesBuzzerTercerUmbral;
        estadoMaquinaBuzzerTercerUmbral = 1;

      }

      break;

    case 1:

      ledcWrite(BUZZER_CHANNEL, 0);

      if ((milisActualesBuzzerTercerUmbral - milisPreviosBuzzerTercerUmbral) > 150) {

        milisPreviosBuzzerTercerUmbral = milisActualesBuzzerTercerUmbral;
        estadoMaquinaBuzzerTercerUmbral = 0;

      }

      break;

  }

  }
*/
