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
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "time.h"
#include "AsyncMqttClient.h"
#include "time.h"
#include "Arduino.h"

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

#define MQTT_HOST IPAddress(10, 162, 24, 47)
#define MQTT_PORT 1884
#define MQTT_USERNAME "esp32"
#define MQTT_PASSWORD "mirko15"
#define MQTT_PUB "/esp32/datos_sensores"

#define BOTtoken "6582349263:AAHnC5r8S53ASk3J4RTncCs0LZy2-jA65pY"
#define CHAT_ID "5939693005"

Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferencesTemp;
Preferences preferencesHum1;
Preferences preferencesHum2;
Preferences preferencesMqtt1;
Preferences preferencesMqtt2;

ESP32Time rtc;

AsyncMqttClient mqttClient;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

String mensaje = "La temperatura actual es: ";

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

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

int intervaloEnvioMqtt;
int intervaloLecturaMqtt;

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

void pedir_lahora(void); // Declaracion de funcion
void setup_rtc_ntp(void); // Declaracion de funcion


const char name_device = 23;  ////device numero de grupo 5A 1x siendo x el numero de grupo
///                        5B 2x siendo x el numero de grupo

unsigned long milisActualesMqtt; ///valor actual
unsigned long milisPreviosMqtt; ///variable para contar el tiempo actual
unsigned long milisPreviosMqtt2; ///variable para contar el tiempo actual

int i = 0;

int indice_entra = 0; ///variables ingresar a la cola struct
int indice_saca = 0;
bool flag_vacio = 1;

char mqtt_payload[150] ;  /////

TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

typedef struct
{
  long time;
  float T1;///tempe
  int H1;///humedad valor entre 0 y 100
  int luz;
  bool Alarma;
} estructura ;

const int valor_max_struct = 1000; ///valor vector de struct
estructura datos_struct [valor_max_struct];///Guardo valores hasta que lo pueda enviar
estructura aux2 ;



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


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password );
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.sendMessage(CHAT_ID, "¡Conexion establecida entre el ESP y VeckiarBot!", "");
  Serial.println(WiFi.localIP());
  Serial.println();

  Wire.begin();

  lightMeter.begin();

  setup_rtc_ntp();

  setupmqtt();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  preferencesTemp.begin("memoria", valorUmbralTemp);
  preferencesHum1.begin("memoria2", valorUmbralHum1);
  preferencesHum2.begin("memoria3", valorUmbralHum2);
  preferencesMqtt1.begin("memoria4", intervaloEnvioMqtt);
  preferencesMqtt2.begin("memoria5", intervaloLecturaMqtt);


  valorUmbralTemp = preferencesTemp.getInt("memoria", 0);
  valorUmbralHum1 = preferencesHum1.getInt("memoria2", 0);
  valorUmbralHum2 = preferencesHum2.getInt("memoria3", 0);
  intervaloEnvioMqtt = preferencesMqtt1.getInt("memoria4", 0);
  intervaloLecturaMqtt = preferencesMqtt2.getInt("memoria5", 0);

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



