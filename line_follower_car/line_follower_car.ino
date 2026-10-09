// IR Sensor Pins (A0 = Far Left, A5 = Far Right)
const int ir[6] = {A0, A1, A2, A3, A4, A5};

// L298N Motor Driver Pins
const int IN1 = 8, IN2 = 9;    // Left motor
const int IN3 = 10, IN4 = 11;  // Right motor
const int ENA = 5, ENB = 6;

// Speeds (0-255)
const int BASE_SPEED = 150;
const int SLOW_SPEED = 90;    // inner wheel in gentle turns (must be above motor stall point)
const int TURN_SPEED = 160;

// Sensor polarity: value the sensor outputs when it sees the LINE
// Black line on white floor: usually LOW. If your module gives HIGH on black, set to HIGH.
const int LINE = LOW;

// Search timeout when line is lost (ms)
const unsigned long SEARCH_TIMEOUT = 1500;

int lastDirection = 0;            // -1 = Left, 1 = Right, 0 = Center
unsigned long lostSince = 0;
bool wasLost = false;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 6; i++) pinMode(ir[i], INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopBot();
}

void loop() {
  bool s[6];                       // true = sensor sees line
  int count = 0;
  for (int i = 0; i < 6; i++) {
    s[i] = (digitalRead(ir[i]) == LINE);
    if (s[i]) count++;
  }

  // Optional debug
  // for (int i = 0; i < 6; i++) Serial.print(s[i]);
  // Serial.println();

  // Line lost timer
  if (count == 0) {
    if (!wasLost) { wasLost = true; lostSince = millis(); }
  } else {
    wasLost = false;
  }

  // 1. All sensors on line -> crossing / finish: go straight
  if (count == 6) {
    forward();
  }
  // 2. Centered
  else if (s[2] && s[3]) {
    forward();
    lastDirection = 0;
  }
  // 3. Slight left drift
  else if (s[2]) {
    gentleLeft();
    lastDirection = -1;
  }
  // 4. Slight right drift
  else if (s[3]) {
    gentleRight();
    lastDirection = 1;
  }
  // 5. Sharp left (outer left sensors)
  else if (s[0] || s[1]) {
    sharpLeft();
    lastDirection = -1;
  }
  // 6. Sharp right (outer right sensors)
  else if (s[4] || s[5]) {
    sharpRight();
    lastDirection = 1;
  }
  // 7. Line lost
  else {
    if (millis() - lostSince > SEARCH_TIMEOUT) {
      stopBot();                         // give up after timeout
    } else if (lastDirection == -1) {
      sharpLeft();
    } else if (lastDirection == 1) {
      sharpRight();
    } else {
      forward();                         // gap in a straight line: keep going
    }
  }
}

// ---------------- Motor Control ---------------- //

void setSpeed(int a, int b) {
  analogWrite(ENA, a);
  analogWrite(ENB, b);
}

void forward() {
  setSpeed(BASE_SPEED, BASE_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void gentleLeft() {                      // slow the left wheel
  setSpeed(SLOW_SPEED, BASE_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void gentleRight() {                     // slow the right wheel
  setSpeed(BASE_SPEED, SLOW_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void sharpLeft() {
  setSpeed(TURN_SPEED, TURN_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // left reverse
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // right forward
}

void sharpRight() {
  setSpeed(TURN_SPEED, TURN_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // left forward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // right reverse
}

void stopBot() {
  setSpeed(0, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
