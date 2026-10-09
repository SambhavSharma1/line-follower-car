// IR Sensor Pins (A0 = Far Left, A5 = Far Right)
const int ir[6] = {A0, A1, A2, A3, A4, A5};

// L298N Motor Driver Pins
const int IN1 = 8,  IN2 = 9;    // Left motor
const int IN3 = 10, IN4 = 11;   // Right motor
const int ENA = 5,  ENB = 6;    // PWM speed pins

// Speeds (0-255)
const int BASE_SPEED = 150;
const int SLOW_SPEED = 90;      // inner wheel in gentle turns (must be above motor stall point)
const int TURN_SPEED = 160;

// Value the sensor outputs when it sees the LINE.
// Black line on white floor: usually LOW. If your module gives HIGH on black, use HIGH.
const int LINE = LOW;

// Stop if the line stays lost for this long (ms)
const unsigned long SEARCH_TIMEOUT = 1500;

int lastDirection = 0;          // -1 = Left, 1 = Right, 0 = Center
unsigned long lostSince = 0;
bool wasLost = false;

// ---------------- Motor Control ---------------- //
// Direction is set first, then speed.

void drive(int in1, int in2, int in3, int in4, int speedA, int speedB) {
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
  digitalWrite(IN3, in3);
  digitalWrite(IN4, in4);
  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);
}

void forward()     { drive(HIGH, LOW,  HIGH, LOW,  BASE_SPEED, BASE_SPEED); }
void gentleLeft()  { drive(HIGH, LOW,  HIGH, LOW,  SLOW_SPEED, BASE_SPEED); }  // slow left wheel
void gentleRight() { drive(HIGH, LOW,  HIGH, LOW,  BASE_SPEED, SLOW_SPEED); }  // slow right wheel
void sharpLeft()   { drive(LOW,  HIGH, HIGH, LOW,  TURN_SPEED, TURN_SPEED); }  // left reverse, right forward
void sharpRight()  { drive(HIGH, LOW,  LOW,  HIGH, TURN_SPEED, TURN_SPEED); }  // left forward, right reverse
void stopBot()     { drive(LOW,  LOW,  LOW,  LOW,  0, 0); }

// ---------------- Setup ---------------- //

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 6; i++) pinMode(ir[i], INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopBot();
}

// ---------------- Main Loop ---------------- //

void loop() {
  bool s[6];                    // true = sensor sees line
  int count = 0;
  for (int i = 0; i < 6; i++) {
    s[i] = (digitalRead(ir[i]) == LINE);
    if (s[i]) count++;
  }

  // Optional debug
  // for (int i = 0; i < 6; i++) Serial.print(s[i]);
  // Serial.println();

  // Line-lost timer
  if (count == 0) {
    if (!wasLost) { wasLost = true; lostSince = millis(); }
  } else {
    wasLost = false;
  }

  // Outer-sensor turn detection
  bool leftSharp  = s[0] || (s[1] && !s[2]);
  bool rightSharp = s[5] || (s[4] && !s[3]);

  // 1. Crossing / finish line: go straight
  if (count == 6 || (leftSharp && rightSharp)) {
    forward();
    lastDirection = 0;
  }
  // 2. Sharp turns (checked before the centre sensors)
  else if (leftSharp) {
    sharpLeft();
    lastDirection = -1;
  }
  else if (rightSharp) {
    sharpRight();
    lastDirection = 1;
  }
  // 3. Centred
  else if (s[2] && s[3]) {
    forward();
    lastDirection = 0;
  }
  // 4. Slight drift
  else if (s[2]) {
    gentleLeft();
    lastDirection = -1;
  }
  else if (s[3]) {
    gentleRight();
    lastDirection = 1;
  }
  // 5. Line lost
  else {
    if (millis() - lostSince > SEARCH_TIMEOUT) {
      stopBot();
    } else if (lastDirection == -1) {
      sharpLeft();
    } else if (lastDirection == 1) {
      sharpRight();
    } else {
      forward();                // gap in a straight line: keep going
    }
  }
}
