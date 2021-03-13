#include "keymap_common.h"


const uint8_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    KEYMAP_M0116_ANSI(
                     V,
    ESC, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   MINS,EQL, BSPC,                          NLCK, EQL, PSLS, PAST,
    TAB, Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   LBRC,RBRC,                               P7,  P8,  P9,  PMNS,
    LCTL, A,   S,   D,   F,   G,   H,   J,   K,   L,   SCLN,QUOT,     ENT,                          P4,  P5,  P6,  PPLS,
    LSFT,Z,   X,   C,   V,   B,   N,   M,   COMM,DOT, SLSH,          RSFT,                          P1,  P2,  P3,
    LCAP,LALT,LGUI,GRV          SPC,                               BSLS,LEFT, RIGHT, DOWN, UP,      P0,    PDOT,PENT
    ),
};

const action_t PROGMEM fn_actions[] = {
};
