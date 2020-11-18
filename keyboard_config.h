#pragma once

static const uint8_t KBSTATP = 0x64;
static const uint8_t KBS_DIB = 0x01;
static const uint8_t KBDATAP = 0x60;

static const uint8_t NO = 0;

static const uint8_t KEY_UP  = 0xE2;
static const uint8_t KEY_DN  = 0xE3;
static const uint8_t KEY_LF  = 0xE4;
static const uint8_t KEY_RT  = 0xE5;

static const uint8_t SHIFT    = 0x2A;
static const uint8_t ALT      = 0x38;
static const uint8_t CTRL     = 0x1D;
static const uint8_t CAPSLOCK = 0x3A;
static const uint8_t NUMLOCK  = 0x45;

static const uint8_t default_table[256] =
        {
                NO,   NO, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
                '7',  '8',  '9',  '0',  '-',  '=',   NO,   NO,
                'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
                'o',  'p',  '[',  ']',  '\n', CTRL,   'a',  's',
                'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
                '\'', '`',  SHIFT,   '\\', 'z',  'x',  'c',  'v',
                'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
                ALT,   ' ',  CAPSLOCK,   NO,   NO,   NO,   NO,   NO,
                NO,   NO,   NO,   NO,   NO,   NUMLOCK,   NO,   '7',  // 0x40
                '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
        };

static const uint8_t shift_table[256] =
        {
                NO,   NO, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
                '7',  '8',  '9',  '0',  '-',  '=',   NO,   NO,
                'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
                'O',  'P',  '[',  ']',  '\n', CTRL,   'A',  'S',
                'D',  'F',  'G',  'H',  'H',  'K',  'L',  ';',  // 0x20
                '\'', '`',  SHIFT,   '\\', 'Z',  'X',  'C',  'V',
                'B',  'N',  'M',  ',',  '.',  '/',  NO,   '*',  // 0x30
                ALT,   ' ',  CAPSLOCK,   NO,   NO,   NO,   NO,   NO,
                NO,   NO,   NO,   NO,   NO,   NUMLOCK,   NO,   '7',  // 0x40
                '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
        };

static const uint8_t E0_table[256] =
        {
                [0x48] KEY_UP,
                [0x4B] KEY_LF,
                [0x4D] KEY_RT,
                [0x50] KEY_DN,
        };
