#include <Adafruit_NeoPixel.h> //Подключение библиотеки

#define LED_PIN 6          // Пин для подключения светодиодов
#define BUT_PIN 10         // Пин для подключения кнопки
#define NUMPIXELS 12       // Количество светодиодов

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); //Подключаем работу со светодиодной лентой

int type = 0;  //Режим работы

void but(){ //Если нажата кнопка
  type++;   //Включаем следующий режим
  if(type > 2) type = 0; //Если номер режима больше чем их колличество
}

void setup() {
  attachInterrupt(BUT_PIN, but, RISING); //Подключаем кнопку к 10 порту
}

void loop() {
  //Если номер режима равен требуемому
  if (type == 0) {pixels.clear(); pixels.show(); }//Выключение светодиодов
  if (type == 1) fillCircle(); // Заполнение круга
  if (type == 2) wave(255,0,0); //Бегущая волна
}


void wave(byte r, byte g, byte b) { //бегущая волна
  static int pos = 0; //Позиция волны
  for (int i = 0; i < NUMPIXELS; i++) { //Цикл по всем светодиодам
    pixels.setPixelColor(i, pixels.Color(r* (sin(i + pos) / 2 + 0.5), g* (sin(i + pos) / 2 + 0.5), b * (sin(i + pos) / 2 + 0.5))); // Синий цвет по синусоиде
  }//устанавливаем цвет
  pixels.show(); //Записываем значения в светодиоды
  pos++;//Увеличиваем позицию
  delay(50);// Пауза
}

void fillCircle() {
  static int pos = 0; //Позиция
  pixels.clear(); //Обнуляем значения всех светодиодов
  for (int i = 0; i <= pos; i++) {//Цикл по всем светодиодам до позиции
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // Белый цвет
    pixels.setPixelColor(NUMPIXELS - 1 - i, pixels.Color(255, 255, 255)); // Заполнение с другой стороны
  }
  pixels.show();// Записать данные в светодиоды
  pos = (pos + 1) % (NUMPIXELS / 2); // Увеличиваем позицию
  delay(150); // Задержка для эффекта
}