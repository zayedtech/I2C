#include "neotrellis.h"
#include "seesaw.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

bool neotrellis_reset(void) {
    uint8_t dum = 0xFF;
    bool ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, &dum, 1);
    sleep_ms(2);                 
    return ok;
}

bool neotrellis_status(uint8_t *hw_id, uint32_t *version) {
    bool ok = true;
    if (hw_id)  ok &= seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, hw_id, 1);
    if (version) {
        uint8_t buf[4];
        ok &= seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4);
        *version = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
    }
    return ok;
}

bool neotrellis_wait_ready(uint32_t timeout_ms) {
    absolute_time_t dl = make_timeout_time_ms(timeout_ms);
    uint8_t id;
    while (!time_reached(dl)) {
        if (seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &id, 1)) {
            if (id == 0x55) return true;   
        }
        sleep_ms(5);
    }
    return false;
}

bool neopixel_begin(uint8_t internal_pin) {
    sleep_ms(100);                             

    uint8_t hw_id1 = 0;
    if (!seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &hw_id1, 1)) {
        printf("Status check #1 failed\n");
        return false;
    }
    printf("Status check #1: HW_ID=0x%02X\n", hw_id1);
    sleep_ms(10);

    uint16_t len = 48;                                // 16 * 3
    uint8_t len_be[2] = { 0x00, 0x30 };
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF_LENGTH, len_be, 2)) {
    printf("BUF_LENGTH write failed\n"); return false;
    } else { printf("BUF length set successfully to 0x%04X (%u)  [MSB=0x%02X LSB=0x%02X]\n", len, len, len_be[0], len_be[1]);}
    sleep_ms(200);

                uint8_t speed = 0x01;  // 800kHz
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SPEED, &speed, 1)) {
        printf("SPEED set fail\n");
        return false;
    }
    printf("SPEED set successfully\n");
    sleep_ms(200);


    uint8_t pin =3 ;  
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_PIN, &pin, 1))  
    { printf("PIN set fail\n");
    } 
    else{
        printf("PIN set succesfully to %d\n", pin);
    }
    sleep_ms(200);


        if (!neotrellis_wait_ready(300)) {
        printf("HW_ID never became 0x55\n");
        return false;
    }
    printf("HW_ID OK (0x55)\n");

    uint8_t probe_id = 0;
    
    bool probe_ok = neotrellis_status(&probe_id, NULL);
    printf("Another status check @0x%02X: %s, HW_ID=0x%02X\n", NEOTRELLIS_ADDR, probe_ok ? "OK" : "FAIL", probe_id);

    return true;
}


bool neopixel_show(void) {
    // Send the header
    uint8_t hdr[2] = { SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW };
    int wrote = i2c_write_blocking(NEOTRELLIS_I2C, NEOTRELLIS_ADDR, hdr, 2, false);
    
    printf("SHOW: wrote=%d (expected 2)\n", wrote);
    
    if (wrote != 2) {
        printf("SHOW command FAILED!\n");
        return false;
    }
    
    sleep_ms(10);  
    return true;
}


#define DBG(fmt, ...)  printf("[NEO] " fmt "\n", ##__VA_ARGS__)

static bool neopixel_buf_write(uint16_t start, const uint8_t *data, size_t len) {
    while (len) {
        size_t n = len > 28 ? 28 : len;  
        uint8_t payload[2 + 28];
        
       
        payload[0] = (uint8_t)(start >> 8) & 0xFF;
        payload[1] = (uint8_t)(start & 0xFF);
        memcpy(&payload[2], data, n);  
        
        size_t total = 2 + n;  
        
       
        bool ok = seesaw_write_buf(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, 
                                   NEOPIXEL_BUF, payload, total);
        if (!ok) {
            return false;
        }
        
        start += (uint16_t)n;
        data  += n;
        len   -= n;
    }
    
    return true;
}

bool neopixel_set_one_and_show(int idx, uint8_t r, uint8_t g, uint8_t b) {
    if ((unsigned)idx >= 16) { printf("idx out of range\n"); return false; }

    uint16_t start = (uint16_t)(idx * 3);
    uint8_t  grb[3] = { g, r, b };           
    

        uint8_t zeros[48] = {0};
neopixel_buf_write(0, zeros, 28);   
sleep_ms(2);
neopixel_buf_write(28, zeros + 28, 20);  
sleep_ms(2);
neopixel_show();

    
    if (!neopixel_buf_write(start, grb, 3)) {
        printf("BUF write failed (idx=%d start=%u)\n", idx, start);
        return false;
    }

    sleep_us(300);   
    int ok = neopixel_show();                        
    return ok;
}




bool neopixel_fill_all_and_show(uint8_t r, uint8_t g, uint8_t b) {
    // 16 pixels * 3 bytes each = 48 bytes
    uint8_t frame[48];


    for (int i = 0; i < 16; ++i) {
        frame[3 * i + 0] = g;
        frame[3 * i + 1] = r;
        frame[3 * i + 2] = b;
    }

    if (!neopixel_buf_write(0, frame, 48)) {
        printf("neopixel_fill_all_and_show: buf_write failed\n");
        return false;
    }


    sleep_us(300);
    int ok = neopixel_show();

    return ok;
}








