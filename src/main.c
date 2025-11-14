// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "seesaw.h"
// #include "neotrellis.h"
// #include "tusb_config.h"




// static void scan_i2c(void) {
//     printf("I2C scan:\n");
//     for (uint8_t a = 0x08; a <= 0x77; a++) {
//         uint8_t dummy = 0;
//         int r = i2c_write_blocking(NEOTRELLIS_I2C, a, &dummy, 1, false);
//         if (r >= 0) printf("  Found 0x%02X\n", a);
//     }
// }


// int main() {
//     stdio_init_all();
//     setvbuf(stdout, NULL, _IONBF, 0);   // unbuffer stdout
//     sleep_ms(500);    
//     printf("\n=== NeoTrellis bring-up ===\n");

//     seesaw_bus_init(100000);  
//     scan_i2c();
    
//     if (!neotrellis_reset()) {
//     printf("Failed to reset NeoTrellis!\n");
//     while (1); // Halt
// }
//     else { printf("neotrellis reset successfullly\n");}

//     if (!neotrellis_wait_ready(300)) {
//     printf("Device not ready.\n");
//     return 0;
// }

    
//     sleep_ms(500);
//     uint8_t probe_id = 0;
    
//     bool probe_ok = neotrellis_status(&probe_id, NULL);
//     printf("Probe @0x%02X: %s, HW_ID=0x%02X\n", NEOTRELLIS_ADDR, probe_ok ? "OK" : "FAIL", probe_id);
//     sleep_ms(100);


//     if (!neopixel_begin(3)) {
//         printf("neopixel_begin() failed. Check wiring and address.\n");
//         while (1) { tight_loop_contents(); }
//     }
//     printf("NeoPixel init OK.\n");



//     if (!neopixel_set_one_and_show(0, 100, 0, 0)) {
//         printf("Single pixel test failed.\n");
//         return false;
//     } else {
//         printf("Single pixel test lit.\n");
//     }



//     sleep_ms(500);

//     // uint8_t frame[NEOTRELLIS_BYTES];
//     // for (;;) {
//     //     for (int phase = 0; phase < 3; ++phase) {
//     //         for (int i = 0; i < NEOTRELLIS_LED_COUNT; ++i) {
//     //             int base = 3*i;
//     //             frame[base+0] = (phase==0)?80:0;   // R
//     //             frame[base+1] = (phase==1)?80:0;   // G
//     //             frame[base+2] = (phase==2)?80:0;   // B
//     //         }
            
//     //         neopixel_set_bulk(frame);
//     //         neopixel_show();
//     //         sleep_ms(300);
//     //     }
//     // }
//     return 0;
// }




#include <stdio.h>
#include "pico/stdlib.h"
#include "seesaw.h"
#include "neotrellis.h"
#include "tusb_config.h"




static void scan_i2c(void) {
    printf("I2C scan:\n");
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        uint8_t dummy = 0;
        int r = i2c_write_blocking(NEOTRELLIS_I2C, a, &dummy, 1, false);
        if (r >= 0) printf("  Found 0x%02X\n", a);
    }
}



int main() {
    stdio_init_all();
    setvbuf(stdout, NULL, _IONBF, 0);   // unbuffer stdout
    sleep_ms(500);    
    printf("\n=== NeoTrellis bring-up ===\n");

    seesaw_bus_init(100000);  
    scan_i2c();
    
    if (!neotrellis_reset()) {
    printf("Failed to reset NeoTrellis!\n");
    while (1); // Halt
}
    else { printf("neotrellis reset successfullly\n");}

    if (!neotrellis_wait_ready(300)) {
    printf("Device not ready.\n");
    return 0;
}


  
    sleep_ms(500);
    uint8_t probe_id = 0;
    
    bool probe_ok = neotrellis_status(&probe_id, NULL);
    printf("Probe @0x%02X: %s, HW_ID=0x%02X\n", NEOTRELLIS_ADDR, probe_ok ? "OK" : "FAIL", probe_id);
    sleep_ms(100);


    if (!neopixel_begin(3)) {
        printf("neopixel_begin() failed. Check wiring and address.\n");
        while (1) { tight_loop_contents(); }
    }
    printf("NeoPixel init OK.\n");

    sleep_ms(2);

    

    if (!neopixel_fill_all_and_show(100, 100, 100)) {
        printf("all pixel test failed.\n");
        return false;
    } 

    // neopixel_test_simple();


    //     trellis_neopixel_dump_config();
// if (!trellis_keypad_begin()) {
//     printf("Keypad init failed.\n");
//     while (1) tight_loop_contents();
// 
// printf("Ready. Press keys to light pixels.\n");








// while (1) {
//     uint8_t k; bool down;
//     if (trellis_read_event(&k, &down)) {
//         if (down) {
//             DKEY("PRESS on key idx=%u -> light pixel", (unsigned)k);
//             neopixel_set_one_and_show(k, 8, 0, 8);
//         } else {
//             DKEY("RELEASE on key idx=%u -> clear pixel", (unsigned)k);
//             neopixel_set_one_and_show(k, 0, 0, 0);
//         }
//     }

//     // else printf("trellis read event failed\n");
//     sleep_ms(2);
// }


    // if (!neopixel_set_one_and_show(0, 100, 5, 5)) {
    //     printf("Single pixel test failed.\n");
    //     return false;
    // }

    // else {
    //     printf("all pixel test lit.\n");
    // }
    //sleep_ms(500);
    // uint8_t frame[NEOTRELLIS_BYTES];
    // for (;;) {
    //     for (int phase = 0; phase < 3; ++phase) {
    //         for (int i = 0; i < NEOTRELLIS_LED_COUNT; ++i) {
    //             int base = 3*i;
    //             frame[base+0] = (phase==0)?80:0;   // R
    //             frame[base+1] = (phase==1)?80:0;   // G
    //             frame[base+2] = (phase==2)?80:0;   // B
    //         }           
    //         neopixel_set_one_and_show(frame);
    //         sleep_ms(300);
    //     }
    // }
    return 0;
}
