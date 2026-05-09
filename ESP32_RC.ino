#include <ps5Controller.h>
#include <ESP32Servo.h>

Servo esc;
Servo steering;

// ---------------- Pins ----------------
const int ESC_PIN = 18;
const int SERVO_PIN = 19;

// ---------------- ESC ----------------
const int ESC_NEUTRAL = 1500;
const int ESC_FORWARD_MIN = 1550;
const int ESC_FORWARD_MAX = 1700;
const int ESC_REVERSE_MIN = 1450;
const int ESC_REVERSE_MAX = 1300;

// ---------------- SERVO ----------------
const int SERVO_MIN = 750;
const int SERVO_MAX = 1900;
const int SERVO_CENTER = 1350;

bool escArmed = false;
unsigned long armStart = 0;

// ---------------- PS5 ----------------
void onConnect() {
  Serial.println("PS5 Connected!");
}

void onDisconnect() {
  Serial.println("PS5 Disconnected!");
  esc.writeMicroseconds(ESC_NEUTRAL);
  steering.writeMicroseconds(SERVO_CENTER);
  escArmed = false;
}

// Map stick (-128 to 127) to servo range
int mapSteering(int stick) {
  return map(stick, -128, 127, SERVO_MIN, SERVO_MAX);
}

void setup() {
  Serial.begin(115200);

  esc.setPeriodHertz(50);
  esc.attach(ESC_PIN, 1000, 2000);

  steering.setPeriodHertz(50);
  steering.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);

  // Start neutral
  esc.writeMicroseconds(ESC_NEUTRAL);
  steering.writeMicroseconds(SERVO_CENTER);

  armStart = millis();

  ps5.attachOnConnect(onConnect);
  ps5.attachOnDisconnect(onDisconnect);

  ps5.begin("E8:47:3A:6F:4E:DC");

  Serial.println("System starting...");
}

void loop() {

  // --- ARM ESC ---
  if (!escArmed) {
    esc.writeMicroseconds(ESC_NEUTRAL);

    if (millis() - armStart > 3000) {
      escArmed = true;
      Serial.println("ESC Armed");
    }

    delay(20);
    return;
  }

  // --- NOT CONNECTED ---
  if (!ps5.isConnected()) {
    esc.writeMicroseconds(ESC_NEUTRAL);
    steering.writeMicroseconds(SERVO_CENTER);
    delay(20);
    return;
  }

  // ---------- MOTOR ----------
  int r2 = ps5.R2Value();
  int l2 = ps5.L2Value();

  int pulse = ESC_NEUTRAL;

  if (r2 > 10 && l2 <= 10) {
    pulse = map(r2, 0, 255, ESC_FORWARD_MIN, ESC_FORWARD_MAX);
  }
  else if (l2 > 10 && r2 <= 10) {
    pulse = map(l2, 0, 255, ESC_REVERSE_MIN, ESC_REVERSE_MAX);
  }

  esc.writeMicroseconds(pulse);

  // ---------- STEERING ----------
  int stickX = ps5.LStickX();  

  int servoPulse = mapSteering(stickX);
  steering.writeMicroseconds(servoPulse);

  // Debug print
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    lastPrint = millis();
    Serial.print("Steer: ");
    Serial.print(stickX);
    Serial.print("  Servo: ");
    Serial.print(servoPulse);
    Serial.print("  R2: ");
    Serial.print(r2);
    Serial.print("  L2: ");
    Serial.println(l2);
  }

  delay(10);
}