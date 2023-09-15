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
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
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
#define ESPERA_GENERAL_MQTT_GMT_11
#define PANTALLA_MQTT_GMT 10
#define ESPERA_MQTT_GMT_GENERAL 11
#define SUMA_MQTT_ 12
#define RESTA_MQTT 13


#define MOVIMIENTOS_CURSOR 10

//GMT Y TIEMPO ENVIO MQTT

#define BOTtoken "6582349263:AAHnC5r8S53ASk3J4RTncCs0LZy2-jA65pY"
#define CHAT_ID "5939693005"

#define MQTT_HOST IPAddress(10, 162, 24, 47)
#define MQTT_PORT 1884
#define MQTT_USERNAME "esp32"
#define MQTT_PASSWORD "mirko15"
#define MQTT_PUB "/esp32/datos_sensores"


Adafruit_BMP280 bmp;

BH1750 lightMeter(0x23);

LiquidCrystal_I2C lcd(0x27, 20, 4);

Preferences preferencesTemp;
Preferences preferencesHum;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
AsyncMqttClient mqttClient;


const char name_device = 1;  ////device numero de grupo 5A 1x siendo x el numero de grupo
///                        5B 2x siendo x el numero de grupo

unsigned long milisegundos; ///valor actual
unsigned long lastMeasure1 = 0; ///variable para contar el tiempo actual
unsigned long lastMeasure2 = 0; ///variable para contar el tiempo actual

const unsigned long interval_envio = 30000;//Intervalo de envio de datos mqtt
const unsigned long interval_leeo =  60000;//Intervalo de lectura de datos y guardado en la cola
int i = 0;

///time
long unsigned int timestamp ;  // hora
const char* ntpServer = "south-america.pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

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
  float H1;///humedad valor entre 0 y 100
  float luz;
  bool Alarma;
} estructura ;

const int valor_max_struct = 1000; ///valor vector de struct
estructura datos_struct [valor_max_struct];///Guardo valores hasta que lo pueda enviar
estructura aux2 ;


//const char* ssid = "ari";
//const char* password = "004367225aa";

const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

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
int valorUmbralHum;

int cursorPantalla;

bool estadoCooler;
bool chequeoCursor;
bool chequeoPantallaUmbral;
bool prendidoBuzzer;
bool flagTemperatura;


float temperatura;

void setup() {


  pinMode(PIN_BOTON_1, INPUT);
  pinMode(PIN_BOTON_2, INPUT);
  pinMode(PIN_BOTON_3, INPUT_PULLUP);
  pinMode(PIN_BOTON_4, INPUT_PULLUP);
  pinMode(PIN_BOTON_5, INPUT_PULLUP);
  pinMode(PIN_RELE_COOLER, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);

  unsigned status;

  status = bmp.begin(0x76);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode.
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering.
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time.


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.sendMessage(CHAT_ID, "¡Conexion establecida entre el ESP y VeckiarBot!", "");

  Wire.begin();

  lightMeter.begin();

  preferencesTemp.begin("memoria", valorUmbralTemp);
  preferencesHum.begin("memoria2", valorUmbralHum);


  valorUmbralTemp = preferencesTemp.getInt("memoria", 0);
  valorUmbralHum = preferencesHum.getInt("memoria2", 0);


  ledcSetup(BUZZER_CHANNEL, 2000, 8); // Configurar el canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // Asociar el pin al canal PWM
  ledcWrite(BUZZER_CHANNEL, 0);

  setupmqtt();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  pantallaMenuGeneralSetUp();

  lcd.setCursor(19, 0);
  lcd.print("*");

}


void loop() {

  milisActuales = millis();

  milisegundos = millis();
  if (milisegundos - lastMeasure1 > interval_envio) {    ////envio el doble de lectura por si falla algun envio
    lastMeasure1 = milisegundos;/// cargo el valor actual de millis
    fun_envio_mqtt();///envio los valores por mqtt
  }
  if (milisegundos - lastMeasure2 > interval_leeo) {
    lastMeasure2 = milisegundos;/// cargo el valor actual de millis
    fun_entra(); ///ingreso los valores a la cola struct
  }

  estadoBotonIzquierda = !digitalRead(PIN_BOTON_1);
  estadoBotonDerecha = digitalRead(PIN_BOTON_2);
  estadoBotonArriba = digitalRead(PIN_BOTON_3);
  estadoBotonAbajo = digitalRead(PIN_BOTON_4);
  estadoBotonEnter = digitalRead(PIN_BOTON_5);

  temperatura = bmp.readTemperature();

  humedad = analogRead(PIN_SENSOR_HUMEDAD);
  humedadPorcentaje = map(humedad, 0, 4095, 100, 0);

  luz = lightMeter.readLightLevel();
  luzPorcentaje = map(luz, 0, 65535, 0, 100);

  if (humedadPorcentaje <= 20) {
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, LOW);
    digitalWrite(PIN_RELE_COOLER, OFF);
    estadoCooler = 0;

  }

  if (humedadPorcentaje > 30 && humedadPorcentaje <= 45) {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, HIGH);
    digitalWrite(PIN_LED_ROJO, LOW);
    digitalWrite(PIN_RELE_COOLER, OFF);
    estadoCooler = 0;
  }

  if (humedadPorcentaje > 45) {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_AMARILLO, LOW);
    digitalWrite(PIN_LED_ROJO, HIGH);
    digitalWrite(PIN_RELE_COOLER, ON);
    estadoCooler = 1;
  }

  if (cursorPantalla < 0) {
    cursorPantalla = 0;
  }

  if (cursorPantalla > 3) {
    cursorPantalla = 3;
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

  maquinaDeEstadosGeneral();
  movimientosCursor();

  lecturaTiempoBot();


}


void maquinaDeEstadosGeneral () {

  switch (estadoMaquinaGeneral) {

    case PANTALLA_GENERAL:

      pantallaMenuGeneral();

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
        milisPrevios = 0;
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
      lcd.print("On ");
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


void lecturaTiempoBot () {

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();

  }


}

void handleNewMessages(int numNewMessages) {


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
  datos_struct[indice_entra].T1 = 20; /// leeo los datos //aca va la funcion de cada sensor
  datos_struct[indice_entra].H1 = 30; //// se puede pasar por un parametro valor entre 0 y 100
  datos_struct[indice_entra].luz = 30;
  datos_struct[indice_entra].Alarma = 1;
  indice_entra++;
  Serial.print("saco valores de la struct ientra");
  Serial.println(indice_entra);
}
