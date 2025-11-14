// #include "neotrellis.h"
// #include "seesaw.h"
// #include "pico/stdlib.h"
// #include <string.h>
// #include <stdio.h>

// bool neotrellis_reset(void) {
//     uint8_t dum = 0xFF;
//     bool ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, &dum, 1);
//     sleep_ms(2);                 
//     return ok;
// }


// bool neotrellis_status(uint8_t *hw_id, uint32_t *version) {
//     bool ok = true;
//     if (hw_id)  ok &= seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, hw_id, 1);
//     if (version) {
//         uint8_t buf[4];
//         ok &= seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4);
//         *version = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
//     }
//     return ok;
// }



// bool neotrellis_wait_ready(uint32_t timeout_ms) {
//     absolute_time_t dl = make_timeout_time_ms(timeout_ms);
//     uint8_t id;
//     while (!time_reached(dl)) {
//         if (seesaw_read(NEOTRELLIS_ADDR, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &id, 1)) {
//             if (id == 0x55) return true;   
//         }
//         sleep_ms(5);
//     }
//     return false;
// }



// bool neopixel_begin(uint8_t internal_pin) {
//     sleep_ms(100);                             

//         uint8_t len_be[2] = { 0x00, 0x30 };
//         uint16_t len = ((uint16_t)len_be[0] << 8) | len_be[1];

//     if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF_LENGTH, len_be, 2)) {
//         printf("BUF_LENGTH write failed\n"); return false;
//     } else { printf("BUF length set successfully to 0x%04X (%u)  [MSB=0x%02X LSB=0x%02X]\n", len, len, len_be[0], len_be[1]);}
//     sleep_ms(200);


//     uint8_t pin = 3;  
//     if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_PIN, &pin, 1))  
//     { printf("PIN set fail\n");
//     } 
//     else{
//         printf("PIN set succesfully to %d\n", pin);
//     }

//         if (!neotrellis_wait_ready(300)) {
//         printf("HW_ID never became 0x55\n");
//         return false;
//     }
//     printf("HW_ID OK (0x55)\n");

//     uint8_t probe_id = 0;
    
//     bool probe_ok = neotrellis_status(&probe_id, NULL);
//     printf("Another status check @0x%02X: %s, HW_ID=0x%02X\n", NEOTRELLIS_ADDR, probe_ok ? "OK" : "FAIL", probe_id);

//     return true;
// }



// bool neopixel_show(void) {
//     bool ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW, NULL, 0);
//     printf("SHOW (0B) -> %s\n", ok ? "OK" : "NACK");
//     if (!ok) {
//         uint8_t one = 1;
//         ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW, &one, 1);
//         printf("SHOW (1B) -> %s\n", ok ? "OK" : "NACK");
//     }

//     sleep_ms(100);
//     return ok;
// }




// #define DBG(fmt, ...)  printf("[NEO] " fmt "\n", ##__VA_ARGS__)





// static bool neopixel_buf_write(uint16_t start, const uint8_t *data, size_t len) {
//     while (len) {
//         size_t n = len > 30 ? 30 : len;          // BUF: 2B addr + up to 30B data = 32 total
//         uint8_t payload[2 + 30];

//         // Start address is BIG-endian: MSB first, then LSB
//         payload[0] = (uint8_t)((start >> 8) & 0xFF);
//         payload[1] = (uint8_t)( start       & 0xFF);
//         memcpy(&payload[2], data, n);

// //         DBG("BUF write: start=%u (0x%04X) bytes=%u", start, start, (unsigned) n);
// //         for (size_t i = 0; i < n; ++i) printf(" %02X", data[i]);  // raw data chunk
// // printf("\n");
// // printf("ADDR BE = %02X %02X\n", payload[0], payload[1]); // 00 00 at start=0



//         for (size_t i = 0; i < (n > 9 ? 9 : n); i++)
//             printf(" %02X", payload[2+i]);
//         if (n) printf("%s", n > 9 ? " ..." : "");
//         printf("\n");
        
        
//         neotrellis_wait_ready(300);
//         bool ok = seesaw_write_buf(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, payload, 2 + n);

//         if (!ok) {
//             DBG("ERROR: BUF write failed (start=%u, n=%u)", start, (unsigned)n);
//             return false;
//         }
//         start += (uint16_t)n;
//         data  += n;
//         len   -= n;
//     }
//     return true;
// }



