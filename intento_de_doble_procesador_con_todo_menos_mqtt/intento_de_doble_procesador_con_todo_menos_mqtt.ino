/* Codio general
   Trabajo Practico Integrador

   Materia: Seminario de Informatica y Telecomunicaciones (ST).
   Grupo: 3.
   Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
   Profesor: Mirko Veckiardo.

*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <ESP32Time.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "time.h"
#include <WiFi.h>

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

#define PIN_SENSOR_HUMEDAD 36
#define PIN_RELE_COOLER 15
#define PIN_LED_ROJO 27
#define PIN_LED_AMARILLO 14
#define PIN_LED_VERDE 12

#define BUZZER_PIN 4    // Pin del buzzer
#define BUZZER_CHANNEL 0 // Canal PWM del buzzer

#define PRESIONADO 0
#define SUELTO 1

#define ARRIBA 0
#define ABAJO 1

#define TEMPERATURA 0
#define HUMEDAD 1

#define OFF 1
#define ON 0

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
#define ESPERA_GENERAL_MQTT_GMT 10
#define PANTALLA_MQTT_GMT 11
#define SUMA_MQTT_GMT 12
#define RESTA_MQTT_GMT 13
#define ESPERA_MQTT_GMT_GENERAL 14
#define MOVIMIENTOS_CURSOR 15

#define BOTtoken "6582349263:AAHnC5r8S53ASk3J4RTncCs0LZy2-jA65pY"
#define CHAT_ID "5939693005"

Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferencesTemp;
Preferences preferencesHum1;
Preferences preferencesHum2;

ESP32Time rtc;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long milisActuales;
unsigned long milisPrevios;

unsigned long milisActualesBuzzer;
unsigned long milisPreviosBuzzer;

int estadoBotonIzquierda;
int estadoBotonDerecha;
int estadoBotonArriba;
int estadoBotonAbajo;
int estadoBotonEnter;

int estadoMaquinaGeneral;
int estadoMaquinaBuzzer;

int luzPorcentaje;
int luz;

int humedad;
int humedadPorcentaje;

int valorUmbralTemp;
int valorUmbralHum1;
int valorUmbralHum2;

int cursorPantalla;

int ultimoEstadoMaquina;

bool estadoCooler;
bool chequeoCursor;
bool chequeoPantallaUmbral;
bool prendidoBuzzer;
bool flagTemperatura;

float temperatura;

int intervaloMqtt;
int hora;
int minutos;
int gmt = -3;

long unsigned int timestamp; // hora
const char *ntpServer = "south-america.pool.ntp.org";
long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

//const char* ssid = "ari";
//const char* password = "004367225aa";

struct tm timeinfo;

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo


void pedir_lahora(void); // Declaracion de funcion
void setup_rtc_ntp(void); // Declaracion de funcion

TaskHandle_t Task1;
TaskHandle_t Task2;

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

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    1000000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */


  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    1000000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);

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


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password );
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.sendMessage(CHAT_ID, "¡Conexion establecida entre el ESP y VeckiarBot!", "");
  Wire.begin();

  lightMeter.begin();

  setup_rtc_ntp();

  Serial.println(F("BH1750 Test begin"));

  preferencesTemp.begin("memoria", valorUmbralTemp);
  preferencesHum1.begin("memoria2", valorUmbralHum1);
  preferencesHum2.begin("memoria3", valorUmbralHum2);


  valorUmbralTemp = preferencesTemp.getInt("memoria", 0);
  valorUmbralHum1 = preferencesHum1.getInt("memoria2", 0);
  valorUmbralHum2 = preferencesHum2.getInt("memoria3", 0);


  ledcSetup(BUZZER_CHANNEL, 2000, 8); // Configurar el canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // Asociar el pin al canal PWM
  ledcWrite(BUZZER_CHANNEL, 0);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  pantallaMenuGeneralSetUp();

  lcd.setCursor(19, 0);
  lcd.print("*");

}



