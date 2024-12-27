#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6 // Пин для подключения светодиодов
#define NUMPIXELS 12 // Количество светодиодов
int type = 0;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// Замените на свои данные Wi-Fi
const char* ssid = "*********"; // Введите ваш SSID
const char* password = "*********"; // Введите ваш пароль

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
void isr(){
  type++;
  if(type > 3) type = 0;
}
void setup() {
  Serial.begin(115200);
  pixels.begin();
  attachInterrupt(10, isr, RISING); 
  // Подключаемся к WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к WiFi...");
  }
  
  Serial.println("Подключено к WiFi!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

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

void loop() {
  server.handleClient(); // Обработка входящих запросов
  // Здесь можно добавить вызовы функций для автоматических режимов
  if (type == 1) startRainbow();
  if (type == 2) startFire();
  if (type == 3) startRunningLights();
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

// Функция для имитации огня
void startFire() {
  for (int i = 0; i < NUMPIXELS; i++) {
    int flicker = random(0, 255);
    pixels.setPixelColor(i, pixels.Color(flicker, flicker / 2, 0)); // Цвет огня
  }
  pixels.show();
  delay(100); // Задержка для эффекта
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
