
/*
 * 5. Leyendo un boton con antirebote por software y determinando su estado.
 *
 *
 * NOTA: en el ejemplo "anterior" estado guardaba el valor de la variable, como
 * ahora vamos a tener un estado de verdad cambiamos ese nombre de variable a
 * valor para hora usar un estado como estado propiamente dicho.
 */

#define APRETADO    0
#define SUELTO      1
#define APRETANDOLO 2
#define SOLTANDOLO  3


#include <SimpleTimer.h>
#include <Wire.h> // Comunicacion I2C 
#include "RTClib.h" // libreria ADAFRUIT para DS3231 
#include <TimeLib.h>
#include <TimeAlarms.h>

RTC_DS3231 RTC; // creamos el objeto RTC
SimpleTimer timer;

bool REGANDO = false;
unsigned long DELAY = 0;
int RELE = 3;
long DURATION = 35UL * 1000;
const int boton = 2; // Botón asignado en el pin 2.
int   anterior;      // guardamos el estado anterior.
int   valor;         // valor actual del botón.
int   estado;
unsigned long temporizador;
unsigned long tiemporebote = 50;
int _hour = 21;
int _minute = 0;
int _second = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RELE, OUTPUT);
  pinMode(boton,INPUT_PULLUP);
  pinMode(13, OUTPUT); // Vamos a usar el led de la placa como señalización.
  valor    = HIGH;
  anterior = HIGH;

  if (!RTC.begin()) {
    Serial.println("No se encuentra RTC");
    while (1);
  }else{
    Serial.println("MORE FINE");
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = RTC.now();
    setTime(now.hour(), now.minute() , now.second(), now.day(), now.month(), now.year());
    Alarm.alarmRepeat(20, 9, 20, start);
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
  
  REGANDO = true;
  //drawTime();
}

void stops() {
  Serial.println("APAGANDOLO");
  digitalWrite(RELE, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}
  
void loop() {
  // Si el estado es igual a lo leido, la entrada no ha cambiado lo que
  // significa que no hemos apretado el botón (ni lo hemos soltado); asi que
  // tenemos que parar el temporizador.
  if ( valor==digitalRead(2) ) {
    temporizador = 0;
  }
  // Si el valor distinto significa que hemos pulsado/soltado el botón. Ahora
  // tendremos que comprobar el estado del temporizador, si vale 0, significa que
  // no hemos guardado el tiempo en el que sa ha producido el cambio, así que
  // hemos de guardarlo.
  else
  if ( temporizador == 0 ) {
    // El temporizador no está iniciado, así que hay que guardar
    // el valor de millis en él.
    temporizador = millis();
  }
  else
  // El temporizador está iniciado, hemos de comprobar si el
  // el tiempo que deseamos de rebote ha pasado.
  if ( millis()-temporizador > tiemporebote ) {
    // Si el tiempo ha pasado significa que el estado es lo contrario
    // de lo que había, asi pues, lo cambiamos.
    valor = !valor;
  }

  // Ahora comprobamos el estado. Recordad que si el boton vale "1" estará suelto,
  // "0" y el botón estará apretado. Si pasa de "1" a "0" es que lo estamos aprentando
  // y si es al contrario es que lo estamos soltando.
  if ( anterior==LOW  && valor==LOW  ) estado = APRETADO;
  if ( anterior==LOW  && valor==HIGH ) estado = SOLTANDOLO;
  if ( anterior==HIGH && valor==LOW  ) estado = APRETANDOLO;
  if ( anterior==HIGH && valor==HIGH ) estado = SUELTO;

  // Recuerda que hay que guardar el estado anterior.
  anterior = valor;

  // Ahora vamos a ver que podemos hacer con el estado.
  switch ( estado ) {
    //case SUELTO:   Serial.println("LIBRE");  break; // Apagamos el led.
    case APRETANDOLO: Serial.println("Has apretado el botón"); break; // Mandamos un mensaje.
    case APRETADO: digitalWrite(13,HIGH); break; // Encendemos el led.
    case SOLTANDOLO: start(); break; // Mandamos un mensaje.
    default: break;
  }
  if (REGANDO && ((millis() - DELAY) >= DURATION)) {
    REGANDO = false;
    //drawTime();
    stops();
  }
  if(!REGANDO && hour() == _hour && minute() == _minute && second() == _second) {
     start();
  }
    //drawTime(); 
    //Alarm.delay(1000);
}
