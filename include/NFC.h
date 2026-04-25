#pragma once
#include <stdint.h>
#include "badge/BadgeStore.h"

#define NFC_UNKNOWN_TAG -2

void           nfcBegin(BadgeStore* store);
int            nfcLoop(); // returns badge index, NFC_UNKNOWN_TAG, or -1 if no new tag
const uint8_t* nfcLastUID();
uint8_t        nfcLastUIDLen();
