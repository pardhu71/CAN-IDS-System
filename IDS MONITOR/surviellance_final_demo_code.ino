#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "driver/twai.h"

/* ---------- CONFIG ---------- */
#define ADC_PIN 34
#define SAMPLE_SIZE 50

#define SPEED_ID  0x101
#define BRAKE_ID  0x102

// 🔴 FINAL THRESHOLDS (from your training)
#define SPEED_THRESHOLD  350
#define BRAKE_THRESHOLD  350

#define BUZZER_PIN 27
#define ABNORMAL_LIMIT 3

LiquidCrystal_I2C lcd(0x27, 16, 2);

int abnormalCount = 0;

void setup() {

  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("CAN IDS SYSTEM");
  lcd.setCursor(0,1);
  lcd.print("INITIALIZING");

  // CAN init
  twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)5, (gpio_num_t)4, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();

  delay(2000);
  lcd.clear();
  lcd.print("SYSTEM READY");

  Serial.println("=== CAN IDS SURVEILLANCE READY ===");
}

void loop() {

  twai_message_t rx_msg;

  if (twai_receive(&rx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {

    /* -------- PHYSICAL LAYER SAMPLING -------- */
    int maxVal = 0;
    int minVal = 4095;

    for (int i = 0; i < SAMPLE_SIZE; i++) {
      int v = analogRead(ADC_PIN);
      if (v > maxVal) maxVal = v;
      if (v < minVal) minVal = v;
      delayMicroseconds(20);
    }

    int diff = maxVal - minVal;

    /* -------- AUTHENTICATION -------- */
    bool valid = false;
    bool knownID = false;

    if (rx_msg.identifier == SPEED_ID) {
      knownID = true;
      if (diff <= SPEED_THRESHOLD) valid = true;
    }
    else if (rx_msg.identifier == BRAKE_ID) {
      knownID = true;
      if (diff <= BRAKE_THRESHOLD) valid = true;
    }

    if (!valid) abnormalCount++;
    else abnormalCount = 0;

    lcd.clear();

    /* -------- OUTPUT -------- */
    if (valid && knownID) {

      digitalWrite(BUZZER_PIN, LOW);

      if (rx_msg.identifier == SPEED_ID) {
        lcd.setCursor(0,0);
        lcd.print("SPEED ECU     ");
        lcd.setCursor(0,1);
        lcd.print("STATUS: UPDATED");

        Serial.print("[OK] SPEED ECU | STATUS: UPDATED | DIFF=");
        Serial.println(diff);
      }
      else {
        lcd.setCursor(0,0);
        lcd.print("BRAKE ECU     ");
        lcd.setCursor(0,1);
        lcd.print("STATUS: UPDATED");

        Serial.print("[OK] BRAKE ECU | STATUS: UPDATED | DIFF=");
        Serial.println(diff);
      }
    }
    else if (abnormalCount >= ABNORMAL_LIMIT) {

      digitalWrite(BUZZER_PIN, HIGH);

      lcd.setCursor(0,0);
      lcd.print("INTRUSION ALERT");
      lcd.setCursor(0,1);
      lcd.print("UNKNOWN NODE  ");

      Serial.print("[ALERT] UNKNOWN NODE | CAN ID=0x");
      Serial.print(rx_msg.identifier, HEX);
      Serial.print(" | DIFF=");
      Serial.println(diff);
    }
    else {
      digitalWrite(BUZZER_PIN, LOW);
      lcd.setCursor(0,0);
      lcd.print("CHECKING...   ");
    }

    delay(200);
  }
}