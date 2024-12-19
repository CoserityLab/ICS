#include <avr/power.h>
#include <avr/sleep.h>

// parametri dello sketch
#define BAUD_RATE         2400
#define NUM_DEVICES       18      // numero massimo di uscite
#define WATCHDOG_TIMEOUT  5000    // timeout in millisecondi
#define WATCHDOG_ALARM    0       // pin dell'allarme del watchdog

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

// puntatore alla get_symbol da utilizzare
char (*get_symbol)(void);

// prototipi delle funzioni
char get_symbol_initial(void);
char get_symbol_full(void);
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
  state = -1;
  get_symbol = get_symbol_initial;
}

// gli stati sono:
//                S: iniziale della macchina 
//                0: stato in cui la macchina si aspetta i comandi
//                1: è arrivato 'A'
//                2: è arrivato 'D'
//                3: è arrivata la prima cifra (per attivazione)
//                4: è arrivata la prima cifra (per disattivazione)
// gli input sono: 'A', 'D', digit (cifra decimale), - (tutto il resto)
// macchina a stati (stato_iniziale, input) -> stato_finale:
//            (S, 'S')    -> 0
//            (S, -)      -> S 
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
    case -1: 
      if ('S' == c) {
        get_symbol = get_symbol_full;
        state = 0;
      } else {
        state = -1;
      }
      break;      
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

char get_symbol_full(void)
{
  int c;
  unsigned long timeout;

  timeout = millis();
  while (Serial.available() <= 0) {
    if (millis() - timeout >= WATCHDOG_TIMEOUT) {
      ACTIVATE(WATCHDOG_ALARM); /* attiva l'allarme del watchdog */
    }
  }
  DEACTIVATE(WATCHDOG_ALARM); /* disattiva l'allarme del watchdog */
  c = Serial.read();
  return c;
}

char get_symbol_initial(void)
{
  int c;
  while (Serial.available() <= 0);
  c = Serial.read();
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

void log(String s)
{
  /*
 Serial.print(s);
 Serial.print("\n");
*/
}
