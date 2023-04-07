#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <UniversalTelegramBot.h>

char id[] = "anj";
char pass[] = "12345678";

#define BOT_TOKEN "6235407127:AAGvSr4SOF4iV4_NGHGB8IfnXNT9AOVOFk4"

const unsigned long BOT_MTBS = 1000;  // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;  // last time messages' scan has been done

const int PinSensor = A0;
const int led_rendah = D7;
const int led_sedang = D6;
const int led_tinggi = D5;
const int Buzzer = D8;

int air_sedang = 500;
int air_tinggi = 600;

int nilai = 0;

void setup() {
  
  Serial.begin(9600);
  pinMode(led_rendah, OUTPUT);
  pinMode(led_sedang, OUTPUT);
  pinMode(led_tinggi, OUTPUT);
  pinMode(Buzzer, OUTPUT);


  digitalWrite(led_rendah, LOW);
  digitalWrite(led_sedang, LOW);
  digitalWrite(led_tinggi, LOW);
  digitalWrite(Buzzer, LOW);

 configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(id);
  WiFi.begin(id, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}


int bacaSensor() {
  nilai = analogRead(PinSensor);
  return nilai;
}

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/cekstatus") {
      int ketinggian = bacaSensor();
      Serial.print("Ketinggian Air : ");
      Serial.println(ketinggian);
      delay(500);

      if (ketinggian <= 100) {
        Serial.println("Ketinggian Air : Kosong");
        digitalWrite(led_rendah, LOW);
        digitalWrite(led_sedang, LOW);
        digitalWrite(led_tinggi, LOW);
        noTone(Buzzer);
        bot.sendMessage(chat_id, "Ketinggian Air Kosong. \nLED Mati", "");
      }


      else if (ketinggian > 0 && ketinggian <= air_sedang) {
        Serial.println("Ketinggian Air : Rendah");
        digitalWrite(led_rendah, HIGH);
        digitalWrite(led_sedang, LOW);
        digitalWrite(led_tinggi, LOW);
        noTone(Buzzer);
        bot.sendMessage(chat_id, "Ketinggian Air Rendah. \nLED Hijau Menyala", "");
      }

      else if (ketinggian > air_sedang && ketinggian <= air_tinggi) {
        Serial.println("Ketinggian Air : Sedang");
        digitalWrite(led_rendah, LOW);
        digitalWrite(led_sedang, HIGH);
        digitalWrite(led_tinggi, LOW);
        noTone(Buzzer);
        bot.sendMessage(chat_id, "Ketinggian Air Sedang. \nLED Kuning Menyala", "");
      }

      else if (ketinggian > air_tinggi) {
        Serial.println("Ketinggian Air : Tinggi");
        digitalWrite(led_rendah, LOW);
        digitalWrite(led_sedang, LOW);
        digitalWrite(led_tinggi, HIGH);
        tone(Buzzer,HIGH);
        bot.sendMessage(chat_id, "Ketinggian Air Tinggi. \nLED Merah Menyala", "");
      }
    }



    if (text == "/start") {
      String welcome = "Welcome to IoT Water Level Bot\n";
      welcome += "List of Commands:\n\n";
      welcome += "/cekstatus";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void loop() {

  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
