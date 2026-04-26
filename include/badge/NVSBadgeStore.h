#pragma once
#include "badge/BadgeStore.h"
#include <vector>

class NVSBadgeStore : public BadgeStore {
public:
    void load();
    int lookup(const uint8_t* uid, uint8_t len) override;
    int count() override;
    BadgeUID get(int index) override;
    int add(const uint8_t* uid, uint8_t len) override;
    bool remove(int index) override;

private:
    std::vector<BadgeUID> _badges;
    void save();
};
