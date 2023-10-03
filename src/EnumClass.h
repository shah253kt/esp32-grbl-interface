#pragma once

#include "Arduino.h"

#ifndef ENUM_NAME_MAX_LENGTH
#define ENUM_NAME_MAX_LENGTH 10
#endif

class EnumClass
{
public:
    EnumClass();
    EnumClass(const char *name, uint16_t value);
    ~EnumClass() = default;

    bool operator==(const EnumClass &other) const;
    EnumClass operator|(const EnumClass &other) const;
    EnumClass operator&(const EnumClass &other) const;
    uint16_t operator|(uint16_t value) const;
    uint16_t operator&(uint16_t value) const;
    EnumClass operator<<(uint16_t shift);
    EnumClass operator>>(uint16_t shift);

    [[nodiscard]] uint16_t value() const;
    [[nodiscard]] const char *name() const;

private:
    mutable char m_name[ENUM_NAME_MAX_LENGTH];
    mutable uint16_t m_value;
};