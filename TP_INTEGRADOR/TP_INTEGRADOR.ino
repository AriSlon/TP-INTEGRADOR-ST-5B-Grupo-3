#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
//#include <WiFi.h>
//#include <WiFiClientSecure.h>
//#include <UniversalTelegramBot.h>
//#include <ArduinoJson.h>


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

//#define BOTtoken "6582349263:AAHnC5r8S53ASk3J4RTncCs0LZy2-jA65pY"
//#define CHAT_ID "5939693005"

Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferencesTemp;
Preferences preferencesHum;

//WiFiClientSecure client;
//UniversalTelegramBot bot(BOTtoken, client);

const char* ssid = "ari";
const char* password = "004367225aa";

//const char* ssid = "ORT-IoT";
//const char* password = "OrtIOTnew22$2";

String mensaje = "La temperatura actual es: ";

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

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

  /*
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);



      while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
      }

      Serial.println(WiFi.localIP());
      bot.sendMessage(CHAT_ID, "Conexion establecida entre el Bot y el microcontrolador", "");

  */
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

pantallaMenuGeneralSetUp();

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

  Serial.print("Boton 1: ");
  Serial.println(estadoBotonIzquierda);
  Serial.print("Boton 2: ");
  Serial.println(estadoBotonDerecha);
  Serial.print("Boton 3: ");
  Serial.println(estadoBotonArriba);
  Serial.print("Boton 4: ");
  Serial.println(estadoBotonAbajo);
  Serial.print("Boton 5: ");
  Serial.println(estadoBotonEnter);


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
  movimientosCursor();

  //lecturaTiempoBot();


}


void maquinaDeEstadosGeneral () {

  switch (estadoMaquinaGeneral) {

    case PANTALLA_GENERAL:

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

      pantallaMenuGeneral();



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


  if ((milisActuales - milisPrevios) > 10000) {

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

    milisPrevios = milisActuales;

  }

  if (luzPorcentaje < 10) {
    lcd.setCursor(6, 2);
    lcd.print(" ");
  }

  if (luzPorcentaje < 100) {
    lcd.setCursor(7, 2);
    lcd.print(" ");
  }


  if (humedadPorcentaje < 10) {
    lcd.setCursor(10, 1);
    lcd.print(" ");
  }

  if (humedadPorcentaje < 100) {
    lcd.setCursor(11, 1);
    lcd.print(" ");
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

  if (estadoMaquinaGeneral == PANTALLA_GENERAL || estadoMaquinaGeneral == MOVIMIENTOS_CURSOR) {

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

  if (estadoMaquinaGeneral != PANTALLA_GENERAL && estadoMaquinaGeneral != MOVIMIENTOS_CURSOR) {

    lcd.setCursor(19, 0);
    lcd.print(" ");
    lcd.setCursor(19, 1);
    lcd.print(" ");
    lcd.setCursor(19, 2);
    lcd.print(" ");
    lcd.setCursor(19, 3);
    lcd.print(" ");
  }

}

void pantallaMenuGeneralSetUp() {

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

  if (luzPorcentaje < 10) {
    lcd.setCursor(6, 2);
    lcd.print(" ");
  }

  if (luzPorcentaje < 100) {
    lcd.setCursor(7, 2);
    lcd.print(" ");
  }


  if (humedadPorcentaje < 10) {
    lcd.setCursor(10, 1);
    lcd.print(" ");
  }

  if (humedadPorcentaje < 100) {
    lcd.setCursor(11, 1);
    lcd.print(" ");
  }



}

/*
  void lecturaTiempoBot () {

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Veo los msj nuevos");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();

  }


  }

  void handleNewMessages(int numNewMessages) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {  ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    ///fin de verificacion

    // imprime el msj recibido
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    /// si rebice /led on enciende el led
    if (text == "/temperatura_actual") {
      bot.sendMessage(chat_id, (String)bmp.readTemperature(), "");
    }

  }

  }

*/

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
