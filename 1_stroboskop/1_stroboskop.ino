#include <Adafruit_NeoPixel.h> //Подключение библиотеки

#define LED_PIN 6          // Пин для подключения светодиодов
#define NUMPIXELS 12       // Количество светодиодов

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); //Подключаем работу со светодиодной лентой

void setup() {
}

void loop() {
  for (int i = 0; i < NUMPIXELS; i++) { //Цикл по всем светодиодам
    pixels.setPixelColor(i, 255, 255, 0); // Желтый цвет
  }
  pixels.show(); // Записываем значения в светодиоды
  delay(500); // Пауза
  pixels.clear(); // Отчищаем все светодиоды
  pixels.show(); // Записываем значения в светодиоды
  delay(500); // Пауза
}

