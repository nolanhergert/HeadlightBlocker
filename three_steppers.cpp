

#define STEP_FREQUENCY 400 // Hz. Max with instant acceleration, might be able to ease into faster though

#define KEY_DURATION_MS 100
#define PIN_LEFT_MOTOR_DIR 9
#define PIN_LEFT_MOTOR_STEP 8
#define PIN_RIGHT_MOTOR_DIR 7
#define PIN_RIGHT_MOTOR_STEP 6


#define STEP_DURATION_MS ((1000)/STEP_FREQUENCY)


void setup() {
  // initialize the serial port:
  Serial.begin(115200);
  pinMode(PIN_LEFT_MOTOR_STEP, OUTPUT);
  pinMode(PIN_LEFT_MOTOR_DIR, OUTPUT);
  pinMode(PIN_RIGHT_MOTOR_STEP, OUTPUT);
  pinMode(PIN_RIGHT_MOTOR_DIR, OUTPUT);
}

char c;
unsigned long millisEnd = 0;
void loop() {
  if (Serial.available() == 0) {
    return;
  }
  
  c = Serial.read();
  
  if (c == 'a' || c == 's' || c == 'd') {
    millisEnd = millis() + KEY_DURATION_MS;
    Serial.println();
  } else {
    return;
  }
  // wasd direction control, except there's no up/'w' yet
  // DIR PIN -> HIGH = pull in, LOW = spool out
  // except invert the right hand side?
  if (c == 'a') {
    // 'a' = move left
    digitalWrite(PIN_LEFT_MOTOR_DIR, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, LOW);
  } else if (c == 'd') {
    // 'd' = move right
    digitalWrite(PIN_LEFT_MOTOR_DIR, HIGH);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, HIGH);
  } else {
    // 's' = pull apart
    digitalWrite(PIN_LEFT_MOTOR_DIR, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, HIGH);    
  }

  while (millis() < millisEnd) {
    digitalWrite(PIN_LEFT_MOTOR_STEP, HIGH);
    digitalWrite(PIN_RIGHT_MOTOR_STEP, HIGH);
    delay(STEP_DURATION_MS/2);
    digitalWrite(PIN_LEFT_MOTOR_STEP, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_STEP, LOW);
    delay(STEP_DURATION_MS/2);
    Serial.print(c);
  }
}