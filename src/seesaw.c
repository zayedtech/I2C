// #include "seesaw.h"
// #include "pico/stdlib.h"
// #include <string.h>


// void seesaw_bus_init(uint32_t hz) {
//     i2c_init(NEOTRELLIS_I2C, hz);
//     gpio_set_function(NEOTRELLIS_SDA, GPIO_FUNC_I2C);
//     gpio_set_function(NEOTRELLIS_SCL, GPIO_FUNC_I2C);
//     gpio_pull_up(NEOTRELLIS_SDA);
//     gpio_pull_up(NEOTRELLIS_SCL);
// }


// bool seesaw_write(uint8_t addr, uint8_t module, uint8_t reg,
//                   const uint8_t *data, uint16_t len) {
//     uint8_t hdr[2] = { module, reg };
    
//     if (i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, true) < 0) return false;
//     if (len == 0) { i2c_write_blocking(NEOTRELLIS_I2C, addr, NULL, 0, false); return true; }
//     return i2c_write_blocking(NEOTRELLIS_I2C, addr, data, len, false) >= 0;
// }

// bool seesaw_write_buf(uint8_t addr, uint8_t module, uint8_t reg,
//                   const uint8_t *data, size_t len)
// {
//     // Build one contiguous frame: [module][reg][payload...]
//     size_t total = 2 + len;
//     uint8_t frame[2 + 30];            // BUF never needs >30 data bytes per write
//     frame[0] = module;
//     frame[1] = reg;
//     if (len) memcpy(&frame[2], data, len);

//     printf("[I2C] frame:");
//         for (size_t i = 0; i < total; ++i) printf(" %02X", frame[i]);
//         printf("\n");

//     int wrote = i2c_write_blocking(NEOTRELLIS_I2C, addr, frame, total, false);
//     printf("[I2C] wrote=%d/%u\n", wrote, (unsigned)total);
//     return wrote == (int)total;
// }





// bool seesaw_read(uint8_t addr, uint8_t module, uint8_t reg,
//                  uint8_t *data, uint16_t len) {
//     uint8_t hdr[2] = { module, reg };
//     if (i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, true) < 0) return false;

//     sleep_us(300);
//     return i2c_read_blocking(NEOTRELLIS_I2C, addr, data, len, false) >= 0;
// }


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



bool seesaw_write_buf(uint8_t addr, uint8_t module, uint8_t reg,
                  const uint8_t *data, size_t len)
{
    // Build one contiguous frame: [module][reg][payload...]
    size_t total = 2 + len;
    uint8_t frame[2 + 30];            // BUF never needs >30 data bytes per write
    frame[0] = module;
    frame[1] = reg;
    if (len) memcpy(&frame[2], data, len);

    size_t used = total;           // bytes already in payload
    size_t cap  = 2 + 30;          // payload capacity (NOT including module+reg)
    if (used < cap) {
        memset(&frame[used], 0, cap - used);
    }

    printf("[I2C] frame:");
        for (size_t i = 0; i < total; ++i) printf(" %02X", frame[i]);
        printf("\n");

    int wrote = i2c_write_blocking(NEOTRELLIS_I2C, addr, frame, total, false);
    printf("[I2C] wrote=%d/%u\n", wrote, (unsigned)total);
            for(int j = 0; j < sizeof(frame); j ++)
        {
            printf("frame[%d]: %x\n", j, frame[j]);
        }

    
    return wrote == (int)total;
}





bool seesaw_read(uint8_t addr, uint8_t module, uint8_t reg,
                 uint8_t *data, uint16_t len) {
    uint8_t hdr[2] = { module, reg };
    if (i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, true) < 0) return false;

    sleep_us(300);
    return i2c_read_blocking(NEOTRELLIS_I2C, addr, data, len, false) >= 0;
}