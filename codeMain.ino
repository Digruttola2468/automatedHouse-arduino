// ------------------Librerys------------------ 

//LCD → Pantalla LCD con I2C
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

//KeyPad → Teclado
#include <Keypad.h>

//RFID → Sensor Tarjeta
#include <SPI.h>
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

//RTC3231 → Reloj y Alarma
#include <RTClib.h>

//DHT → Sensor Temperatura Humedad
#include <DHT.h>
#include <DHT_U.h>



/*Connection modules

*/



// ------------------PIN modules------------------ 
#define RST_PIN 9
#define SS_PIN 10
#define RELE 2
#define DHTPIN 3


// ------------------MFRC522------------------ 
MFRC522 mfrc522 (SS_PIN, RST_PIN);


// ------------------RTC3231------------------ 
RTC_DS3231 rtc;


// ------------------DHT11------------------ 
DHT dht(DHTPIN, DHT11);


// ------------------LCD I2C------------------ 
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7


// ------------------KeyPad------------------ 
const byte FILAS = 4;     // define numero de filas
const byte COLUMNAS = 4;    // define numero de columnas
char keys[FILAS][COLUMNAS] = {    // define la distribucion de teclas
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinesFilas[FILAS] = {9,8,7,6};   // pines correspondientes a las filas
byte pinesColumnas[COLUMNAS] = {5,4,3,2}; // pines correspondientes a las columnas
Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);  // crea objeto



// ------------------Global Variables------------------ 
char TECLA;
float temperatura,humedad;


void setup() {
  Serial.begin(9600); //Inicialize serial port

  dht.begin();
  SPI.begin();
  lcd.begin(20,4);
  if(!rtc.begin())
    Serial.println("Modulo RTC no encontrado !");

  lcd.clear();
  mfrc522.PCD_Init();
  rtc.adjust(DateTime(__DATE__,__TIME__));
}

void loop() {
  DateTime fecha = rtc.now();
  TECLA = teclado.getKey();

  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();
  
}
