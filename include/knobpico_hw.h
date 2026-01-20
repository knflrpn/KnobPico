#pragma once

// Board / pin configuration for KnobPico (RP2040 / Pico SDK)

#include "hardware/i2c.h"

// -------------------------
// I2C / Encoder
// -------------------------
#define I2C_PORT i2c1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

// AS5600-like encoder address used by original firmware
#define ENCODER_ADDR 0x36

// -------------------------
// WS2812 LED strip
// -------------------------
#define NEOPIXEL_PIN 14

// -------------------------
// Buttons
// -------------------------
#define PIN_LEFT 11
#define PIN_MID 10
#define PIN_RIGHT 9

// -------------------------
// Defaults
// -------------------------
#define DEFAULT_DEBOUNCE_MS 10
#define DEFAULT_LONG_PRESS_MS 400
#define DEFAULT_SCROLL_DIV 24
#define DEFAULT_SCROLL_DIR (-1)

// -------------------------
// Flash
// -------------------------
#ifndef PICO_FLASH_SIZE_BYTES
#error "PICO_FLASH_SIZE_BYTES not defined (board config missing?)"
#endif

#define BIGKNOB_CFG_FLASH_SECTOR_SIZE 4096u
#define BIGKNOB_CFG_FLASH_PAGE_SIZE 256u

// Place config in the last sector of flash
#define BIGKNOB_CFG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - BIGKNOB_CFG_FLASH_SECTOR_SIZE)