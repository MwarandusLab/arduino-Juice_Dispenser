#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int Relay_1 = 10;  //Solenoid Valve
int Relay_2 = 9;   //Pump
int Relay_3 = 8;
int Call_1 = 0;
int Call_2 = 0;

const int buttonPin1 = 3;
const int buttonPin2 = 19;
const int buttonPin3 = 18;
const int irSensorPin = 2;

volatile boolean buttonState1 = HIGH;          // current state of the button
volatile boolean lastButtonState1 = HIGH;      // previous state of the button
volatile unsigned long lastDebounceTime1 = 0;  // last time the output pin was toggled
volatile unsigned long debounceDelay1 = 50;    // the debounce time; increase if the output flickers
volatile boolean objectDetected = false;

volatile boolean buttonState2 = HIGH;
volatile boolean lastButtonState2 = HIGH;
volatile unsigned long lastDebounceTime2 = 0;
volatile unsigned long debounceDelay2 = 50;

volatile boolean buttonState3 = HIGH;
volatile boolean lastButtonState3 = HIGH;
volatile unsigned long lastDebounceTime3 = 0;
volatile unsigned long debounceDelay3 = 50;

unsigned long tap_10_StartTime = 0;
unsigned long tap_20_StartTime = 0;

unsigned long relayActivationTime = 0;
unsigned long relayDeactivationTime = 0;

enum State {
  IDLE,
  WITH_TUMBLER,
  NO_TUMBLER,
  PAY_10,
  PAY_20,
  PAY_10_DISPLAY,
  PAY_20_DISPLAY,
  WAIT_TUMBLER_10,
  WAIT_TUMBLER_20,
  TRACKER_10,
  TRACKER_20
};

State currentState = IDLE;

LiquidCrystal_I2C lcd(0x3F, 20, 4);

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);

  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);
  pinMode(Relay_3, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(irSensorPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(buttonPin1), buttonInterrupt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonInterrupt2, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin3), buttonInterrupt3, FALLING);
  attachInterrupt(digitalPinToInterrupt(irSensorPin), irSensorInterrupt, CHANGE);

  Serial.println("Intitialization");

  digitalWrite(Relay_2, LOW);

  Serial3.println("AT");  // Test communication
  delay(100);
  Serial3.println("AT+CMGF=1");  // Set SMS mode to text
  delay(100);
  Serial3.println("AT+CNMI=2,2,0,0,0");  // Enable receiving SMS notifications
  delay(1000);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(7, 1);
  lcd.print("SYSTEM");
  lcd.setCursor(3, 2);
  lcd.print("INITIALIZATION");
  delay(2000);
}

void loop() {
  checkButtonPresses();
  switch (currentState) {
    case IDLE:
      Idle();
      break;

    case WITH_TUMBLER:
      withTumbler();
      break;

    case NO_TUMBLER:
      noTumbler();
      break;

    case PAY_10_DISPLAY:
      Display_10();
      break;

    case PAY_20_DISPLAY:
      Display_20();
      break;

    case PAY_10:
      Payment_10();
      break;

    case PAY_20:
      Payment_20();
      break;

    case WAIT_TUMBLER_10:
      irSensorInterrupt();
      break;

    case WAIT_TUMBLER_20:
      irSensorInterrupt();
      break;

    case TRACKER_10:
      irSensorInterrupt_10();
      break;

    case TRACKER_20:
      irSensorInterrupt_20();
      break;
  }
}
void Idle() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("JUICE DISPENSER");
  lcd.setCursor(0, 1);
  lcd.print("1. WITH TUMBLER");
  lcd.setCursor(0, 2);
  lcd.print("2. NO TUMBLER");
  lcd.setCursor(0, 3);
  lcd.print("SELECT ABOVE OPTIONS");
  checkButtonPresses();
  if (buttonState1 == LOW) {
    currentState = WITH_TUMBLER;
  } else if (buttonState2 == LOW) {
    currentState = NO_TUMBLER;
  } else if (buttonState3 == LOW) {
    currentState = IDLE;
    digitalWrite(Relay_1, LOW);
  }
  delay(500);
}
void withTumbler() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("JUICE DISPENSER");
  lcd.setCursor(0, 1);
  lcd.print("1. 1000ML ");
  lcd.setCursor(0, 2);
  lcd.print("2. 500ML ");
  lcd.setCursor(0, 3);
  lcd.print("SELECT ABOVE OPTIONS");
  checkButtonPresses();
  if (buttonState1 == LOW) {
    currentState = PAY_20_DISPLAY;
    Serial.println("currentState = PAY_20_DISPLAY");
  } else if (buttonState2 == LOW) {
    currentState = PAY_10_DISPLAY;
    Serial.println("currentState = PAY_10_DISPLAY");
  } else if (buttonState3 == LOW) {
    currentState = IDLE;
    Serial.println("currentState = IDLE");
    digitalWrite(Relay_1, LOW);
  }
  delay(500);
}
void noTumbler() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("JUICE DISPENSER");
  lcd.setCursor(0, 1);
  lcd.print("1. 1000ML ");
  lcd.setCursor(0, 2);
  lcd.print("2. 500ML ");
  lcd.setCursor(0, 3);
  lcd.print("SELECT ABOVE OPTIONS");
  checkButtonPresses();
  if (buttonState1 == LOW) {
    currentState = PAY_20_DISPLAY;
    Serial.println("currentState = PAY_20_DISPLAY");
  } else if (buttonState2 == LOW) {
    currentState = PAY_10_DISPLAY;
    Serial.println("currentState = PAY_10_DISPLAY");
  } else if (buttonState3 == LOW) {
    currentState = IDLE;
    Serial.println("currentState = IDLE");
  }
  delay(500);
}
void Display_10() {
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("KINDLY PAY KSH 10");
  lcd.setCursor(0, 2);
  lcd.print("WAITING FOR PAYMENT");
  if (buttonState3 == LOW) {
    currentState = IDLE;
    Serial.println("currentState = IDLE");
    digitalWrite(Relay_1, LOW);
  }
  currentState = PAY_10;
  Serial.println("currentState = PAY_10");
  delay(500);
}
void Display_20() {
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("KINDLY PAY KSH 20");
  lcd.setCursor(0, 2);
  lcd.print("WAITING FOR PAYMENT");
  if (buttonState3 == LOW) {
    currentState = IDLE;
    Serial.println("currentState = IDLE");
    digitalWrite(Relay_1, LOW);
  }
  currentState = PAY_20;
  Serial.println("currentState = PAY_20");
  delay(1000);
}

