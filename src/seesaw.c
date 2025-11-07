#include "seesaw.h"
#include "pico/stdlib.h"
#include <string.h>


void seesaw_bus_init(uint32_t hz) {
    i2c_init(NEOTRELLIS_I2C, hz);
    gpio_set_function(NEOTRELLIS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(NEOTRELLIS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(NEOTRELLIS_SDA);
    gpio_pull_up(NEOTRELLIS_SCL);
}


bool seesaw_write(uint8_t addr, uint8_t module, uint8_t reg,
                  const uint8_t *data, uint16_t len) {
    uint8_t hdr[2] = { module, reg };
    
    if (i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, true) < 0) return false;
    if (len == 0) { i2c_write_blocking(NEOTRELLIS_I2C, addr, NULL, 0, false); return true; }
    return i2c_write_blocking(NEOTRELLIS_I2C, addr, data, len, false) >= 0;
}



bool seesaw_read(uint8_t addr, uint8_t module, uint8_t reg,
                 uint8_t *data, uint16_t len) {
    uint8_t hdr[2] = { module, reg };
    if (i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, true) < 0) return false;

    sleep_us(300);
    return i2c_read_blocking(NEOTRELLIS_I2C, addr, data, len, false) >= 0;
}