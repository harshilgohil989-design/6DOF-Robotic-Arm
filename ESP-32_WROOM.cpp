


#include <Arduino.h>
#include <ESP32Servo.h>

#define CAM_RX_PIN 16
#define CAM_TX_PIN 17

const uint8_t SERVO_COUNT = 6;

// Example pins; change to match your actual wiring.
const uint8_t SERVO_PINS[SERVO_COUNT] = {
  13, 14, 18, 19, 21, 22
};

enum Joint {
  BASE = 0,
  SHOULDER,
  ELBOW,
  WRIST,
  WRIST_ROTATION,
  GRIPPER
};

Servo arm[SERVO_COUNT];

// These are starting positions, NOT guaranteed safe for your mechanics.
int currentAngle[SERVO_COUNT] = {
  90, 90, 90, 90, 90, 60
};

const int SERVO_MIN_ANGLE = 10;
const int SERVO_MAX_ANGLE = 170;

String commandBuffer = "";
bool armReady = false;

// Adjust these positions to your actual arm geometry.
struct Pose {
  int base;
  int shoulder;
  int elbow;
  int wrist;
  int rotation;
  int gripper;
};

const Pose HOME_POSE = {90, 90, 90, 90, 90, 60};
const Pose PICK_POSE = {90, 110, 70, 100, 90, 100};

const Pose RED_DROP  = {35, 100, 80, 90, 90, 100};
const Pose BLUE_DROP = {90, 100, 80, 90, 90, 100};
const Pose BLACK_DROP = {145, 100, 80, 90, 90, 100};

void moveJoint(uint8_t joint, int target, int stepDelay = 15) {
  if (joint >= SERVO_COUNT) return;

  target = constrain(target, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

  int direction = (target >= currentAngle[joint]) ? 1 : -1;

  while (currentAngle[joint] != target) {
    currentAngle[joint] += direction;

    arm[joint].write(currentAngle[joint]);
    delay(stepDelay);
  }
}

void movePose(const Pose &pose) {
  int target[SERVO_COUNT] = {
    pose.base,
    pose.shoulder,
    pose.elbow,
    pose.wrist,
    pose.rotation,
    pose.gripper
  };

  // Sequential movement is easier to calibrate.
  for (int i = 0; i < SERVO_COUNT; i++) {
    moveJoint(i, target[i]);
  }
}

void pickAndPlace(const Pose &dropPose, const char *colour) {
  Serial.print("Starting pick and place: ");
  Serial.println(colour);

  // Move above the pick location.
  movePose(HOME_POSE);

  // Open gripper.
  moveJoint(GRIPPER, 100);
  delay(500);

  // Move to the calibrated pick pose.
  movePose(PICK_POSE);
  delay(500);

  // Close gripper around the object.
  moveJoint(GRIPPER, 60);
  delay(700);

  // Lift to a safer transfer pose.
  movePose(HOME_POSE);
  delay(500);

  // Move to the colour's calibrated drop location.
  movePose(dropPose);
  delay(500);

  // Release object.
  moveJoint(GRIPPER, 100);
  delay(700);

  // Return to home.
  movePose(HOME_POSE);

  Serial.println("Sequence finished.");
}

void handleCommand(String command) {
  command.trim();
  command.toUpperCase();

  Serial.print("Received: ");
  Serial.println(command);

  if (command == "COLOR:RED") {
    pickAndPlace(RED_DROP, "RED");
  }
  else if (command == "COLOR:BLUE") {
    pickAndPlace(BLUE_DROP, "BLUE");
  }
  else if (command == "COLOR:BLACK") {
    pickAndPlace(BLACK_DROP, "BLACK");
  }
  else if (command == "HOME") {
    movePose(HOME_POSE);
  }
  else if (command == "STOP") {
    // Stops accepting new commands, but cannot instantly brake servos.
    armReady = false;
    Serial.println("Command processing disabled.");
  }
  else if (command == "START") {
    armReady = true;
    Serial.println("Command processing enabled.");
  }
  else {
    Serial.println("Unknown command.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, CAM_RX_PIN, CAM_TX_PIN);

  Serial.println();
  Serial.println("6-DOF Robotic Arm Controller");
  Serial.println("Check mechanical clearance before testing.");

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  for (int i = 0; i < SERVO_COUNT; i++) {
    arm[i].setPeriodHertz(50);
    arm[i].attach(SERVO_PINS[i], 500, 2500);
    arm[i].write(currentAngle[i]);
  }

  delay(1000);
  armReady = true;

  Serial.println("Ready. Waiting for camera commands.");
}

void loop() {
  while (Serial2.available()) {
    char c = (char)Serial2.read();

    if (c == '\n' || c == '\r') {
      if (commandBuffer.length() > 0) {
        if (armReady) {
          handleCommand(commandBuffer);
        } else {
          Serial.println("Controller disabled. Send START.");
        }

        commandBuffer = "";
      }
    }
    else if (commandBuffer.length() < 40) {
      commandBuffer += c;
    }
    else {
      commandBuffer = "";
      Serial.println("Invalid/oversized command discarded.");
    }
  }
}
