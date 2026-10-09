// IR Sensor Pins (A0 = Far Left, A5 = Far Right)
const int ir[6] = {A0, A1, A2, A3, A4, A5};

// L298N Motor Driver Pins
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;
const int ENA = 5;
const int ENB = 6;

// Speed Configuration (0 - 255)
const int BASE_SPEED = 140; // Adjust for your motors / battery voltage
const int TURN_SPEED = 160;

// Track last known turn direction (-1 = Left, 1 = Right, 0 = Center)
int lastDirection = 0;

void setup() {
  Serial.begin(9600); // Initialized serial monitor for debugging

  for (int i = 0; i < 6; i++) {
    pinMode(ir[i], INPUT);
  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  setSpeed(BASE_SPEED, BASE_SPEED);
}

void loop() {
  int s[6];
  for (int i = 0; i < 6; i++) {
    s[i] = digitalRead(ir[i]);
  }

  // NOTE: Assuming 0 = Black Line, 1 = White Background
  // If your sensor outputs 1 for Black Line, invert these logic checks!

  // 1. Perfectly Centered on Line
  if ((s[2] == 0 && s[3] == 0) || (s[1] == 0 && s[2] == 0 && s[3] == 0 && s[4] == 0)) {
    forward();
    lastDirection = 0;
  }
  // 2. Slight Left Drift -> Gentle Left Turn
  else if (s[2] == 0 || (s[1] == 0 && s[2] == 0)) {
    gentleLeft();
    lastDirection = -1;
  }
  // 3. Slight Right Drift -> Gentle Right Turn
  else if (s[3] == 0 || (s[3] == 0 && s[4] == 0)) {
    gentleRight();
    lastDirection = 1;
  }
  // 4. Sharp Left Turn (outer left sensors triggered)
  else if (s[0] == 0 || s[1] == 0) {
    sharpLeft();
    lastDirection = -1;
  }
  // 5. Sharp Right Turn (outer right sensors triggered)
  else if (s[4] == 0 || s[5] == 0) {
    sharpRight();
    lastDirection = 1;
  }
  // 6. Line Lost (all sensors on white) -> Search in last known direction
  else if (s[0] == 1 && s[1] == 1 && s[2] == 1 && s[3] == 1 && s[4] == 1 && s[5] == 1) {
    if (lastDirection == -1) {
      sharpLeft();
    } else if (lastDirection == 1) {
      sharpRight();
    } else {
      stopBot();
    }
  }
  // 7. Crossroad / All Black (all sensors on line)
  else if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0 && s[4] == 0 && s[5] == 0) {
    forward();
  }

  // No delay so the car reacts instantaneously
}

// ---------------- Motor Control Functions ---------------- //

void setSpeed(int speedA, int speedB) {
  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);
}

void forward() {
  setSpeed(BASE_SPEED, BASE_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void gentleLeft() {
  setSpeed(BASE_SPEED / 2, BASE_SPEED); // Slow down left wheel
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void gentleRight() {
  setSpeed(BASE_SPEED, BASE_SPEED / 2); // Slow down right wheel
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void sharpLeft() {
  setSpeed(TURN_SPEED, TURN_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); // Left wheel reverse
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);  // Right wheel forward
}

void sharpRight() {
  setSpeed(TURN_SPEED, TURN_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);  // Left wheel forward
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH); // Right wheel reverse
}

void stopBot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
