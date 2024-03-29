#include "keyboard.h"
#include "kernel/print.h"
#include "interrupt.h"
#include "kernel/io.h"
#include "global.h"
#include <stdbool.h>
#include "stdint.h"
#include "ioqueue.h"

#define KBD_BUF_PORT 0x60 // the keboard buffer reg port

// control characters
#define esc         '\x1b'
#define backspace   '\b'
#define tab         '\t'
#define enter       '\r'
#define delete      '\x7f'

// invisiable characters
#define char_invisible  0
#define ctrl_l_char     char_invisible
#define ctrl_r_char     char_invisible
#define shift_l_char    char_invisible
#define shift_r_char    char_invisible
#define alt_l_char      char_invisible
#define alt_r_char      char_invisible
#define caps_lock_char  char_invisible

#define shift_l_make    0x2a
#define shift_r_make    0x36
#define alt_l_make      0x38
#define alt_r_make      0xe038
#define alt_r_break     0xe0b8
#define ctrl_l_make     0x1d
#define ctrl_r_make     0xe01d
#define ctrl_r_break    0xe09d
#define caps_lock_make  0x3a

struct ioqueue kbd_buf;

/* To record the status of pressing key
 * ext_scancode record that if the makecode has the prefix 0xe0 */
static bool ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

static char keymap[][2] = {
    /* 0x00 */      {0,     0},
    /* 0x01 */      {esc,   esc},
    /* 0x02 */      {'1',   '!'},
    /* 0x03 */      {'2',   '@'},
    /* 0x04 */      {'3',   '#'},
    /* 0x05 */      {'4',   '$'},
    /* 0x06 */      {'5',   '%'},
    /* 0x07 */      {'6',   '^'},
    /* 0x08 */      {'7',   '&'},
    /* 0x09 */      {'8',   '*'},
    /* 0x0A */      {'9',   '('},
    /* 0x0B */      {'0',   ')'},
    /* 0x0C */      {'-',   '_'},
    /* 0x0D */      {'=',   '+'},
    /* 0x0E */      {backspace, backspace},
    /* 0x0F */      {tab,   tab},
    /* 0x10 */      {'q',   'Q'},
                    {'w', 'W'},
                    {'e', 'E'},
                    {'r', 'R'},
                    {'t', 'T'},
                    {'y', 'Y'},
                    {'u', 'U'},
                    {'i', 'I'},
                    {'o', 'O'},
                    {'p', 'P'},
                    {'[', '{'},
                    {']', '}'},
                    {enter, enter},
                    {ctrl_l_char, ctrl_l_char},
                    {'a', 'A'},
                    {'s', 'S'},
                    {'d', 'D'},
                    {'f', 'F'},
                    {'g', 'G'},
                    {'h', 'H'},
                    {'j', 'J'},
                    {'k', 'K'},
                    {'l', 'L'},
                    {';', ':'},
                    {'\'', '"'},
                    {'`', '~'},
                    {shift_l_char, shift_l_char},
                    {'\\', '|'},
                    {'z', 'Z'},
                    {'x', 'X'},
                    {'c', 'C'},
                    {'v', 'V'},
                    {'b', 'B'},
                    {'n', 'N'},
                    {'m', 'M'},
                    {',', '<'},
                    {'.', '>'},
                    {'/', '?'},
                    {shift_r_char, shift_r_char},
                    {'*', '*'},
                    {alt_l_char, alt_l_char},
                    {' ', ' '},
                    {caps_lock_char, caps_lock_char}
};

static void intr_keyboard_handler(void)
{
    bool ctrl_down_last = ctrl_status;
    bool shift_down_last = shift_status;
    bool caps_lock_last = caps_lock_status;

    bool break_code;

    uint16_t scanCode = inb(KBD_BUF_PORT);

    if (scanCode == 0xe0) {
        ext_scancode = true;
        return;
    }

    if (ext_scancode) {
        scanCode = ((0xe000) | scanCode);
        ext_scancode = false;
    }

    break_code = ((scanCode & 0x0080) != 0);

    if (break_code) {
        uint16_t make_code = (scanCode &= 0xff7f);

        if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
            ctrl_status = false;
        } else if (make_code == shift_l_make || make_code == shift_r_make) {
            shift_status = false;
        } else if (make_code == alt_l_make || make_code == alt_r_make) {
            alt_status = false;
        }

        return;
    } else if ((scanCode > 0x00 && scanCode < 0x3b) || 
            (scanCode == alt_r_make) ||
            (scanCode == ctrl_r_make)) {
        bool shift = false;

        if ((scanCode < 0x0e) || (scanCode == 0x29) || 
                (scanCode == 0x1a) || (scanCode == 0x1b) ||
                (scanCode == 0x2b) || (scanCode == 0x27) || 
                (scanCode == 0x28) || (scanCode == 0x33) || 
                (scanCode == 0x34) || (scanCode == 0x35)) {
            if (shift_down_last) {
                shift = true;
            }
        } else {
            if (shift_down_last && caps_lock_last) {
                shift = false;
            } else if (shift_down_last || caps_lock_last) {
                shift = true;
            } else {
                shift = false;
            }
        }

        uint8_t index = (scanCode &= 0x00ff);
        char cur_char = keymap[index][shift];

        if (cur_char) {
            if ((ctrl_down_last && cur_char == 'l') ||
                    (ctrl_down_last && cur_char == 'u')) {
                cur_char -= 'a';
            }
            if (!ioq_full(&kbd_buf)) {
                // put_char(cur_char);
                ioq_putchar(&kbd_buf, cur_char);
            }
            return;
        }

        if (scanCode == ctrl_l_make || scanCode == ctrl_r_make) {
            ctrl_status = true;
        } else if (scanCode == shift_l_make || scanCode == shift_r_make) {
            shift_status = true;
        } else if (scanCode == alt_l_make || scanCode == alt_r_make) {
            alt_status = true;
        } else if (scanCode == caps_lock_make) {
            caps_lock_status = !caps_lock_status;
        }
    } else {
        put_str("unknown key\n");
    }
}

void keyboard_init()
{
    put_str("keyboard init start\n");
    ioqueue_init(&kbd_buf);
    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard init done\n");
}
