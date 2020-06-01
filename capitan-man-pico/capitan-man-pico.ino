/* ################## Test RTC DS3231 #############################
* Filename: RTC_DS3231_Ej1.ino
* Descripción: Puesta en hora RTC DS3231
* Autor: Jose Mª Morales
* Revisión: 6-04-2017
* Probado: ARDUINO UNO r3 - IDE 1.8.2 (Windows7)
* Web: www.playbyte.es/electronica/
* Licencia: Creative Commons Share-Alike 3.0
* http://creativecommons.org/licenses/by-sa/3.0/deed.es_ES
* ##############################################################
*/

// valores a introducir para configurar el RTC
// ==========================
// ===========================

int RELE = 10;
long DURATION = 30UL * 1000;
int PULSADOR = 9;
int V_PULSADOR = 0;
bool PULSADO = false;
int STATE = 0;
int PREV = 0;

bool REGANDO = false;
unsigned long DELAY = 0;



#include <SimpleTimer.h>
#include <Wire.h> // Comunicacion I2C 
#include "RTClib.h" // libreria ADAFRUIT para DS3231 
#include <TimeLib.h>
#include <TimeAlarms.h>

RTC_DS3231 RTC; // creamos el objeto RTC
SimpleTimer timer;



void setup() {
  pinMode(RELE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PULSADOR, INPUT);
  Serial.begin(9600);
  // Wire.begin(); // Inicia Wire sólo si no se hace dentro de la librería 
  // supone que se usa Wire para comunicar con otros dispositivos, no sólo con el DS3231
  
  
  if (!RTC.begin()) {
    Serial.println("No se encuentra RTC");
    while (1);
  }else{
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = RTC.now();
    setTime(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
    Alarm.alarmRepeat(19, 41, 0, start);
  }
}

void drawTime() {
  String str = "NO ESTOY REGANDO ";
  if(REGANDO) str = "SI ESTOY REGANDO";
  Serial.print(str);
  Serial.print(": ");
  
  Serial.print(hour(), DEC);
  Serial.print(':');
  Serial.print(minute(), DEC);
  Serial.print(':'); 
  Serial.print(second(), DEC);
  
  Serial.println(); 
}
void start () {
  DELAY = millis();   // start delay
  Serial.println("ENCIENDO");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RELE, HIGH);
  digitalWrite(PULSADOR, LOW);REGANDO = true;
 
  
}

void stops() {
  Serial.println("APAGANDOLO");
  digitalWrite(RELE, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PULSADOR, LOW);
}


void loop() {
  
  //if(!REGANDO) {
    V_PULSADOR = digitalRead(PULSADOR);
  //}
  
  if (REGANDO && ((millis() - DELAY) >= DURATION)) {
    REGANDO = false;
    stops();
  }
  if ( (V_PULSADOR == HIGH) && (PREV == LOW)){
    STATE = 1 - STATE;
    delay(340);
  }
  
  
  if(STATE == 1) {
    STATE = 0;
    start();
    while(STATE == 1);
  } 
  PREV = V_PULSADOR;
  drawTime(); 
  Alarm.delay(1000);
}
