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


    if (!neotrellis_reset()) { printf("SWRST write NACK\n"); return false; }
    sleep_ms(10);

    uint8_t pin = 3;  
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_PIN, &pin, 1))  
    { printf("PIN set fail\n");
    } 
    else{
        printf("PIN set succesfully\n");
    }
    sleep_ms(2);


    if (!neotrellis_wait_ready(300)) {
        printf("HW_ID never became 0x55\n");
        return false;
    }
    printf("HW_ID OK (0x55)\n");


    uint ver = 0;
    if (neotrellis_status(NULL, &ver)) {
        printf("FW ver: 0x%08X\n", ver);
    }



    uint8_t len_be[2] = { 0x00, 0x30 };

    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF_LENGTH, len_be, 2)) {
        printf("BUF_LENGTH write failed\n"); return false;
    }
    sleep_ms(20);

    if (!neotrellis_status(NULL, NULL)) { printf("final status ping failed\n"); return false; }

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
    return ok;
}





static void dump_bytes(const char *tag, const uint8_t *p, size_t n) {
    printf("%s (%u):", tag, (unsigned)n);
    for (size_t i = 0; i < n; ++i) printf(" %02X", p[i]);
    printf("\n");
}






bool neopixel_set_bulk(const uint8_t *rgb48) {
    // chunk #1: offset 0x0000, 30 data bytes (payload 32)
    uint8_t p0[2 + 30];
    p0[0] = 0x00; p0[1] = 0x00;                 // start address = 0
    memcpy(p0 + 2, rgb48, 30);
    dump_bytes("BUF[0..29] payload", p0, sizeof p0);

    bool ok1 = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, p0, sizeof p0);
    printf("BUF[0..29] -> %s\n", ok1 ? "OK" : "NACK");
    if (!ok1) return false;

    // chunk #2: offset 0x001E, remaining 18 data bytes (payload 20)
    uint8_t p1[2 + 18];
    p1[0] = 0x00; p1[1] = 0x1E;                 // start address = 30
    memcpy(p1 + 2, rgb48 + 30, 18);
    dump_bytes("BUF[30..47] payload", p1, sizeof p1);

    bool ok2 = seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, p1, sizeof p1);
    printf("BUF[30..47] -> %s\n", ok2 ? "OK" : "NACK");
    return ok2;
}





bool neopixel_set_one_and_show(int idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx < 0 || idx >= 16) {
        printf("[ERR] idx %d out of range\n", idx);
        return false;
    }

    uint8_t frame[48] = {0};
    int base = idx * 3;    
    frame[base + 0] = g;   
    frame[base + 1] = r;
    frame[base + 2] = b;

    printf("[DBG] set idx=%d RGB=(%u,%u,%u) off=%d\n", idx, r, g, b, base);

    
    uint8_t p0[2 + 30];
    p0[0] = 0x00;           
    p0[1] = 0x00;          
    memcpy(p0 + 2, frame, 30);

    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, p0, sizeof p0)) {
        printf("[ERR] BUF write chunk0 failed (off=0,len=30)\n");
        return false;
    } else {
        printf("[DBG] BUF chunk0 ok (off=0,len=30)\n");
    }


    uint8_t p1[2 + 18];
    p1[0] = 0x00;           
    p1[1] = 0x1E;       
    memcpy(p1 + 2, frame + 30, 18);

    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF, p1, sizeof p1)) {
        printf("[ERR] BUF write chunk1 failed (off=30,len=18)\n");
        return false;
    } else {
        printf("[DBG] BUF chunk1 ok (off=30,len=18)\n");
    }


    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SHOW, NULL, 0)) {
        printf("[ERR] SHOW failed\n");
        return false;
    }
    printf("[DBG] SHOW ok\n");
    return true;
}