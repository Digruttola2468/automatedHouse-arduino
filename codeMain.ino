// ------------------Librerys------------------

//HC-05
#include <SoftwareSerial.h>

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

  HC-05
    VCC → 5v
    GND → GND
    TXD → 10
    RXD → 11

  RELES 4
    IN1 → 40
    IN2 → 38
    IN3 → 36
    IN4 → 34

*/

/*ESP8266
  D6 → DHT11 DATA
  D7 → PIN 9 ARDUINO MEGA
  D8 → PIN 8 ARDUINO MEGA
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

#define PIN_RXD 11
#define PIN_TXD 10

#define PININPUTESP1 9
#define PININPUTESP2 8

// ------------------MFRC522------------------
MFRC522 mfrc522 (SS_PIN, RST_PIN);


// ------------------RTC3231------------------
RTC_DS3231 rtc;


// ------------------DHT11------------------
DHT dht(PINDHT, DHT11);


// ------------------LCD I2C------------------
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7


// ------------------HC-05------------------
SoftwareSerial miBT(PIN_TXD, PIN_RXD);


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
char INDEX;
char SUBINDEX;
char CLAVE[7];
char claveCerradura[7] = "135791";
int INDICE = 0;

//
int temperatura, humedad;

//
byte LecturaUID[4];
byte Usuario1[4] = {0x6C, 0xF2, 0xDD, 0x2B};

//
int reloj[] = {1, 2, 2021, 18, 55}; //{Day, Mounth, Year, Hour, Minute}
int indexReloj = 0;
int indexCursor = 0;

//
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;

//
bool ESTADO1 = false;
bool ESTADO2 = false;

//
char DATO = '0';

//
bool menuFechaHorario = false;
bool menuAlarma = false;

//  openDoorPieza
//secuenciaLeds showDateTimeLCD showTempHumdLCD
void leerTarjeta();
void getESP8266();
void bluetooth();
void openDoorPieza();
void OnOffLed(int PIN);
boolean comparaUID(byte lectura[], byte usuario[]);
void secuenciaLeds();
void showDateTimeLCD();
void showTempHumdLCD();

void setup() {
  Serial.begin(9600); //Inicialize serial port

  if (! rtc.begin()) {       // si falla la inicializacion del modulo
    Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
    //while (1);         // bucle infinito que detiene ejecucion del programa
  }

  dht.begin();
  SPI.begin();
  miBT.begin(38400);

  lcd.setBacklightPin(3, POSITIVE); // puerto P3 de PCF8574 como positivo
  lcd.setBacklight(HIGH);   // habilita iluminacion posterior de LCD
  lcd.begin(20, 4);
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

  pinMode(PININPUTESP1, INPUT);
  pinMode(PININPUTESP2, INPUT);

  digitalWrite(PINRELE_CERRADURA, HIGH);
  digitalWrite(PINRELE_PASILLO, HIGH);
  digitalWrite(PINRELE_PATIO, HIGH);
  digitalWrite(PINRELE_PIEZA, HIGH);

  previousMillis = millis();
}

void loop() {
  currentMillis = millis();   // Obtiene millis
  TECLA = teclado.getKey();   // obtiene tecla presionada y asigna a variable

  leerTarjeta();
  getESP8266();
  bluetooth();


  if (TECLA) {       // comprueba que se haya presionado una tecla

    if (menuFechaHorario) {
      INDEX = TECLA;
      indexCursor++;
      if (TECLA == '#') {
        menuFechaHorario = false;
        lcd.clear();
      }
    } else if (menuAlarma) {
      if (TECLA == '1') {
        Serial.println("Seleccionaste agregar alarma");
      }
      if (TECLA == '2') {
        Serial.println("Seleccionaste Ver Alarmas");
      }
      if (TECLA == '3') {
        menuAlarma = false;
        lcd.clear();
      }
    } else {
      if (TECLA == 'C') {
        OnOffLed(PINRELE_PASILLO);
      } else if (TECLA == 'D') {
        OnOffLed(PINRELE_PATIO);
      } else if (TECLA == 'A') {
        menuFechaHorario = true;
        lcd.clear();
      } else if (TECLA == 'B') {
        menuAlarma = true;
        lcd.clear();
      }
      else {
        CLAVE[INDICE] = TECLA;    // almacena en array la tecla presionada
        INDICE++;       // incrementa indice en uno
        lcd.setCursor((INDICE + 6), 3);
        lcd.print(TECLA);
      }
    }

  }

  if (INDICE == 6) {     // si ya se almacenaron los 6 digitos
    lcd.clear();
    if (!strcmp(CLAVE, claveCerradura)) { // compara clave ingresada con clave maestra
      openDoorPieza();
    }
    else {
      Serial.println("Incorrecta");
    }
    INDICE = 0;
  }

  if (menuFechaHorario) {
    lcd.setCursor(0, 0);
    lcd.print("12/02/2023  12:30:40");

    lcd.setCursor(0, 1);
    lcd.print("Fecha: ");

    lcd.setCursor((indexCursor + 6), 1);
    lcd.print(INDEX);

    /*
      int reloj[] = {1, 2, 2021, 18, 55}; //{Day, Mounth, Year, Hour, Minute}
      int indexReloj = 0;
      int indexCursor = 0;
    */

    lcd.setCursor(0, 2);
    lcd.print("Horario: ");

    lcd.setCursor(0, 3);
    lcd.print("# Cancelar");


    return;
  }

  if (menuAlarma) {
    lcd.setCursor(0, 0);
    lcd.print("Alarmas");
    lcd.setCursor(0, 1);
    lcd.print("1. Agregar Alarma");
    lcd.setCursor(0, 2);
    lcd.print("2. Ver Alarmas");
    lcd.setCursor(0, 3);
    lcd.print("3. Atras");
    return;
  }

  lcd.setCursor(0, 3);
  lcd.print("Clave: ");
  if ( (currentMillis - previousMillis) >= interval) {
    showDateTimeLCD();
    showTempHumdLCD();
    previousMillis = currentMillis;
  }


}

