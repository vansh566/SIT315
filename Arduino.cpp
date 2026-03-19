// ================= PIN DEFINITIONS =================
#define BUTTON_PIN 8
#define PIR_PIN    2
#define TRIG_PIN   6
#define ECHO_PIN   5
#define LED_PIN    13
#define BUZZER_PIN 9

// ================= FLAGS (ISR SAFE) =================
volatile bool buttonFlag = false;
volatile bool pirFlag    = false;
volatile bool timerFlag  = false;

volatile uint8_t lastPortBState = 0;

// ================= TIMING =================
unsigned long lastUltrasonicRead = 0;
const unsigned long ultrasonicInterval = 1000;

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setupExternalInterrupt();
  setupPinChangeInterrupt();
  setupTimer1();

  Serial.println("System Started");
}

// ================= INTERRUPT SETUP =================

// External Interrupt (PIR)
void setupExternalInterrupt() {
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirISR, RISING);
}

// Pin Change Interrupt (Button - D8 / PB0)
void setupPinChangeInterrupt() {
  cli();

  PCICR  |= (1 << PCIE0);    // Enable PORTB interrupts
  PCMSK0 |= (1 << PCINT0);   // Enable D8 (PB0)

  lastPortBState = PINB;

  sei();
}

// Timer1 Interrupt (~2 sec)
void setupTimer1() {
  cli();

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 31249; // ~2 sec (16MHz, prescaler 1024)

  TCCR1B |= (1 << WGM12);              // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
  TIMSK1 |= (1 << OCIE1A);             // Enable interrupt

  sei();
}

// ================= INTERRUPT SERVICE ROUTINES =================

// Button ISR (Pin Change)
ISR(PCINT0_vect) {
  uint8_t currentState = PINB;
  uint8_t changed = currentState ^ lastPortBState;

  if (changed & (1 << PB0)) {
    if (!(currentState & (1 << PB0))) { // Button pressed (LOW)
      buttonFlag = true;
    }
  }

  lastPortBState = currentState;
}

// PIR ISR
void pirISR() {
  pirFlag = true;
}

// Timer ISR
ISR(TIMER1_COMPA_vect) {
  timerFlag = true;
}

// ================= SENSOR FUNCTION =================

// Ultrasonic Distance
float readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

// ================= MAIN LOOP =================
void loop() {

  handleButton();
  handlePIR();
  handleTimer();
  handleUltrasonic();
}

// ================= LOGIC HANDLERS =================

// Button Logic
void handleButton() {
  if (buttonFlag) {
    buttonFlag = false;

    Serial.println("Button Pressed");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

// PIR Logic
void handlePIR() {
  if (pirFlag) {
    pirFlag = false;

    Serial.println("Motion Detected");
    digitalWrite(LED_PIN, HIGH);
  }
}

// Timer Logic (Buzzer Toggle)
void handleTimer() {
  if (timerFlag) {
    timerFlag = false;

    digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
    Serial.println("Timer Event: Buzzer Toggled");
  }
}

// Ultrasonic Logic
void handleUltrasonic() {
  if (millis() - lastUltrasonicRead >= ultrasonicInterval) {

    lastUltrasonicRead = millis();
    float distance = readUltrasonicDistance();

    if (distance > 0) {
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      if (distance < 20) {
        Serial.println("Object Close (LED ON)");
        digitalWrite(LED_PIN, HIGH);
      }
    } 
    else {
      Serial.println("No echo from sensor");
    }
  }
}