// bool neopixel_set_one_and_show(int idx, uint8_t r, uint8_t g, uint8_t b) {
//     if (idx < 0 | idx > 15) { DBG("ERROR: idx<0"); return false; }
//     uint16_t start = (uint16_t)(idx * 3);
//     uint8_t rgb[3] = { g, r, b };               // WS2812 uses rgb

//     DBG("Set one: idx=%d -> start=%u  RGB=(%u,%u,%u) rgb=(%u,%u,%u)",
//         idx, start, r,g,b, rgb[0],rgb[1],rgb[2]);

//     if (!neopixel_buf_write(start, rgb, sizeof(rgb))) return false;
//     return neopixel_show();
// }



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

        uint8_t len_be[2] = { 0x00, 0x30 };
        uint16_t len = ((uint16_t)len_be[0] << 8) | len_be[1];

    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF_LENGTH, len_be, 2)) {
        printf("BUF_LENGTH write failed\n"); return false;
    } else { printf("BUF length set successfully to 0x%04X (%u)  [MSB=0x%02X LSB=0x%02X]\n", len, len, len_be[0], len_be[1]);}
    sleep_ms(200);


    uint8_t pin = 3;  
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_PIN, &pin, 1))  
    { printf("PIN set fail\n");
    } 
    else{
        printf("PIN set succesfully to %d\n", pin);
    }

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
    bool ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW, NULL, 0);
    printf("SHOW (0B) -> %s\n", ok ? "OK" : "NACK");
    if (!ok) {
        uint8_t one = 1;
        ok = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW, &one, 1);
        printf("SHOW (1B) -> %s\n", ok ? "OK" : "NACK");
    }

    sleep_ms(100);
    return ok;
}




#define DBG(fmt, ...)  printf("[NEO] " fmt "\n", ##__VA_ARGS__)



static bool neopixel_buf_write(uint16_t start, const uint8_t *data, size_t len) {
    while (len) {
        size_t n = len > 30 ? 30 : len;          // BUF: 2B addr + up to 30B data = 32 total
        uint8_t payload[2 + 30];

        // Start address is BIG-endian: MSB first, then LSB
        payload[0] = (uint8_t)((start >> 8) & 0xFF);
        payload[1] = (uint8_t)( start       & 0xFF);
        memcpy(&payload[2], data, n);



        // for(int j = 0; j < sizeof(payload); j ++)
        // {
        //     printf("payload[%d]: %d\n", j, payload[j]);
        // }

//         DBG("BUF write: start=%u (0x%04X) bytes=%u", start, start, (unsigned) n);
//         for (size_t i = 0; i < n; ++i) printf(" %02X", data[i]);  // raw data chunk
// printf("\n");
// printf("ADDR BE = %02X %02X\n", payload[0], payload[1]); // 00 00 at start=0

        // for (size_t i = 0; i < (n > 9 ? 9 : n); i++)
        //     printf(" %02X", payload[2+i]);
        // if (n) printf("%s", n > 9 ? " ..." : "");
        // printf("\n");
        
        
        //neotrellis_wait_ready(300);
        bool ok = seesaw_write_buf(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, payload, 2 + n);

        if (!ok) {
            DBG("ERROR: BUF write failed (start=%u, n=%u)", start, (unsigned)n);
            return false;
        }
        start += (uint16_t)n;
        data  += n;
        len   -= n;
    }
    return true;
}



bool neopixel_set_one_and_show(int idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx < 0 | idx > 15) { DBG("ERROR: idx<0"); return false; }
    uint16_t start = (uint16_t)(idx * 3);
    uint8_t rgb[6] = { r, g, b, r, g, b };               // WS2812 uses rgb

    DBG("Set one: idx=%d -> start=%u  RGB=(%u,%u,%u) rgb=(%u,%u,%u)",
        idx, start, r,g,b, rgb[0],rgb[1],rgb[2] );

    if (!neopixel_buf_write(start, rgb, sizeof(rgb))) return false;
    return neopixel_show();
}








// // Funky key codes from the Adafruit sheet (row-major on 4x4)
// static const uint8_t trellis_keycodes[16] = {
//     0,1,2,3, 8,9,10,11, 16,17,18,19, 24,25,26,27
// };

