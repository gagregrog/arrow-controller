#pragma once
#include <stdint.h>

struct BadgeUID {
    uint8_t bytes[7];
    uint8_t len;
};

class BadgeStore {
public:
    virtual int lookup(const uint8_t* uid, uint8_t len) = 0;
    virtual int count() = 0;
    virtual BadgeUID get(int index) = 0;
    virtual int add(const uint8_t* uid, uint8_t len) = 0;
    virtual bool remove(int index) = 0;
    virtual ~BadgeStore() = default;
};
