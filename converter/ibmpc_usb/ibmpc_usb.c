/*
Copyright 2019 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>
#include "action.h"
#include "print.h"
#include "util.h"
#include "debug.h"
#include "ibmpc.h"
#include "host.h"
#include "led.h"
#include "matrix.h"
#include "timer.h"


static void matrix_make(uint8_t code);
static void matrix_break(uint8_t code);


/*
 * Matrix Array usage:
 * 'Scan Code Set 2' is assigned into 256(32x8)cell matrix.
 * Hmm, it is very sparse and not efficient :(
 *
 * Notes:
 * Both 'Hanguel/English'(F1) and 'Hanja'(F2) collide with 'Delete'(E0 71) and 'Down'(E0 72).
 * These two Korean keys need exceptional handling and are not supported for now. Sorry.
 *
 *    8bit wide
 *   +---------+
 *  0|         |
 *  :|   XX    | 00-7F for normal codes(without E0-prefix)
 *  f|_________|
 * 10|         |
 *  :|  E0 YY  | 80-FF for E0-prefixed codes
 * 1f|         |     (<YY>|0x80) is used as matrix position.
 *   +---------+
 *
 * Exceptions:
 * 0x83:    F7(0x83) This is a normal code but beyond  0x7F.
 * 0xFC:    PrintScreen
 * 0xFE:    Pause
 */
static uint8_t matrix[MATRIX_ROWS];
#define ROW(code)      (code>>3)
#define COL(code)      (code&0x07)

// matrix positions for exceptional keys
#define F7             (0x83)
#define PRINT_SCREEN   (0xFC)
#define PAUSE          (0xFE)

static int16_t read_wait(uint16_t wait_ms)
{
    uint16_t start = timer_read();
    int16_t code;
    while ((code = ibmpc_host_recv()) == -1 && timer_elapsed(start) < wait_ms);
    return code;
}

static uint16_t read_keyboard_id(void)
{
    uint16_t id = 0;
    int16_t  code = 0;

    // Disable
    code = ibmpc_host_send(0xF5);

    // Read ID
    code = ibmpc_host_send(0xF2);
    if (code == -1)  return 0xFFFF;     // XT or No keyboard
    if (code != 0xFA) return 0xFFFE;    // Broken PS/2?
    
    code = read_wait(1000);
    if (code == -1)  return 0x0000;     // AT
    id = (code & 0xFF)<<8;

    code = read_wait(1000);
    id |= code & 0xFF;

    // Enable
    code = ibmpc_host_send(0xF4);

    return id;
}

void matrix_init(void)
{
    debug_enable = true;
    ibmpc_host_init();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) matrix[i] = 0x00;

    return;
}

/*
 * keyboard recognition
 *
 * 1. Send F2 to get keyboard ID
 *      a. no ACK(FA): XT keyobard
 *      b. ACK but no ID: 84-key AT keyboard CodeSet2
 *      c. ID is AB 83: PS/2 keyboard CodeSet2
 *      d. ID is BF BF: Terminal keyboard CodeSet3
 *      e. error on recv: maybe broken PS/2
 */
uint8_t matrix_scan(void)
{

    // scan code reading states
    static enum {
        INIT,
        WAIT_STARTUP,
        READ_ID,
        LOOP,
        END
    } state = INIT;


 
    if (ibmpc_error) {
        xprintf("err: %02X\n", ibmpc_error);
        // clear or process error
        ibmpc_error = IBMPC_ERR_NONE;
    }

    static enum {
        NONE,
        PC_XT,
        PC_AT,
        PC_TERMINAL,
        OTHER,
    } keyboard_kind = NONE;
    static uint16_t last_time;
    static uint16_t keyboard_id;
    int16_t code;
    switch (state) {
        case INIT:
            last_time = timer_read();
            state = WAIT_STARTUP;
            break;
        case WAIT_STARTUP:
            // read and ignore BAT code and other codes when power-up
            code = ibmpc_host_recv();
            if (timer_elapsed(last_time) > 1000) {
                state = READ_ID;
            }
            break;
        case READ_ID:
            keyboard_id = read_keyboard_id();
            xprintf("ID: %04X\n", keyboard_id);
            if (0xAB00 == (keyboard_id & 0xFF00)) {
                // CodeSet2 PS/2
                keyboard_kind = PC_AT;
            } else if (0xBF00 == (keyboard_id & 0xFF00)) {
                // CodeSet3 Terminal
                keyboard_kind = PC_TERMINAL;
            } else if (0x0000 == keyboard_id) {
                // CodeSet2 AT
                keyboard_kind = PC_AT;
            } else if (0xFFFF == keyboard_id) {
                // CodeSet1 XT
                keyboard_kind = PC_XT;
            } else if (0xFFFE == keyboard_id) {
                // CodeSet2 PS/2 fails to response?
                keyboard_kind = PC_AT;
            } else if (0x00FF == keyboard_id) {
                // Mouse is not supported
                xprintf("Mouse: not supported\n");
                keyboard_kind = NONE;
            } else {
                keyboard_kind = PC_AT;
            }

            // protocol
            if (keyboard_kind == PC_XT) {
                ibmpc_protocol = IBMPC_PROTOCOL_XT;
            } else {
                ibmpc_protocol = IBMPC_PROTOCOL_AT;
            }
            state = LOOP;
            break;
        case LOOP:
            code = ibmpc_host_recv();
            switch (code) {
                case 0xAA:  // BAT OK
                case 0xFC:  // BAT FAIL
                    // new keyboard plug-in
                    state = INIT;
                    break;
            }
            break;
        default:
            break;
    }


    return 1;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & (1<<col));
}

inline
uint8_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

uint8_t matrix_key_count(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        count += bitpop(matrix[i]);
    }
    return count;
}


inline
static void matrix_make(uint8_t code)
{
    if (!matrix_is_on(ROW(code), COL(code))) {
        matrix[ROW(code)] |= 1<<COL(code);
    }
}

inline
static void matrix_break(uint8_t code)
{
    if (matrix_is_on(ROW(code), COL(code))) {
        matrix[ROW(code)] &= ~(1<<COL(code));
    }
}

void matrix_clear(void)
{
    for (uint8_t i=0; i < MATRIX_ROWS; i++) matrix[i] = 0x00;
}

void led_set(uint8_t usb_led)
{
/*
    uint8_t ibmpc_led = 0;
    if (usb_led &  (1<<USB_LED_SCROLL_LOCK))
        ibmpc_led |= (1<<IBMPC_LED_SCROLL_LOCK);
    if (usb_led &  (1<<USB_LED_NUM_LOCK))
        ibmpc_led |= (1<<IBMPC_LED_NUM_LOCK);
    if (usb_led &  (1<<USB_LED_CAPS_LOCK))
        ibmpc_led |= (1<<IBMPC_LED_CAPS_LOCK);
    ibmpc_host_set_led(ibmpc_led);
*/
}