// // Build a fast reverse map: code(0..27) -> idx(0..15), else 0xFF
// static inline uint8_t keycode_to_idx(uint8_t code) {
//     // Small table (size 28) for O(1) decode; fill on first use
//     static uint8_t lut[28] = { [0 ... 27] = 0xFF };
//     static bool init = false;
//     if (!init) {
//         for (uint8_t i = 0; i < 16; i++) lut[trellis_keycodes[i]] = i;
//         init = true;
//     }
//     return (code < 28) ? lut[code] : 0xFF;
// }
// bool trellis_keypad_begin(void) {
//     uint8_t one = 1, both_edges = 0x03;
//     // Enable scanner
//     if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_KEYPAD_BASE, KEYPAD_ENABLE, &one, 1))
//         return false;

//     // Enable event generation into FIFO
//     if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_KEYPAD_BASE, KEYPAD_INTEN, &both_edges, 1))
//         return false;

//     return true;
// }
// bool trellis_read_event(uint8_t *idx, bool *pressed) {
//     uint8_t cnt = 0;
//     if (!seesaw_read(NEOTRELLIS_ADDR, SEESAW_KEYPAD_BASE, KEYPAD_COUNT, &cnt, 1)) {
//         DKEY("COUNT read failed");
//         return false;
//     }
//     if (cnt < 4) {
//         // Uncomment if you want to see idle polling:
//         // DKEY("COUNT=%u (<4) no full event yet", cnt);
//         return false;
//     }

//     uint8_t evt[4] = {0};
//     if (!seesaw_read(NEOTRELLIS_ADDR, SEESAW_KEYPAD_BASE, KEYPAD_FIFO, evt, 4)) {
//         DKEY("FIFO read failed");
//         return false;
//     }

//     uint8_t edge     = evt[0] & 0x03;     // 2=rising(press), 3=falling(release)
//     uint8_t key_code = evt[0] >> 2;       // funky code (0,1,2,3,8,9,...,27)
//     uint8_t k        = keycode_to_idx(key_code);  // 0..15 or 0xFF

//     DKEY("CNT>=4, EVT raw: %02X %02X %02X %02X  | code=%u -> idx=%u  edge=%u (%s)",
//          evt[0], evt[1], evt[2], evt[3],
//          key_code, (unsigned)k, edge,
//          (edge==2) ? "PRESS" : (edge==3) ? "RELEASE" : "OTHER");

//     if (k == 0xFF) {
//         DKEY("Unknown key_code=%u (not mapped) — ignoring", key_code);
//         return false;
//     }

//     *idx     = k;
//     *pressed = (edge == 0x02);     // flip to (edge==0x03) if your board is inverted
//     return true;
// }



// 1) Send [module][reg] (no STOP so we can follow with BUF payload)
static inline bool seesaw_write_hdr(uint8_t addr, uint8_t module, uint8_t reg) {
    uint8_t hdr[2] = { module, reg };
    printf("[HDR] addr=0x%02X mod=0x%02X reg=0x%02X\n", addr, module, reg);
    int wrote = i2c_write_blocking(NEOTRELLIS_I2C, addr, hdr, 2, /*nostop=*/true);
    printf("[HDR] wrote=%d (expect 2), nostop=1\n", wrote);
    return wrote == 2;
}

// 2) Send payload bytes; 'nostop=false' ends with STOP
static inline bool seesaw_write_payload(uint8_t addr, const uint8_t *buf, size_t len, bool nostop) {
    printf("[PAYLOAD] addr=0x%02X len=%u nostop=%d\n", addr, (unsigned)len, (int)nostop);
    if (len >= 2) {
        printf("[PAYLOAD] first bytes: %02X %02X%s\n",
               buf[0], buf[1], (len>2 ? " ..." : ""));
    }
    int wrote = i2c_write_blocking(NEOTRELLIS_I2C, addr, buf, (int)len, nostop);
    printf("[PAYLOAD] wrote=%d (expect %u)\n", wrote, (unsigned)len);
    return wrote == (int)len;
}



static bool neopixel_buf_write_chunk(uint16_t start, const uint8_t *data, size_t n) {
    if (n > 28) { printf("[BUF] n=%u > 28\n", (unsigned)n); return false; }

    uint8_t frame[2 + 2 + 28];   // [module,reg] + [start16] + data
    size_t  total = 2 + 2 + n;

    frame[0] = SEESAW_NEOPIXEL_BASE;   // 0x0E
    frame[1] = NEOPIXEL_BUF;           // 0x04
    frame[3] = (uint8_t)(start >> 8);
    frame[2] = (uint8_t)(start & 0xFF);
    memcpy(&frame[4], data, n);

    int wrote = i2c_write_blocking(NEOTRELLIS_I2C, NEOTRELLIS_ADDR, frame, (int)total, false);
    printf("[BUF] start=%u n=%u wrote=%d need=%u\n",
           start, (unsigned)n, wrote, (unsigned)total);
    return wrote == (int)total;
}




