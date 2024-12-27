#include <AsyncTelegram2.h>
#include <time.h>
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include <SimplePortal.h>
WiFiClientSecure client;  


#define PIN 6              // Пин для подключения светодиодов
#define NUMPIXELS 12       // Количество светодиодов
#define DEBUG_ENABLE true
AsyncTelegram2 myBot(client);
int type = 0;
WebServer server(80);

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid  =  "******";     // SSID WiFi network
const char* pass  =  "*******";     // Password  WiFi network
const char* token =  "********";  // Telegram token

const char* startBotText = R"rawliteral(
Добрйы день я бот для управления звездочкой
Если хочешь включить: /setOn
Если хочешь выключить: /setOff
Режим огня: /setMode fire


  )rawliteral";

// HTML страница
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang='ru-RU'>
<head>
  <meta charset='UTF-8'/>
  <title>ESP32 Color Control</title>
  <style>
    body { text-align: center; }
    input[type=range] { width: 300px; }
  </style>
</head>
<body>
  <h1>Управление цветом светодиода</h1>
  
  <h2>Ручной режим</h2>
  <label>Красный: <input type="range" id="red" min="0" max="255" value="0" onchange="updateColor()"></label><br>
  <label>Зеленый: <input type="range" id="green" min="0" max="255" value="0" onchange="updateColor()"></label><br>
  <label>Синий: <input type="range" id="blue" min="0" max="255" value="0" onchange="updateColor()"></label><br>
  
  <h2>Автоматический режим</h2>
  <label>
    <input type="radio" name="mode" value="rainbow" onchange="setMode()"> Радужная смена цвета
  </label><br>
  <label>
    <input type="radio" name="mode" value="fire" onchange="setMode()"> Имитация огня
  </label><br>
  <label>
    <input type="radio" name="mode" value="running" onchange="setMode()"> Бегущий огонек
  </label><br>
  
  <script>
    function updateColor() {
      var red = document.getElementById("red").value;
      var green = document.getElementById("green").value;
      var blue = document.getElementById("blue").value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setColor?red=" + red + "&green=" + green + "&blue=" + blue, true);
      xhr.send();
    }
    
    function setMode() {
      var mode = document.querySelector('input[name="mode"]:checked').value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setMode?mode=" + mode, true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";


void serverOn(){
    // Обработка запросов
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/setColor", []() {
    type = 0;
    if (server.hasArg("red") && server.hasArg("green") && server.hasArg("blue")) {
      int red = server.arg("red").toInt();
      int green = server.arg("green").toInt();
      int blue = server.arg("blue").toInt();
      for(int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
      }
      pixels.show();
      server.send(200, "text/plain", "Цвет установлен!");
    } else {
      server.send(400, "text/plain", "Неверный запрос");
    }
  });

  server.on("/setMode", []() {
    if (server.hasArg("mode")) {
      String mode = server.arg("mode");
      if (mode == "rainbow") {
        type = 1;
      } else if (mode == "fire") {
        type = 2;
      } else if (mode == "running") {
        type = 3;
      }
      server.send(200, "text/plain", "Режим установлен!");
    } else {
      server.send(400, "text/plain", "Неверный запрос");
    }
  });

  server.begin(); // Запуск сервера
}
void but(){
  type++;
  if(type > 6) {
    type = 0;
    setColor(0, 0, 0);
  }
}
InlineKeyboard myInlineKbd1;
void onPressed(const TBMessage &queryMsg){
  Serial.printf("\nON button pressed (callback);\nQueryId: %s\n\n", queryMsg.callbackQueryID);
  myBot.endQuery(queryMsg, "Light on", true);
}
void setup() {
  Serial.begin(115200);
    //portalRun();  // запустить с таймаутом 60с
  //portalRun(30000); // запустить с кастомным таймаутом
  
  Serial.println(portalStatus());
  // статус: 0 error, 1 connect, 2 ap, 3 local, 4 exit, 5 timeout
  
  if (portalStatus() == SP_SUBMIT) {
    Serial.println(portalCfg.SSID);
    Serial.println(portalCfg.pass);
    // забираем логин-пароль
  }

  Serial.println("\nStarting TelegramBot...");
  attachInterrupt(10, but, RISING); 
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(portalCfg.SSID, portalCfg.pass);
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  serverOn();
  configTzTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  client.setCACert(telegram_cert);
  
  // Set the Telegram bot properies
  myBot.setUpdateTime(2000);
  myBot.setTelegramToken(token);

  // Check if all things are ok
  Serial.print("\nTest Telegram connection... ");
  myBot.begin() ? Serial.println("OK") : Serial.println("NOK");
  char welcome_msg[128];
  snprintf(welcome_msg, 128, "BOT @%s online\n/help all commands avalaible.", myBot.getBotName());
  Serial.println(WiFi.localIP());

  //myInlineKbd1.addButton("/setOn");
  //myInlineKbd1.addButton("Выключить",  "/setOff", KeyboardButtonQuery, onPressed);
  //myInlineKbd1.addButton("Огонь",  "/setMode fire", KeyboardButtonQuery, onPressed);
  myBot.addInlineKeyboard(&myInlineKbd1);
}