void loop() {

  milisActuales = millis();
  milisActualesMqtt = millis();

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
  Serial.println(estadoMaquinaGeneral);

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

  if (milisActualesMqtt - milisPreviosMqtt > intervaloEnvioMqtt) {
    fun_envio_mqtt();
    milisPreviosMqtt = milisActualesMqtt;
  }

  if (milisActualesMqtt - milisPreviosMqtt2 > intervaloLecturaMqtt) {
    fun_entra();
    milisPreviosMqtt2 = milisActualesMqtt;
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

  if (intervaloEnvioMqtt < 0) {
    intervaloEnvioMqtt = 0;
  }

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



  lecturaTiempoBot();
  maquinaDeEstadosGeneral();
  movimientosCursor();


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

      if (cursorPantalla == 1 && estadoBotonDerecha == PRESIONADO) {
        estadoMaquinaGeneral = SUMA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonIzquierda == PRESIONADO) {
        estadoMaquinaGeneral = RESTA_MQTT_GMT;
      }

      if (cursorPantalla == 2 && estadoBotonDerecha == PRESIONADO && gmt < 12) {
        estadoMaquinaGeneral = SUMA_MQTT_GMT;
      }

      if (cursorPantalla == 2 && estadoBotonIzquierda == PRESIONADO && gmt > (-12)) {
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
        intervaloEnvioMqtt += 5000;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonDerecha == SUELTO ) {
        intervaloLecturaMqtt += 5000;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }


      if (cursorPantalla == 2 && estadoBotonDerecha == SUELTO ) {
        gmt += 1;
        gmtOffset_sec = gmtOffset_sec + 3600;
        setup_rtc_ntp();
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      break;

    case RESTA_MQTT_GMT:

      if (cursorPantalla == 0 && estadoBotonIzquierda == SUELTO ) {
        intervaloEnvioMqtt -= 5000;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      if (cursorPantalla == 1 && estadoBotonIzquierda == SUELTO ) {
        intervaloLecturaMqtt -= 5000;
        estadoMaquinaGeneral = PANTALLA_MQTT_GMT;
      }

      if (cursorPantalla == 2 && estadoBotonIzquierda == SUELTO ) {
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
        preferencesMqtt1.putInt("memoria4", intervaloEnvioMqtt);
        preferencesMqtt2.putInt("memoria5", intervaloLecturaMqtt);
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


  if ((milisActuales - milisPrevios) > 2000) {

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
  lcd.print("Envio MQTT: ");
  lcd.print(intervaloEnvioMqtt/1000);

  lcd.setCursor(0, 1);
  lcd.print("Lectura MQTT: ");
  lcd.print(intervaloLecturaMqtt/1000);

  lcd.setCursor(0, 2);
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


void setupmqtt()
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  connectToWifi();
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void fun_envio_mqtt ()
{
  fun_saca ();////veo si hay valores nuevos
  if (flag_vacio == 0) ////si hay los envio
  {
    Serial.print("enviando");
    ////genero el string a enviar
    snprintf (mqtt_payload, 150, "%u&%ld&%.2f&%.2f&%.2f&%u", name_device, aux2.time, aux2.T1, aux2.H1, aux2.luz, aux2.Alarma); //random(10,50)
    aux2.time = 0; ///limpio valores
    aux2.T1 = 0;
    aux2.H1 = 0;
    aux2.luz = 0;
    aux2.Alarma = 0;
    Serial.print("Publish message: ");
    Serial.println(mqtt_payload);
    // Publishes Temperature and Humidity values
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, mqtt_payload);
  }
  else
  {
    Serial.println("no hay valores nuevos");
  }
}



void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void fun_saca () {
  if (indice_saca != indice_entra)
  {
    aux2.time = datos_struct[indice_saca].time;
    aux2.T1 = datos_struct[indice_saca].T1;
    aux2.H1 = datos_struct[indice_saca].H1;
    aux2.luz = datos_struct[indice_saca].luz;
    aux2.Alarma = datos_struct[indice_saca].Alarma;
    flag_vacio = 0;

    Serial.println(indice_saca);
    if (indice_saca >= (valor_max_struct - 1))
    {
      indice_saca = 0;
    }
    else
    {
      indice_saca++;
    }
    Serial.print("saco valores de la struct isaca:");
    Serial.println(indice_saca);
  }
  else
  {
    flag_vacio = 1; ///// no hay datos
  }
  return ;
}


void fun_entra (void)
{
  if (indice_entra >= valor_max_struct)
  {
    indice_entra = 0; ///si llego al maximo de la cola se vuelve a cero
  }
  //////////// timestamp/////// consigo la hora
  Serial.print("> NTP Time:");
  timestamp =  time(NULL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;  //// si no puede conseguir la hora
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  ///////////////////////// fin de consigo la hora
  datos_struct[indice_entra].time = timestamp;
  datos_struct[indice_entra].T1 = temperatura; /// leeo los datos //aca va la funcion de cada sensor
  datos_struct[indice_entra].H1 = humedadPorcentaje; //// se puede pasar por un parametro valor entre 0 y 100
  datos_struct[indice_entra].luz = luzPorcentaje;//estadoCooler;
  datos_struct[indice_entra].Alarma = 1; //estadoCooler;

  indice_entra++;
  Serial.print("saco valores de la struct ientra");
  Serial.println(indice_entra);
}