// static bool neopixel_buf_write_chunk(uint16_t start, const uint8_t *data, size_t data_len) {
//     if (data_len > 28) {
//         printf("[BUF] ERROR data_len=%u > 28\n", (unsigned)data_len);
//         return false;
//     }
//     uint8_t payload[2 + 28];
//     payload[0] = (uint8_t)((start >> 8) & 0xFF);
//     payload[1] = (uint8_t)( start       & 0xFF);
//     memcpy(&payload[2], data, data_len);
//     printf("[BUF] start=%u data_len=%u payload_len=%u (frame_total=%u)\n",
//            (unsigned)start, (unsigned)data_len, (unsigned)(2 + data_len),
//            (unsigned)(2 /*hdr*/ + 2 /*start*/ + data_len));
//     if (!seesaw_write_payload(NEOTRELLIS_ADDR, payload, 2 + data_len, /*nostop=*/false)) {
//         printf("[BUF] ERROR payload write failed\n");
//         return false;
//     }
//     printf("[BUF] chunk OK\n");
//     return true;
// }



// 4) Write the whole 48B frame in two chunks, with a header before the first chunk
bool neotrellis_write_frame48_and_show(const uint8_t frame48[48]) {
    printf("[FRAME48] begin\n");

    // header first (keeps bus open for BUF payload)
    // sleep_us(300);
    // if (!seesaw_write_hdr(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF)) {
    //     printf("[FRAME48] ERROR: header write failed\n");
    //     return false;
    // }

    // chunk 0: 28 bytes @ offset 0
    sleep_ms(3);
    printf("[FRAME48] chunk0 start=0 len=28\n");
    if (!neopixel_buf_write_chunk(0, frame48, 28)) {
        printf("[FRAME48] ERROR: chunk0 failed\n");
        return false;
    }



    // // header again for the second chunk (new command)
    // if (!seesaw_write_hdr(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF)) {
    //     printf("[FRAME48] ERROR: header write failed (chunk1)\n");
    //     return false;
    // }

    // chunk 1: remaining 20 bytes @ offset 28
    printf("[FRAME48] chunk1 start=28 len=20\n");
    if (!neopixel_buf_write_chunk(28, frame48 + 28, 20)) {
        printf("[FRAME48] ERROR: chunk1 failed\n");
        return false;
    }

    printf("[FRAME48] SHOW...\n");
    bool ok = neopixel_show();
    printf("[FRAME48] SHOW ret=%d\n", (int)ok);
    return ok;
}




bool neopixel_fill_all_and_show(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t frame[48];
    for (int i = 0; i < 16; ++i) {
        frame[3*i + 0] = g;
        frame[3*i + 1] = r;
        frame[3*i + 2] = b;
    }
    return neotrellis_write_frame48_and_show(frame);
}


bool neopixel_test_simple() {
    printf("Testing LED patterns...\n");
    
    // Test 1: Full brightness red (GRB order)
    uint8_t red[3] = {0xFF, 0x00, 0x00};  // G=0, R=0xFF, B=0
    // Test 2: Full brightness green  
    uint8_t green[3] = {0x00, 0xFF, 0x00}; // G=0xFF, R=0, B=0
    // Test 3: Full brightness blue
    uint8_t blue[3] = {0x00, 0x00, 0xFF};  // G=0, R=0, B=0xFF
    
    // Test red on first pixel
    uint8_t cmd[6] = {
        SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF,
        0x00, 0x00,  // pixel 0
        red[0], red[1], red[2]
    };
    
    // i2c_write_blocking(NEOTRELLIS_I2C, NEOTRELLIS_ADDR, cmd, 6, false);
    // neopixel_show();
    // sleep_ms(1000);
    
    // Test green on second pixel  
    cmd[2] = 0x03; cmd[3] = 0x00;  // pixel 1 (address 3 bytes per pixel)
    cmd[4] = green[0]; cmd[5] = green[1]; cmd[6] = green[2];
    i2c_write_blocking(NEOTRELLIS_I2C, NEOTRELLIS_ADDR, cmd, 6, false);
    neopixel_show();
    sleep_ms(1000);
    
    return true;
}