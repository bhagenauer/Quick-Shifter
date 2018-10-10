// #define DEBUG



#include <EEPROM.h>

const int extShiftSwitch = 4; //digital input, shorted to D12
const int extCfgSwitch = 5;  //digital input, shorted to A2
const int extShiftLed = 6; //output
const int relayPin = 7; //output
const int cfgBtnPin = A2;  //digital input
const int shiftBtnPin = A3; //digital input
const int shiftLedPin = 10; //output
const int pwrLedPin = 11; //output

bool switchState = HIGH;

long lastDebounceTime1 = 0;
long lastDebounceTime2 = 0;
long lastDebounceTime3 = 0;
long lastDebounceTime4 = 0;
long debounceDelay1 = 400;  // shift debounce delay in ms
long debounceDelay2 = 125;  // cfg button debounce delay
long debounceDelay3 = 0; //leave this as 0
long debounceDelay4 = 250; //debounce shifter.. must not cmd shift for X ms to cmd a second shift
bool debouncetrigger;

int recordDelay = 1; //time interval to record cfg settings in minutes
int delayTime[] = {75, 85, 95, 105, 115, 125, 135, 145, 155, 165, 200}; //ign kill time in ms
int setPoint;  //ignkill time


#ifdef DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif

/*  ignState low = spark on
    relaystate high = spark on
    switchstate low = shift commanded
*/
#define RUN LOW
#define KILL HIGH

void setup() {
  digitalWrite(relayPin, RUN);  //try to keep it from cycling on powerup
  digitalWrite(pwrLedPin, HIGH);  //power led always on
  //  digitalWrite(shiftLedPin, HIGH);

  pinMode(extShiftSwitch, INPUT_PULLUP);
  pinMode(extCfgSwitch, INPUT_PULLUP);
  pinMode(extShiftLed, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(shiftLedPin, OUTPUT);
  pinMode(pwrLedPin, OUTPUT);
  pinMode(shiftBtnPin, INPUT_PULLUP); //normally true
  pinMode(cfgBtnPin, INPUT_PULLUP); //normally true
  setPoint = EEPROM.read(1);  //read setpoint
  //setPoint = 105;  //debugging only
  setPoint = constrain(setPoint, 0, 10);
  debounceDelay3 = recordDelay * 60 * 1000; // #min * 60sec/min * 1000 ms/sec

  for (int i = 1; i < 5; i++) {
    digitalWrite(shiftLedPin, HIGH);
    digitalWrite(pwrLedPin, LOW);
    digitalWrite(extShiftLed, HIGH);
    delay(100);
    digitalWrite(shiftLedPin, LOW);
    digitalWrite(pwrLedPin, HIGH);
    digitalWrite(extShiftLed, LOW);
    delay(100);
  }
  digitalWrite(pwrLedPin, HIGH);


#ifdef DEBUG
  Serial.begin(9600);
#endif
  DEBUG_PRINT("setPoint is: ");
  DEBUG_PRINTLN(delayTime[setPoint]);

}

void loop() {
  //this debounce doesn't seem to work. Still occasionally get a shift while releasing the shift btn after a long hold
   if ( (digitalRead(shiftBtnPin) == HIGH) &&  (digitalRead(extShiftSwitch) == HIGH ) ) {
     if ( (millis() - lastDebounceTime4) > debounceDelay4) {
     lastDebounceTime4 = millis();
     debouncetrigger = LOW;
     }
   }


  if ( (digitalRead(shiftBtnPin) == LOW) ||  (digitalRead(extShiftSwitch) == LOW ) ) {
    if ( ( (millis() - lastDebounceTime1) > debounceDelay1 ) && (debouncetrigger == LOW) ) {
      lastDebounceTime1 = millis();
      debouncetrigger = HIGH;
      switchState = HIGH;
      DEBUG_PRINT("Kill at ");
      DEBUG_PRINTLN(millis());
      digitalWrite(relayPin, KILL);
      digitalWrite(shiftLedPin, HIGH);
      digitalWrite(extShiftLed, HIGH);
      delay(delayTime[setPoint] - (millis() - lastDebounceTime1) );
      //    delay(delayTime[setPoint]);
      DEBUG_PRINTLN(millis());
      digitalWrite(relayPin, RUN);
      digitalWrite(shiftLedPin, LOW);
      digitalWrite(extShiftLed, LOW);

    }
  }

  if ( (digitalRead(cfgBtnPin) == LOW) || ( digitalRead(extCfgSwitch) == LOW ) ) {
    if (( millis() - lastDebounceTime2) > debounceDelay2) {
      lastDebounceTime2 = millis();
      setPoint++;
      if (setPoint >= 10) {
        setPoint = 0;
      }
      DEBUG_PRINT("New setPoint: ");
      DEBUG_PRINTLN(delayTime[setPoint]);
      delay(50);
      for (int i = 0; i <= setPoint; i++) {
        digitalWrite(shiftLedPin, HIGH);
        digitalWrite(extShiftLed, HIGH);
        delay(150);
        digitalWrite(shiftLedPin, LOW);
        digitalWrite(shiftLedPin, LOW);
        delay(150);
      }
      while ( (digitalRead(cfgBtnPin) == LOW) || ( digitalRead(extCfgSwitch) == LOW ) ) {  //wait until btn is released
      }
      EEPROM.update(1, setPoint);
    }
  }
//  if ( (millis() - lastDebounceTime3) > debounceDelay3 ) {
//    lastDebounceTime3 = millis();
//    EEPROM.update(1, setPoint);
//  }


}
