

#include <Arduino.h>
#include "esp_camera.h"

// AI Thinker ESP32-CAM camera pin mapping
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define UART_TX_PIN       13
#define UART_RX_PIN       14

HardwareSerial ArmSerial(1);

const unsigned long SCAN_INTERVAL_MS = 400;
unsigned long lastScan = 0;

String stableColour = "NONE";
String lastSentColour = "NONE";
int stableFrames = 0;

const int REQUIRED_STABLE_FRAMES = 3;

// Pixel thresholds are starting values and require real-scene testing.
const int MIN_DETECTION_PIXELS = 100;

bool initCamera() {
  camera_config_t config = {};

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t result = esp_camera_init(&config);

  if (result != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", result);
    return false;
  }

  Serial.println("Camera initialized.");
  return true;
}

String detectColour(camera_fb_t *fb) {
  if (!fb || fb->format != PIXFORMAT_RGB565) {
    return "NONE";
  }

  uint32_t redCount = 0;
  uint32_t blueCount = 0;
  uint32_t blackCount = 0;
  uint32_t validCount = 0;

  const uint16_t *pixels =
      reinterpret_cast<const uint16_t *>(fb->buf);

  size_t pixelCount = fb->len / sizeof(uint16_t);

  // Sample every second pixel to reduce processing time.
  for (size_t i = 0; i < pixelCount; i += 2) {
    uint16_t p = pixels[i];

    int r = ((p >> 11) & 0x1F) * 255 / 31;
    int g = ((p >> 5) & 0x3F) * 255 / 63;
    int b = (p & 0x1F) * 255 / 31;

    // Ignore extremely bright highlights.
    if (r > 245 && g > 245 && b > 245) {
      continue;
    }

    validCount++;

    if (r > 100 && r > g * 1.4 && r > b * 1.4) {
      redCount++;
    }
    else if (b > 90 && b > r * 1.3 && b > g * 1.2) {
      blueCount++;
    }
    else if (r < 45 && g < 45 && b < 45) {
      blackCount++;
    }
  }

  if (validCount == 0) {
    return "NONE";
  }

  // Require a minimum number of pixels and a meaningful share
  // of the sampled image to avoid triggering on a few pixels.
  uint32_t threshold = max(
      (uint32_t)MIN_DETECTION_PIXELS,
      validCount / 12
  );

  if (redCount > threshold &&
      redCount > blueCount &&
      redCount > blackCount) {
    return "RED";
  }

  if (blueCount > threshold &&
      blueCount > redCount &&
      blueCount > blackCount) {
    return "BLUE";
  }

  if (blackCount > threshold &&
      blackCount > redCount &&
      blackCount > blueCount) {
    return "BLACK";
  }

  return "NONE";
}

void setup() {
  Serial.begin(115200);

  ArmSerial.begin(
      9600,
      SERIAL_8N1,
      UART_RX_PIN,
      UART_TX_PIN
  );

  Serial.println();
  Serial.println("ESP32-CAM Colour Detection");

  if (!psramFound()) {
    Serial.println(
      "Warning: PSRAM not detected. Camera may fail to initialize."
    );
  }

  if (!initCamera()) {
    Serial.println("Restart board after checking camera setup.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Waiting for a coloured object...");
}

void loop() {
  if (millis() - lastScan < SCAN_INTERVAL_MS) {
    return;
  }

  lastScan = millis();

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera frame capture failed.");
    return;
  }

  String detected = detectColour(fb);
  esp_camera_fb_return(fb);

  Serial.print("Detected: ");
  Serial.println(detected);

  if (detected == "NONE") {
    stableColour = "NONE";
    stableFrames = 0;
    lastSentColour = "NONE";
    return;
  }

  if (detected == stableColour) {
    stableFrames++;
  } else {
    stableColour = detected;
    stableFrames = 1;
  }

  // Send only after consistent detection across several scans.
  if (stableFrames >= REQUIRED_STABLE_FRAMES &&
      detected != lastSentColour) {

    ArmSerial.print("COLOR:");
    ArmSerial.println(detected);

    Serial.print("Command sent: COLOR:");
    Serial.println(detected);

    lastSentColour = detected;
  }
}
