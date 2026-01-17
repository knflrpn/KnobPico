#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/i2c.h"

#include "tusb.h"

#include "knobpico_hw.h"
#include "knobpico_hid_task.h"
#include "knobpico_led.h"
#include "knobpico_encoder.h"
#include "knobpico_state.h"
#include "knobpico_time.h"

int main(void) {
  stdio_init_all();

  // I2C init
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA_PIN);
  gpio_pull_up(I2C_SCL_PIN);

  // Init global state
  knobpico_state_init(millis());
  KnobPicoState* st = knobpico_state();

  // GPIO init for buttons
  for (int i = 0; i < 3; i++) {
    gpio_init(st->cfg.buttons[i].pin);
    gpio_set_dir(st->cfg.buttons[i].pin, GPIO_IN);
    gpio_pull_up(st->cfg.buttons[i].pin);
  }

  sleep_ms(10);
  encoder_configure_hysteresis();

  // Launch LED core
  multicore_launch_core1(core1_led_entry);

  // USB stack
  tusb_init();

  while (true) {
    tud_task();
    hid_task();
  }
}