void handleNewMessages(TBMessage msg)
{
    String text = msg.text;
    if (text.startsWith("/start"))
    {
        myBot.sendMessage(msg, startBotText, myInlineKbd1);
    }
    if (text.startsWith("/setOn"))
    {
        type = 0;
        setColor(255, 255, 255);
        myBot.sendMessage(msg, "Цвет установлен!");
    }
        if (text.startsWith("/getSite"))
    {
        myBot.sendMessage(msg, WiFi.localIP().toString());
    }
    if (text.startsWith("/setOff"))
    {
        type = 0;
        setColor(0, 0, 0);
        myBot.sendMessage(msg, "Цвет установлен!");
    }
    if (text.startsWith("/setColor"))
    {
        type = 0;
        int red = 0, green = 0, blue = 0;
        sscanf(text.c_str(), "/setColor %d %d %d", &red, &green, &blue);
        setColor(red, green, blue);
        myBot.sendMessage(msg, "Цвет установлен!");
    }

      if (text.startsWith("/setMode")) {
        String mode = text.substring(9); // Извлекаем режим
        if (mode == "rainbow") type = 1;
        else if (mode == "fire") type = 2;
        else if (mode == "running") type = 3;
        else if (mode == "fillCircle") type = 4;
        else if (mode == "theater") type = 5; // Театральный эффект
        else if (mode == "wave") type = 6; // Волноподобный эффект
        myBot.sendMessage(msg, "Режим установлен!", "");
      }
  
}

void setColor(int red, int green, int blue) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }
  pixels.show();
}

void loop() {
server.handleClient();
  if (type == 1) startRainbow();
  if (type == 2) startFire();
  if (type == 3) startRunningLights();
  if (type == 4) fillCircle(); // Заполнение круга
  if (type == 5) theaterChase(); 
  if (type == 6) wave(255,0,0); 

  TBMessage msg;
  if (myBot.getNewMessage(msg)) { 
    handleNewMessages(msg); 

    String message ;
    message += "Message from @";
    message += myBot.getBotName();
    message += ":\n";
    message += msg.text;
    Serial.println(message);
    // echo the received message
    myBot.sendMessage(msg, msg.text);
  }
}
// Функция для радужной смены цвета
void startRainbow() {
  static int hue = 0;
  hue += 1; // Увеличиваем оттенок
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.ColorHSV(hue * 256)); // Устанавливаем цвет по оттенку
  }
  pixels.show();
  delay(50); // Задержка для плавности
}
void fillCircle() {
  static int pos = 0;
  pixels.clear();
  for (int i = 0; i <= pos; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // Белый цвет
    pixels.setPixelColor(NUMPIXELS - 1 - i, pixels.Color(255, 255, 255)); // Заполнение с другой стороны
  }
  pixels.show();
  pos = (pos + 1) % (NUMPIXELS / 2); // Увеличиваем позицию
  delay(150); // Задержка для эффекта
}
// Функция для имитации огня
void startFire() {
  for (int i = 0; i < NUMPIXELS; i++) {
    int flicker = random(0, 255);
    pixels.setPixelColor(i, pixels.Color(flicker, flicker / 2, 0)); // Цвет огня
  }
  pixels.show();
  delay(100); // Задержка для эффекта
}
// Паттерн 5: Театральный эффект
void theaterChase() {
  static int pos = 0;
  for (int i = 0; i < NUMPIXELS; i += 3) {
    pixels.setPixelColor(i + pos, pixels.Color(255, 255, 255)); // Белый цвет
  }
  pixels.show();
  pos = (pos + 1) % 3; // Двигаем позицию
  delay(100);
}

// Паттерн 6: Волноподобный эффект
void wave(byte r, byte g, byte b) { //бегущая волна
  static int pos = 0; //Позиция волны
  for (int i = 0; i < NUMPIXELS; i++) { //Цикл по всем светодиодам
    pixels.setPixelColor(i, pixels.Color(r* (sin(i + pos) / 2 + 0.5), g* (sin(i + pos) / 2 + 0.5), b * (sin(i + pos) / 2 + 0.5))); // Синий цвет по синусоиде
  }
  pixels.show(); //Записываем значения в светодиоды
  pos++;//Увеличиваем позицию
  delay(50);// Пауза
}


// Функция для бегущего огонька
void startRunningLights() {
  static int pos = 0;
  pixels.clear(); // Очищаем предыдущие цвета
  pixels.setPixelColor(pos, pixels.Color(255, 255, 255)); // Устанавливаем белый цвет
  pixels.show();
  pos = (pos + 1) % NUMPIXELS; // Двигаем позицию
  delay(100); // Задержка для эффекта
}