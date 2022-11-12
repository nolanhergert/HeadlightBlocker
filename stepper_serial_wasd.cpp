

#define STEP_FREQUENCY 100 // Hz

#define KEY_DURATION_US 10000
#define PIN_LEFT_MOTOR_DIR 5
#define PIN_LEFT_MOTOR_STEP 4
#define PIN_RIGHT_MOTOR_DIR 3
#define PIN_RIGHT_MOTOR_STEP 2


#define STEP_DURATION_US ((1000*1000)/STEP_FREQUENCY)


void setup() {
  // initialize the serial port:
  Serial.begin(115200);
  pinMode(PIN_LEFT_MOTOR_STEP, OUTPUT);
  pinMode(PIN_LEFT_MOTOR_DIR, OUTPUT);
  pinMode(PIN_RIGHT_MOTOR_STEP, OUTPUT);
  pinMode(PIN_RIGHT_MOTOR_DIR, OUTPUT);
}

char c;
unsigned long microsEnd = 0;
void loop() {
  if (Serial.available() == 0) {
    return;
  }
  
  c = Serial.read();
  
  if (c == 'a' || c == 's' || c == 'd') {
    microsEnd = micros() + KEY_DURATION_US;
  } else {
    return;
  }
  // wasd direction control, except there's no up/'w' yet
  // DIR PIN -> HIGH = pull in, LOW = spool out
  // except invert the right hand side?
  if (c == 'a') {
    // 'a' = move left
    digitalWrite(PIN_LEFT_MOTOR_DIR, HIGH);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, HIGH);
  } else if (c == 'd') {
    // 'd' = move right
    digitalWrite(PIN_LEFT_MOTOR_DIR, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, LOW);
  } else {
    // 's' = pull apart
    digitalWrite(PIN_LEFT_MOTOR_DIR, HIGH);
    digitalWrite(PIN_RIGHT_MOTOR_DIR, LOW);    
  }

  while (micros() < microsEnd) {
    digitalWrite(PIN_LEFT_MOTOR_STEP, HIGH);
    digitalWrite(PIN_RIGHT_MOTOR_STEP, HIGH);
    delayMicroseconds(STEP_DURATION_US/2);
    digitalWrite(PIN_LEFT_MOTOR_STEP, LOW);
    digitalWrite(PIN_RIGHT_MOTOR_STEP, LOW);
    delayMicroseconds(STEP_DURATION_US/2);
  }
}