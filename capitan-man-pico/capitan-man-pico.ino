
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



#include <time.h>                       // time() ctime()
#ifdef ESP8266
#include <sys/time.h>                   // struct timeval
#endif
#include <Wire.h> // Comunicacion I2C 
#include "RTClib.h" // libreria ADAFRUIT para DS3231 
//#include <CronAlarms.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 RTC; // creamos el objeto RTC
LiquidCrystal_I2C lcd(0x27,20,4); 

char buf3[20], *_months[] =  {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
bool REGANDO = false;
unsigned long DELAY = 0;
int RELE = 3;
int DURATION = 24;
const int boton = 2; // Botón asignado en el pin 2.
int   anterior;      // guardamos el estado anterior.
int   valor;         // valor actual del botón.
int   estado;
unsigned long temporizador;
unsigned long tiemporebote = 50;
int _hour = 20;
int _minute = 2;
int _second = 40;
const int potPin = A0; //pin A0 to read analog input
int LED = 7;


void setup() {
  Serial.begin(9600);
  pinMode(RELE, OUTPUT);
  pinMode(boton,INPUT_PULLUP);
  pinMode(13, OUTPUT); // Vamos a usar el led de la placa como señalización.
  pinMode(LED , OUTPUT); 
  valor    = HIGH;
  anterior = HIGH;
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("IRRIGATE START");
  lcd.setCursor(7, 1);
  lcd.print("V.1");

  if (!RTC.begin()) {
    Serial.println("No se encuentra RTC");
    while (1);
  }else{
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = RTC.now();
    struct tm tm_newtime; // set time to Saturday 8:29:00am Jan 1 2011
    tm_newtime.tm_year = now.year() - 1900;
    tm_newtime.tm_mon = now.month() - 1;
    tm_newtime.tm_mday = now.day();
    tm_newtime.tm_hour = now.hour();
    tm_newtime.tm_min = now.minute();
    tm_newtime.tm_sec = now.second();
    tm_newtime.tm_isdst = 0;
    #ifdef ESP8266
      timeval tv = { mktime(&tm_newtime), 0 };
      timezone tz = { 0, 0};
      settimeofday(&tv, &tz);
    #elif defined(__AVR__)
      set_zone(0);
      set_dst(0);
      set_system_time( mktime(&tm_newtime) );
    #endif
    lcd.init(); 
    
    //Cron.create("3 51 16 * * *", start, false);  // 8:30am every day
  }
}
String getDigits(int digits){
  // utility function for digital clock display: prints colon and leading 0
  String str = ""; 
  if(digits < 10)
    str = '0';
  return str + (String) digits;
}

void drawTime() {
  #ifdef __AVR__
    system_tick(); // must be implemented at 1Hz
  #endif
  DateTime now = RTC.now();
  
  lcd.setCursor(0,0);
  
  String _h = getDigits(now.hour());
  String _m = getDigits(now.minute());
  String _s = getDigits(now.second());
  String _d = getDigits(now.day());
  String _M = _months[now.month() -1];
  String _y = (String) now.year();
  lcd.print(_d + "/" + _M + "  "+ _h + ":"+ _m + ":" +_s);
  
}
void setDuration() {
  int  _value = analogRead(potPin);          //Read and save analog value from potentiometer
  _value = map(_value, 0, 1023, 10, 70); //Map value 0-1023 to 0-255 (PWM)
  DURATION = _value;
  lcd.setCursor(0, 1);
  lcd.print("dur:" + getDigits(DURATION) + cutdown());
}
void start () {
  Serial.println("ENCIENDO");
  DELAY = millis();   // start delay
  digitalWrite(LED , HIGH); 
  
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RELE, HIGH);
  
  REGANDO = true;
}

void stops() {
  digitalWrite(RELE, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED , LOW);
}

String cutdown() {
  
  String str = "";
  if (REGANDO) {
    int tr = DURATION -  ((millis() - DELAY)/1000);
    str =  "/" + getDigits(tr);
  } else {
    str = "   ";
  }
  return str;
}


void loop() {
  DateTime now = RTC.now();
  if(!REGANDO  && now.minute() == _minute && now.hour() == _hour && now.second() == _second) {
     start();
  }
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
    case APRETADO: digitalWrite(13, HIGH); break; // Encendemos el led.
    case SOLTANDOLO: start(); break; // Mandamos un mensaje.
    default: break;
  }
  if (REGANDO && ((millis() - DELAY) >= (DURATION * 1000))) {
    REGANDO = false;
    stops();
  }
  
  

  drawTime();
  setDuration();
  //Cron.delay();
  
  //Alarm.delay(1000);
}