void OpenTap_10() {
  digitalWrite(Relay_1, HIGH);
  delay(5000);
  digitalWrite(Relay_1, LOW);
}

void OpenTap_20() {
  digitalWrite(Relay_1, HIGH);
  delay(10000);
  digitalWrite(Relay_1, LOW);
}


void printRelayDuration() {
  unsigned long relayDuration = relayDeactivationTime - relayActivationTime;
  Serial.print("Relay Duration: ");
  Serial.print(relayDuration);
  Serial.println(" ms");
}

void Payment_10() {
  if (Serial3.available() > 0) {
    String command = Serial3.readStringUntil('\n');  // Read the incoming message until newline character
    Serial.println("Received command: " + command);

    // Process the command and take appropriate action
    if (command.startsWith("10")) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AMOUNT PAID: KSH 10");
      lcd.setCursor(2, 1);
      lcd.print("PAYMENT RECEIVED");
      lcd.setCursor(1, 2);
      lcd.print("PLACE THE TUMBLER");
      lcd.setCursor(3, 3);
      lcd.print("BELOW THE TAP");
      delay(5000);
      currentState = WAIT_TUMBLER_10;
      Serial.println("CurrentState = WAIT_TUMBLER");
    }
    if (buttonState3 == LOW) {
      currentState = IDLE;
      Serial.println("currentState = IDLE");
      digitalWrite(Relay_1, LOW);
    }
    // Delete the processed SMS message
    //Serial3.println("AT+CMGD=1,4");
    delay(1000);
  }
}
void Payment_20() {
  if (Serial3.available() > 0) {
    String command = Serial3.readStringUntil('\n');  // Read the incoming message until newline character
    Serial.println("Received command: " + command);
    // Process the command and take appropriate action
    if (command.startsWith("20")) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AMOUNT PAID: KSH 20");
      lcd.setCursor(2, 1);
      lcd.print("PAYMENT RECEIVED");
      lcd.setCursor(1, 2);
      lcd.print("PLACE THE TUMBLER");
      lcd.setCursor(3, 3);
      lcd.print("BELOW THE TAP");
      delay(5000);
      currentState = WAIT_TUMBLER_20;
      Serial.println("currentState = WAIT_TUMBLER");
    }
    if (buttonState3 == LOW) {
      currentState = IDLE;
      Serial.println("currentState = IDLE");
      digitalWrite(Relay_1, LOW);
    }
    // Delete the processed SMS message
    //Serial3.println("AT+CMGD=1,4");
    delay(1000);
  }
}
void buttonInterrupt1() {
  if (millis() - lastDebounceTime1 > debounceDelay1) {
    buttonState1 = digitalRead(buttonPin1);
    if (buttonState1 == LOW && lastButtonState1 == HIGH) {
      Serial.println("Button 1 Pressed!");
      // Add your command or function call here
    }
    lastButtonState1 = buttonState1;
    lastDebounceTime1 = millis();
  }
}

void buttonInterrupt2() {
  if (millis() - lastDebounceTime2 > debounceDelay2) {
    buttonState2 = digitalRead(buttonPin2);
    if (buttonState2 == LOW && lastButtonState2 == HIGH) {
      Serial.println("Button 2 Pressed!");
      // Add your command or function call here
    }
    lastButtonState2 = buttonState2;
    lastDebounceTime2 = millis();
  }
}

void buttonInterrupt3() {
  if (millis() - lastDebounceTime3 > debounceDelay3) {
    buttonState3 = digitalRead(buttonPin3);
    if (buttonState3 == LOW && lastButtonState3 == HIGH) {
      Serial.println("Button 3 Pressed!");
      // Add your command or function call here
    }
    lastButtonState3 = buttonState3;
    lastDebounceTime3 = millis();
  }
}

void checkButtonPresses() {
  buttonInterrupt1();
  buttonInterrupt2();
  buttonInterrupt3();
}
void irSensorInterrupt_10() {
  if (digitalRead(irSensorPin) == LOW && Call_1 == 0) {
    OpenTap_10();
    objectDetected = true;
    Call_1 = 1;
    currentState = IDLE;
    delay(1000);
  } else {
    //digitalWrite(Relay_1, LOW);
    objectDetected = false;
    Call_1 = 0;
    Serial.println("Object Not Detected!");
    delay(1000);
  }
}
void irSensorInterrupt_20() {
  if (digitalRead(irSensorPin) == LOW && Call_2 == 0) {
    OpenTap_20();
    currentState = IDLE;
    objectDetected = true;
    Call_2 = 1;
    delay(1000);
  } else {
    objectDetected = false;
    Call_1 = 0;
    Serial.println("Object Not Detected!");
    delay(1000);
    
  }
}
void irSensorInterrupt() {
  if (currentState == WAIT_TUMBLER_10) {
    Call_1 = 0;
    irSensorInterrupt_10();
  } else if (currentState == WAIT_TUMBLER_20) {
    Call_2 = 0;
    irSensorInterrupt_20();
  }
}
