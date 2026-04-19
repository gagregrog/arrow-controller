#pragma once

// I2C bus
#define PIN_SDA      21
#define PIN_SCL      22

// PN532 NFC reader
#define PIN_NFC_IRQ   16  // active-low, triggers interrupt on tag detect
#define PIN_NFC_RESET 12  // not wired; PN532 uses power-on reset

// WS2811 LED strip
#define PIN_LEDS 17

// Physical buttons (wired to GND, active low)
#define PIN_BTN_PLAY            23
#define PIN_BTN_STOP             5
#define PIN_BTN_PREVIOUS        19
#define PIN_BTN_NEXT            26
#define PIN_BTN_RESTART         18
#define PIN_BTN_MOPIDY_RESTART  13
