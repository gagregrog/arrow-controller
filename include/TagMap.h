#pragma once
#include <stdint.h>

struct TagEntry {
    uint8_t uid[7];
    uint8_t uidLen;
    int id;
};

// Add badge UIDs here after scanning them (logged to serial on first read).
// Example: { {0xDE, 0xAD, 0xBE, 0xEF}, 4, 1 }
static const TagEntry TAG_MAP[] = {
};

static const int TAG_MAP_SIZE = sizeof(TAG_MAP) / sizeof(TAG_MAP[0]);
