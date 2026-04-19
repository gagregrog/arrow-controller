#pragma once

#define NFC_UNKNOWN_TAG -2

void nfcBegin();
int nfcLoop(); // returns tag id, NFC_UNKNOWN_TAG, or -1 if no new tag
