#include "neotrellis.h"
#include "seesaw.h"
#include "pico/stdlib.h"
#include <string.h>

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


    if (!neotrellis_wait_ready(300)) {
        printf("HW_ID never became 0x55\n");
        return false;
    }
    printf("HW_ID OK (0x55)\n");


    uint32_t ver = 0;
    if (neotrellis_status(NULL, &ver)) {
        printf("FW ver: 0x%08X\n", ver);
    }


    uint8_t len_be[2] = { 0x00, 0x30 };
    if (!seesaw_write(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_BUF_LENGTH, len_be, 2)) {
        printf("BUF_LENGTH write failed\n"); return false;
    }


    // if (!seesaw_write_u8(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_PIN, internal_pin)) {
    //     printf("NEOPIXEL_PIN write failed\n"); return false;
    // }


    // (void)seesaw_write_u8(NEOTRELLIS_ADDR, SEESAW_NEOPIXEL_BASE, NEOPIXEL_SPEED, 1);


    if (!neotrellis_status(NULL, NULL)) { printf("final status ping failed\n"); return false; }

    return true;
}


static void dump_bytes(const char *tag, const uint8_t *p, size_t n) {
    printf("%s (%u):", tag, (unsigned)n);
    for (size_t i = 0; i < n; ++i) printf(" %02X", p[i]);
    printf("\n");
}

// Writes the full 48-byte frame to NEOPIXEL_BUF using the required payload shape:
// [offset_hi, offset_lo, DATA...] with total payload ≤ 32 bytes per write.
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


bool neopixel_set_one_and_show(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= NEOTRELLIS_LED_COUNT) {
        printf("IDX out of range: %u\n", index);
        return false;
    }

    uint8_t buf[NEOTRELLIS_BYTES] = {0};

    // Fill all LEDs to 0 first (explicit for clarity/log symmetry)
    for (int i = 0; i < NEOTRELLIS_LED_COUNT; ++i) {
        int base = 3 * i;
        buf[base + 0] = 0;
        buf[base + 1] = 0;
        buf[base + 2] = 0;
    }

    // Set selected pixel — GRB (swap to RGB if colors look wrong)
    int base = 3 * index;
    buf[base + 0] = g;
    buf[base + 1] = r;
    buf[base + 2] = b;

    printf("Lighting LED %u to (r=%u,g=%u,b=%u) [GRB order]\n", index, r, g, b);
    dump_bytes("Frame[0..15] preview", buf, 16);  // small peek

    bool ok = neopixel_set_bulk(buf);
    if (!ok) {
        printf("neopixel_set_bulk() FAILED\n");
        return false;
    }

    ok = neopixel_show();
    if (!ok) {
        printf("neopixel_show() FAILED\n");
        return false;
    }

    printf("neopixel_set_one_and_show() -> SUCCESS\n");
    return true;
}