void Task2code( void * pvParameters ) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    milisActuales = millis();

    estadoBotonIzquierda = !digitalRead(PIN_BOTON_1);
    estadoBotonDerecha = digitalRead(PIN_BOTON_2);
    estadoBotonArriba = digitalRead(PIN_BOTON_3);
    estadoBotonAbajo = digitalRead(PIN_BOTON_4);
    estadoBotonEnter = digitalRead(PIN_BOTON_5);

    /* Serial.print("Boton 1: ");
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
    //Serial.println(estadoMaquinaGeneral);


    temperatura = bmp.readTemperature();

    humedad = analogRead(PIN_SENSOR_HUMEDAD);
    humedadPorcentaje = map(humedad, 0, 4095, 100, 0);

    luz = lightMeter.readLightLevel();
    luzPorcentaje = map(luz, 0, 65535, 0, 100);

    if (humedadPorcentaje <= valorUmbralHum1) {
      digitalWrite(PIN_LED_VERDE, HIGH);
      digitalWrite(PIN_LED_AMARILLO, LOW);
      digitalWrite(PIN_LED_ROJO, LOW);
      digitalWrite(PIN_RELE_COOLER, OFF);
      estadoCooler = 0;

    }

    if (humedadPorcentaje > valorUmbralHum1 && humedadPorcentaje <= valorUmbralHum2) {
      digitalWrite(PIN_LED_VERDE, LOW);
      digitalWrite(PIN_LED_AMARILLO, HIGH);
      digitalWrite(PIN_LED_ROJO, LOW);
      digitalWrite(PIN_RELE_COOLER, OFF);
      estadoCooler = 0;
    }

    if (humedadPorcentaje > valorUmbralHum2) {
      digitalWrite(PIN_LED_VERDE, LOW);
      digitalWrite(PIN_LED_AMARILLO, LOW);
      digitalWrite(PIN_LED_ROJO, HIGH);
      digitalWrite(PIN_RELE_COOLER, ON);
      estadoCooler = 1;
    }



    if (valorUmbralHum1 < 0) {
      valorUmbralHum1 = 0;
    }
    if (valorUmbralHum1 > 100) {
      valorUmbralHum1 = 100;
    }

    if (valorUmbralHum2 < 0) {
      valorUmbralHum2 = 0;
    }
    if (valorUmbralHum2 > 100) {
      valorUmbralHum2 = 100;
    }

    if (gmtOffset_sec > 43200) {
      gmtOffset_sec = 43200;
    }

    if (gmtOffset_sec < -43200) {
      gmtOffset_sec = -43200;
    }

    if (gmt < -12) {
      gmt = -12;
    }

    if (gmt > 12) {
      gmt = 12;
    }


    maquinaDeEstadosGeneral();
    movimientosCursor();




  }

}




void maquinaDeEstadosGeneral () {

  switch (estadoMaquinaGeneral) {

    case PANTALLA_GENERAL:

      pantallaMenuGeneral();

      if (estadoBotonAbajo == PRESIONADO && cursorPantalla < 3) {
        chequeoCursor = ABAJO;
        ultimoEstadoMaquina = PANTALLA_GENERAL;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (estadoBotonArriba == PRESIONADO && cursorPantalla > 0 ) {
        chequeoCursor = ARRIBA;
        ultimoEstadoMaquina = PANTALLA_GENERAL;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (cursorPantalla == 0 && estadoBotonEnter == PRESIONADO) {
        estadoMaquinaGeneral = ESPERA_GENERAL_UMBRALTEMP;
      }

      if (cursorPantalla == 1 && estadoBotonEnter == PRESIONADO) {
        estadoMaquinaGeneral = ESPERA_GENERAL_UMBRALHUM;
      }

      if (cursorPantalla == 3 && estadoBotonAbajo == PRESIONADO) {
        cursorPantalla = 0;
        estadoMaquinaGeneral = ESPERA_GENERAL_MQTT_GMT;
      }

      if (bmp.readTemperature() > valorUmbralTemp) {
        buzzer();
      }

      if (bmp.readTemperature() < valorUmbralTemp) {
        ledcWrite(BUZZER_CHANNEL, 0);
      }


      break;

    case ESPERA_GENERAL_UMBRALTEMP:

      pantallaMenuGeneral();
      ledcWrite(BUZZER_CHANNEL, 0);

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

      if (estadoBotonEnter == PRESIONADO) {
        chequeoPantallaUmbral = TEMPERATURA;
        estadoMaquinaGeneral = ESPERA_VUELTA_GENERAL;
      }

      if (bmp.readTemperature() < valorUmbralTemp) {
        ledcWrite(BUZZER_CHANNEL, 0);
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
      ledcWrite(BUZZER_CHANNEL, 0);

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

      if (estadoBotonEnter == PRESIONADO) {
        chequeoPantallaUmbral = HUMEDAD;
        estadoMaquinaGeneral = ESPERA_VUELTA_GENERAL;
      }

      if (estadoBotonAbajo == PRESIONADO && cursorPantalla < 1) {
        chequeoCursor = ABAJO;
        ultimoEstadoMaquina = PANTALLA_UMBRAL_HUMEDAD;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (estadoBotonArriba == PRESIONADO && cursorPantalla > 0) {
        chequeoCursor = ARRIBA;
        ultimoEstadoMaquina = PANTALLA_UMBRAL_HUMEDAD;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }


      break;

    case SUMA_UMBRAL_HUMEDAD:

      pantallaUmbralHum();

      if (cursorPantalla == 0 && estadoBotonDerecha == SUELTO ) {
        valorUmbralHum1 += 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }

      if (cursorPantalla == 1 && estadoBotonDerecha == SUELTO ) {
        valorUmbralHum2 += 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }


      break;

    case RESTA_UMBRAL_HUMEDAD:

      pantallaUmbralHum();

      if (cursorPantalla == 0 && estadoBotonIzquierda == SUELTO ) {
        valorUmbralHum1 -= 1;
        estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
      }

      if (cursorPantalla == 1 && estadoBotonIzquierda == SUELTO ) {
        valorUmbralHum2 -= 1;
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

      if (estadoBotonEnter == SUELTO && chequeoPantallaUmbral == TEMPERATURA) {
        preferencesTemp.putInt("memoria", valorUmbralTemp);
        lcd.clear();
        milisPrevios = 0;
        cursorPantalla = 0;
        estadoMaquinaGeneral = PANTALLA_GENERAL;
      }

      if (estadoBotonEnter == SUELTO && chequeoPantallaUmbral == HUMEDAD) {

        preferencesHum1.putInt("memoria2", valorUmbralHum1);
        preferencesHum2.putInt("memoria3", valorUmbralHum2);

        lcd.clear();
        milisPrevios = 0;
        cursorPantalla = 1;
        estadoMaquinaGeneral = PANTALLA_GENERAL;

      }

      break;

    case ESPERA_GENERAL_MQTT_GMT:

      if (estadoBotonAbajo == SUELTO) {
        lcd.clear();
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      break;

    case PANTALLA_MQTT_GMT:

      pantallaMqttGmt();

      if (cursorPantalla == 0 && estadoBotonDerecha == PRESIONADO) {
        estadoMaquinaGeneral = SUMA_MQTT_GMT;
      }

      if (cursorPantalla == 0 && estadoBotonIzquierda == PRESIONADO) {
        estadoMaquinaGeneral = RESTA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonDerecha == PRESIONADO && gmt < 12) {
        estadoMaquinaGeneral = SUMA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonIzquierda == PRESIONADO && gmt > (-12)) {
        estadoMaquinaGeneral = RESTA_MQTT_GMT;
      }

      if (estadoBotonAbajo == PRESIONADO && cursorPantalla < 1) {
        chequeoCursor = ABAJO;
        ultimoEstadoMaquina = PANTALLA_MQTT_GMT;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (estadoBotonArriba == PRESIONADO && cursorPantalla) {
        chequeoCursor = ARRIBA;
        ultimoEstadoMaquina = PANTALLA_MQTT_GMT;
        estadoMaquinaGeneral = MOVIMIENTOS_CURSOR;
      }

      if (cursorPantalla == 0 && estadoBotonArriba == PRESIONADO) {
        estadoMaquinaGeneral = ESPERA_MQTT_GMT_GENERAL;
      }


      break;


    case SUMA_MQTT_GMT:

      pantallaMqttGmt();

      if (cursorPantalla == 0 && estadoBotonDerecha == SUELTO ) {
        intervaloMqtt += 1;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonDerecha == SUELTO ) {
        gmt += 1;
        gmtOffset_sec = gmtOffset_sec + 3600;
        setup_rtc_ntp();
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      break;

    case RESTA_MQTT_GMT:

      if (cursorPantalla == 0 && estadoBotonIzquierda == SUELTO ) {
        intervaloMqtt -= 1;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonIzquierda == SUELTO ) {
        gmt -= 1;
        gmtOffset_sec = gmtOffset_sec - 3600;
        setup_rtc_ntp();
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      break;

    case ESPERA_MQTT_GMT_GENERAL:

      if (estadoBotonArriba == SUELTO) {
        lcd.clear();
        milisPrevios = 0;
        cursorPantalla = 3;
        estadoMaquinaGeneral = PANTALLA_GENERAL;

      }

      break;

    case MOVIMIENTOS_CURSOR:

      if (estadoBotonAbajo == SUELTO && chequeoCursor == ABAJO ) {
        cursorPantalla += 1;

        if (ultimoEstadoMaquina == PANTALLA_GENERAL) {
          estadoMaquinaGeneral = PANTALLA_GENERAL;
        }

        if (ultimoEstadoMaquina == PANTALLA_UMBRAL_HUMEDAD) {
          estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
        }

        if (ultimoEstadoMaquina == PANTALLA_MQTT_GMT) {
          estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
        }

      }

      if (estadoBotonArriba == SUELTO && chequeoCursor == ARRIBA ) {
        cursorPantalla -= 1;

        if (ultimoEstadoMaquina == PANTALLA_GENERAL) {
          estadoMaquinaGeneral = PANTALLA_GENERAL;
        }

        if (ultimoEstadoMaquina == PANTALLA_UMBRAL_HUMEDAD) {
          estadoMaquinaGeneral = PANTALLA_UMBRAL_HUMEDAD;
        }

        if (ultimoEstadoMaquina == PANTALLA_MQTT_GMT) {
          estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
        }

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
      lcd.print("On ");
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

    milisPrevios = milisActuales;


  }


}


void pantallaUmbralTemp() {

  lcd.setCursor(0, 0);
  lcd.print("Umbral Temp: ");
  lcd.print(valorUmbralTemp);

}


void pantallaUmbralHum() {

  lcd.setCursor(0, 0);
  lcd.print("1 Umbral Hum: ");
  lcd.print(valorUmbralHum1);

  lcd.setCursor(0, 1);
  lcd.print("2 Umbral Hum: ");
  lcd.print(valorUmbralHum2);



  if (valorUmbralHum1 < 10) {
    lcd.setCursor(15, 0);
    lcd.print(" ");
  }

  if (valorUmbralHum1 < 100) {
    lcd.setCursor(16, 0);
    lcd.print(" ");
  }

  if (valorUmbralHum2 < 10) {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }

  if (valorUmbralHum2 < 100) {
    lcd.setCursor(16, 1);
    lcd.print(" ");
  }

}

void pantallaMqttGmt() {

  lcd.setCursor(0, 0);
  lcd.print("MQTT: ");
  lcd.print(intervaloMqtt);

  lcd.setCursor(0, 1);
  lcd.print("GMT: ");
  lcd.print(gmt);

  minutos = timeinfo.tm_min;

  if (!getLocalTime(&timeinfo))
  {
    timestamp = rtc.getEpoch() - gmtOffset_sec;
    timeinfo = rtc.getTimeStruct();
    lcd.setCursor(0, 3);
    lcd.print(&timeinfo, "%H:%M");
  }

  else {

    timestamp = time(NULL);
    if (minutos != timeinfo.tm_min) {
      lcd.clear();
    }
    lcd.setCursor(0, 3);
    lcd.print(&timeinfo, "%H:%M");

  }

  if (gmt > (-1) && gmt < 10) {
    lcd.setCursor(6, 1);
    lcd.print("  ");
  }

  if (gmt > (-10) && gmt < 0) {
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }

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

void buzzer() {

  milisActualesBuzzer = millis();

  switch (estadoMaquinaBuzzer) {

    case 0:

      ledcWrite(BUZZER_CHANNEL, 128);

      if ((milisActualesBuzzer - milisPreviosBuzzer) > 1000) {

        milisPreviosBuzzer = milisActualesBuzzer;
        estadoMaquinaBuzzer = 1;

      }

      break;

    case 1:

      ledcWrite(BUZZER_CHANNEL, 0);

      if ((milisActualesBuzzer - milisPreviosBuzzer) > 1000) {
        milisPreviosBuzzer = milisActualesBuzzer;
        estadoMaquinaBuzzer = 0;
      }

      break;

  }

}

void setup_rtc_ntp(void) {

  // init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  timestamp = time(NULL);
  rtc.setTime(timestamp + gmtOffset_sec);
}




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


void Task1code( void * pvParameters ) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    //lecturaTiempoBot();

    if (flagTemperatura == 0) {

      if (temperatura > valorUmbralTemp) {

        flagTemperatura = 1;

        bot.sendMessage(CHAT_ID, "La temperatura supero el valor umbral!!!", "");
      }

    }

    if (flagTemperatura == 1) {


      if (temperatura < valorUmbralTemp) {

        flagTemperatura = 0;

        bot.sendMessage(CHAT_ID, "La temperatura es menor al valor umbral", "");

      }

    }

  }
}

void loop() {
  Serial.println("loop");
}