void getESP8266() {
  //
  if (digitalRead(PININPUTESP1) == HIGH) {
    ESTADO1 = true;
  }
  if (digitalRead(PININPUTESP1) == LOW && ESTADO1) {
    OnOffLed(PINRELE_PATIO);
    ESTADO1 = false;
  }

  if (digitalRead(PININPUTESP2) == HIGH) {
    ESTADO2 = true;
  }
  if (digitalRead(PININPUTESP2) == LOW && ESTADO2) {
    OnOffLed(PINRELE_PASILLO);
    ESTADO2 = false;
  }
}

void bluetooth() {
  //  Bluetooth
  if (miBT.available()) {
    DATO = miBT.read();

    if (DATO == '1')
      OnOffLed(PINRELE_PIEZA);

    if (DATO == '2')
      OnOffLed(PINRELE_PATIO);

    if (DATO == '3')
      OnOffLed(PINRELE_PASILLO);

    if (DATO == '4')
      openDoorPieza();

  }
}

void leerTarjeta() {
  //
  if ( mfrc522.PICC_IsNewCardPresent() ) {
    if ( mfrc522.PICC_ReadCardSerial() ) {
      for (byte i = 0; i < mfrc522.uid.size; i++)  // bucle recorre de a un byte por vez el UID
        LecturaUID[i] = mfrc522.uid.uidByte[i];

      if (comparaUID(LecturaUID, Usuario1))   // llama a funcion comparaUID con Usuario1
        openDoorPieza();

      mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta
    }
  }
}

void openDoorPieza() {
  //Abir la puerta de casa
  digitalWrite(PINRELE_CERRADURA, LOW);
  delay(3000);
  digitalWrite(PINRELE_CERRADURA, HIGH);
}

void OnOffLed(int PIN) {
  digitalWrite(PIN, !digitalRead(PIN));
}

boolean comparaUID(byte lectura[], byte usuario[]) {
  for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
    if (lectura[i] != usuario[i])       // si byte de UID leido es distinto a usuario
      return (false);         // retorna falso
  }
  return (true);          // si los 4 bytes coinciden retorna verdadero
}

void secuenciaLeds() {
  digitalWrite(PINLED_R, HIGH);
  delay(1000);
  digitalWrite(PINLED_R, LOW);
  delay(1000);

  digitalWrite(PINLED_G, HIGH);
  delay(1000);
  digitalWrite(PINLED_G, LOW);
  delay(1000);

  digitalWrite(PINLED_B, HIGH);
  delay(1000);
  digitalWrite(PINLED_B, LOW);
  delay(1000);

}

void showDateTimeLCD() {
  DateTime fecha = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print(fecha.day());
  lcd.print("/");
  lcd.print(fecha.month());
  lcd.print("/");
  lcd.print(fecha.year());
  lcd.print("  ");
  lcd.print(fecha.hour());
  lcd.print(":");
  lcd.print(fecha.minute());
  lcd.print(":");
  lcd.print(fecha.second());
  lcd.print("  ");

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
  Serial.print("  ");
}

void showTempHumdLCD() {
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();

  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(temperatura);
  lcd.print("C");
  lcd.print("   ");
  lcd.print("Humd:");
  lcd.print(humedad);
  lcd.print("%");

  Serial.print("Temp:");
  Serial.print(temperatura);
  Serial.print("C");
  Serial.println();
  Serial.print("Humd:");
  Serial.print(humedad);
  Serial.print("%");
}
