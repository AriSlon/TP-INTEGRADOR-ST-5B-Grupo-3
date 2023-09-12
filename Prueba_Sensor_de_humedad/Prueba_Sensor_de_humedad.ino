/* Prueba Sensor de humedad Higrometro
   Trabajo Practico Integrador

   Materia: Seminario de Informatica y Telecomunicaciones (ST).
   Grupo: 3.
   Integrantes: Santiago Eulmesekian, Mateo Iadarola, Ariel Rakowszczyk, Santiago Rapetti y Ariel Slonimsqui.
   Profesor: Mirko Veckiardo.

*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define PIN_SENSOR 36

#define BOTtoken "6582349263:AAHnC5r8S53ASk3J4RTncCs0LZy2-jA65pY"
#define CHAT_ID "5939693005"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//const char* ssid = "ari";
//const char* password = "004367225aa";

const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

String mensaje = "La temperatura actual es: ";

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo
float humedad;

void setup() {

  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.sendMessage(CHAT_ID, "¡Conexion establecida entre el ESP y VeckiarBot!", "");
  
}

void loop() {

  humedad = analogRead(PIN_SENSOR);
  Serial.println(humedad);

  lecturaTiempoBot();

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
    if (text == "/humedad_actual") {
      bot.sendMessage(chat_id, (String)humedad, "");
    }

  }

}
