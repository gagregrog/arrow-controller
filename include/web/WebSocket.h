#pragma once
#include <stdint.h>

void wsBegin();
void wsLoop();
void wsNotifyBadgeScan(int index);
void wsNotifyUnknownBadge(const uint8_t* uid, uint8_t len);
