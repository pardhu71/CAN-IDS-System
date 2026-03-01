//speed demo code 

#include "driver/twai.h"

#define SPEED_CAN_ID 0x101
#define BUTTON_PIN 18

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)5, (gpio_num_t)4, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
  twai_start();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {

    twai_message_t msg;
    msg.identifier = SPEED_CAN_ID;
    msg.extd = 0;
    msg.rtr = 0;
    msg.data_length_code = 1;
    msg.data[0] = 60;   // dummy speed value

    twai_transmit(&msg, pdMS_TO_TICKS(10));
    delay(50);
  }
}