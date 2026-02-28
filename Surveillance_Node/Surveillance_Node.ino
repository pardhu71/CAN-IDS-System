#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "driver/twai.h"

#define ADC_PIN         34
#define SAMPLE_SIZE     50
#define SPEED_ID        0x101
#define BRAKE_ID        0x102
#define SPEED_THRESHOLD 350
#define BRAKE_THRESHOLD 350
#define BUZZER_PIN      27

LiquidCrystal_I2C lcd(0x27, 16, 2);

void sampleBus(int &maxVal, int &minVal) {
  maxVal = 0;
  minVal = 4095;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    int v = analogRead(ADC_PIN);
    if (v > maxVal) maxVal = v;
    if (v < minVal) minVal = v;
    delayMicroseconds(20);
  }
}

void showIdle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CAN IDS SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING... ");
}

void flushBuffer() {
  twai_message_t dummy;
  while (twai_receive(&dummy, pdMS_TO_TICKS(0)) == ESP_OK);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CAN IDS SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("INITIALIZING...");

  twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)5, (gpio_num_t)4, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  delay(2000);
  showIdle();
  Serial.println("=== CAN IDS READY (SPEED + BRAKE) ===");
}

void loop() {

  twai_message_t rx_msg;

  if (twai_receive(&rx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {

    // Sample ADC after message received
    int maxVal, minVal;
    sampleBus(maxVal, minVal);
    int diff = maxVal - minVal;

    Serial.print("[MSG] CAN ID=0x");
    Serial.print(rx_msg.identifier, HEX);
    Serial.print(" | DIFF=");
    Serial.println(diff);

    // STEP 1 — Unknown ID → instant alert no threshold check
    if (rx_msg.identifier != SPEED_ID && rx_msg.identifier != BRAKE_ID) {

      digitalWrite(BUZZER_PIN, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("INTRUSION ALERT");
      lcd.setCursor(0, 1);
      lcd.print("UNKNOWN NODE  ");
      Serial.print("[ALERT] UNKNOWN NODE | CAN ID=0x");
      Serial.println(rx_msg.identifier, HEX);
      delay(3000);
      digitalWrite(BUZZER_PIN, LOW);
      flushBuffer();
      showIdle();
      return;
    }

    // STEP 2 — Known ID → OR logic (ID match OR diff match)
    bool idMatch   = true;
    bool diffMatch = false;

    if (rx_msg.identifier == SPEED_ID) diffMatch = (diff <= SPEED_THRESHOLD);
    if (rx_msg.identifier == BRAKE_ID) diffMatch = (diff <= BRAKE_THRESHOLD);

    bool valid = (idMatch || diffMatch);

    if (valid) {

      digitalWrite(BUZZER_PIN, LOW);
      lcd.clear();

      if (rx_msg.identifier == SPEED_ID) {
        lcd.setCursor(0, 0);
        lcd.print("SPEED ECU     ");
        lcd.setCursor(0, 1);
        lcd.print("STATUS: UPDATED");
        Serial.print("[OK] SPEED ECU | DIFF=");
        Serial.print(diff);
        Serial.print(" | DIFF CHECK=");
        Serial.println(diffMatch ? "PASS" : "FAIL (saved by ID)");
      }
      else if (rx_msg.identifier == BRAKE_ID) {
        lcd.setCursor(0, 0);
        lcd.print("BRAKE ECU     ");
        lcd.setCursor(0, 1);
        lcd.print("STATUS: UPDATED");
        Serial.print("[OK] BRAKE ECU | DIFF=");
        Serial.print(diff);
        Serial.print(" | DIFF CHECK=");
        Serial.println(diffMatch ? "PASS" : "FAIL (saved by ID)");
      }

      delay(1000);
      showIdle();
    }
  }
}
