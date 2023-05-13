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



/*Connection modules

 RFID
    RST/Reset   RST          5
    SPI SS      SDA(SS)      53
    SPI MOSI    MOSI         51
    SPI MISO    MISO         50
    SPI SCK     SCK          52
    3.3v                     3.3v
    GND                      GND
    UID: 6C F2 DD 2B

 LCD I2C
    SDA → 20
    SCL → 21
    VCC → 5v (VCC)
    GND → GND

 RTC3231
    SDA → 20
    SCL → 21
    VCC → 5v (VCC)
    GND → GND

 DHT11
    VCC  → 5v
    DATA → 2
    NC   → -
    GND  → GND

 Keypad
    FILAS → 42, 44, 46, 48
    COLUM → 43, 45, 47, 49

 RELES 4
    IN1 → 40
    IN2 → 38
    IN3 → 36
    IN4 → 34
*/



// ------------------PIN modules------------------
#define PINLED_R 37  //PIN LED RED
#define PINLED_G 39  //PIN LED GREEN
#define PINLED_B 41  //PIN LED BLUE

#define RST_PIN  5      // constante para referenciar pin de reset
#define SS_PIN  53      // constante para referenciar pin de slave select

#define PINRELE_PIEZA 40
#define PINRELE_PATIO 38
#define PINRELE_PASILLO 36
#define PINRELE_CERRADURA 34

#define PINDHT 2


// ------------------MFRC522------------------
MFRC522 mfrc522 (SS_PIN, RST_PIN);


// ------------------RTC3231------------------
RTC_DS3231 rtc;


// ------------------DHT11------------------
DHT dht(PINDHT, DHT11);


// ------------------LCD I2C------------------
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7


// ------------------KeyPad------------------
const byte FILAS = 4;     // define numero de filas
const byte COLUMNAS = 4;    // define numero de columnas
char keys[FILAS][COLUMNAS] = {    // define la distribucion de teclas
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinesFilas[FILAS] = {42, 44, 46, 48};   // pines correspondientes a las filas
byte pinesColumnas[COLUMNAS] = {43, 45, 47, 49}; // pines correspondientes a las columnas
Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);  // crea objeto



// ------------------Global Variables------------------
char TECLA;
int temperatura, humedad;


void setup() {
  Serial.begin(9600); //Inicialize serial port

  if (! rtc.begin()) {       // si falla la inicializacion del modulo
    Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
    while (1);         // bucle infinito que detiene ejecucion del programa
  }
  
  dht.begin();
  SPI.begin();

  lcd.setBacklightPin(3, POSITIVE); // puerto P3 de PCF8574 como positivo
  lcd.setBacklight(HIGH);   // habilita iluminacion posterior de LCD
  lcd.begin(16, 2);
  lcd.clear();

  mfrc522.PCD_Init();
  rtc.adjust(DateTime(__DATE__, __TIME__));

  pinMode(PINLED_R, OUTPUT);
  pinMode(PINLED_G, OUTPUT);
  pinMode(PINLED_B, OUTPUT);
  pinMode(PINDHT, OUTPUT);
  pinMode(PINRELE_CERRADURA, OUTPUT);
  pinMode(PINRELE_PASILLO, OUTPUT);
  pinMode(PINRELE_PATIO, OUTPUT);
  pinMode(PINRELE_PIEZA, OUTPUT);
}

void loop() {
  // ...
}

void secuenciaLeds() {
  digitalWrite(PINLED_R,HIGH);
  delay(1000);
  digitalWrite(PINLED_R,LOW);
  delay(1000);

  digitalWrite(PINLED_G,HIGH);
  delay(1000);
  digitalWrite(PINLED_G,LOW);
  delay(1000);

  digitalWrite(PINLED_B,HIGH);
  delay(1000);
  digitalWrite(PINLED_B,LOW);
  delay(1000);

}

void mostrarHoraFecha() {
  DateTime fecha = rtc.now();
  Serial.print(fecha.day());
  Serial.print("/");
  Serial.print(fecha.month());
  Serial.print("/");
  Serial.print(fecha.year());
  Serial.print("  ");
  Serial.print(fecha.hour());
  Serial.print(":");
  Serial.print(fecha.minute());
  Serial.print(":");
  Serial.print(fecha.second());
  Serial.println();
}

void mostrarTemperatura(int temperatura, int humedad) {
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print("°C");
  Serial.print("  ");
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %");
  Serial.println();
}
