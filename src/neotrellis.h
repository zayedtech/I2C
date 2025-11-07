#pragma once
#include <stdint.h>
#include <stdbool.h>

#define NEOTRELLIS_LED_COUNT   16
#define NEOTRELLIS_BYTES       (NEOTRELLIS_LED_COUNT * 3)

// === Seesaw module IDs (from Adafruit Seesaw) ===
// (Keep these #defines grouped here so you can adjust if needed.)
#define SEESAW_STATUS_BASE     0x00
#define SEESAW_NEOPIXEL_BASE   0x0E
#define SEESAW_KEYPAD_BASE     0x10  // (not used in this minimal LED demo)

// Status registers (common patterns)
#define SEESAW_STATUS_HW_ID    0x01  // read: 1 byte (chip id)
#define SEESAW_STATUS_VERSION  0x02  // read: 4 bytes (fw ver)
#define SEESAW_STATUS_SWRST    0x7F  // write: any byte triggers reset

// NeoPixel registers (typical in Adafruit lib)
#define NEOPIXEL_PIN           0x01  // write: 1 byte (internal pin number)
#define NEOPIXEL_SPEED         0x02  // write: 1=800kHz, 0=400kHz (optional)
#define NEOPIXEL_BUF_LENGTH    0x03  // write: 2 bytes length (MSB,LSB)
#define NEOPIXEL_BUF           0x04  // write: RGB bytes payload
#define NEOPIXEL_SHOW          0x05  // write: any byte to latch/show

bool neotrellis_reset(void);
bool neotrellis_status(uint8_t *hw_id, uint32_t *version);
bool neopixel_begin(uint8_t internal_pin /* usually 3 */);
bool neopixel_set_bulk(const uint8_t *rgb48);
bool neopixel_show(void);

bool neopixel_set_one_and_show(int index, uint8_t r, uint8_t g, uint8_t b);
