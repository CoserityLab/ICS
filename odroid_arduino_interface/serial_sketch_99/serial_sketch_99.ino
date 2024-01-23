#include <avr/power.h>
#include <avr/sleep.h>

// parametri dello sketch
#define BAUD_RATE 2400
#define NUM_DEVICES 18   // numero massimo di uscite

// macro
#define TO_NUM(x) ((x)-'0')
#define CHECK_RANGE(x) (((x) >= 0) && ((x) <= NUM_DEVICES))
#define ACTIVATE(n) set_value((n), HIGH)
#define DEACTIVATE(n) set_value((n), LOW)

// pin da attivare/disattivare
int pins[NUM_DEVICES] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

// stato della macchina
int state;

// numero del dispositivo
int num;
    
// prototipi delle funzioni
char get_symbol(void);
void set_value(int n, int value);
void sleepNow(void);
void log(String s);

void setup()
{
  int i;
  for (i = 0; i < NUM_DEVICES; i++) {
    pinMode(pins[i], OUTPUT);
  }
  Serial.begin(BAUD_RATE);
  log("Ready");
  state = 0;
}

// gli stati sono:
//                0: iniziale
//                1: è arrivato 'A'
//                2: è arrivato 'D'
//                3: è arrivata la prima cifra (per attivazione)
//                4: è arrivata la prima cifra (per disattivazione)
// gli input sono: 'A', 'D', digit (cifra decimale), - (tutto il resto)
// macchina a stati (stato_iniziale, input) -> stato_finale:
//            (0, 'A')    -> 1
//            (0, 'D')    -> 2
//            (0, -)      -> 0
//            (1, 'A')    -> 1
//            (1, 'D')    -> 2
//            (1, digit)  -> 3
//            (1, -)      -> 0
//            (2, 'A')    -> 1
//            (2, 'D')    -> 2
//            (2, digit)  -> 4
//            (2, -)      -> 0
//            (3, 'A')    -> 1
//            (3, 'D')    -> 2
//            (3, digit)  -> 0 (attiva la linea)
//            (3, -)      -> 0
//            (4, 'A')    -> 1
//            (4, 'D')    -> 2
//            (4, digit)  -> 0 (disattiva la linea)
//            (4, -)      -> 0

void loop()
{
  char c;

  c = get_symbol();
  switch(state) {
    case 0:
      if ('A' == c) {
        state = 1;
      } else if ('D' == c) {
        state = 2;
      } else {
        state = 0;
      }
      break;
    case 1:
      num = 0;
      if ('A' == c) {
        state = 1;
      } else if ('D' == c) {
        state = 2;
      } else if (isDigit(c)) {
        num = TO_NUM(c) * 10;
        state = 3;
      } else {
        state = 0;
      }
      break;
    case 2:
      num = 0;
      if ('A' == c) {
        state = 1;
      } else if ('D' == c) {
        state = 2;
      } else if (isDigit(c)) {
        num = TO_NUM(c) * 10;
        state = 4;
      } else {
        state = 0;
      }
      break;
    case 3:
      if ('A' == c) {
        state = 1;
      } else if ('D' == c) {
        state = 2;
      } else if (isDigit(c)) {
        num += TO_NUM(c);
        ACTIVATE(num);
        state = 0;
      } else {
        state = 0;
      }
      break;
    case 4:
      if ('A' == c) {
        state = 1;
      } else if ('D' == c) {
        state = 2;
      } else if (isDigit(c)) {
        num += TO_NUM(c);
        DEACTIVATE(num);
        state = 0;
      } else {
        state = 0;
      }
      break;
  }
}

char get_symbol(void)
{
  int c;

  while (-1 == (c = Serial.read())) {
    delay(100);
    sleepNow();
    delay(100);
  }
  /*while (-1 == (c = Serial.read()));*/
  return c;
}

void set_value(int n, int value)
{
  int i;
  String s;

  if (CHECK_RANGE(n)) {
    if (0 == n) {
      for (i = 0; i < NUM_DEVICES; i++) {
        digitalWrite(pins[i], value);
      }
      s = "Setting all outputs to ";
    } else {
      digitalWrite(pins[n-1], value);
      s = "Setting output ";
      s.concat(n);
      s.concat(" to ");
    }
    if (HIGH == value) {
      s.concat("HIGH");
    } else {
      s.concat("LOW");
    }
  } else {    
    s = "Value ";
    s.concat(n);
    s.concat(" is out of range");
  }
  log(s);
}

void sleepNow(void)
{
  /*
  *
  * In the avr/sleep.h file, the call names of these sleep modus are to be found:
  *
  * The 5 different modes are:
  * SLEEP_MODE_IDLE -the least power savings
  * SLEEP_MODE_ADC
  * SLEEP_MODE_PWR_SAVE
  * SLEEP_MODE_STANDBY
  * SLEEP_MODE_PWR_DOWN -the most power savings
  *
  */

  set_sleep_mode(SLEEP_MODE_IDLE); // sleep mode is set here
  
  sleep_enable(); // enables the sleep bit in the mcucr register
                  // so sleep is possible. just a safety pin

  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();

  sleep_mode(); // here the device is actually put to sleep!!

  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();  // first thing after waking from sleep:
                    // disable sleep...

  power_all_enable();
}

void log(String s)
{
  /*
 Serial.print(s);
 Serial.print("\n");
 */
}
