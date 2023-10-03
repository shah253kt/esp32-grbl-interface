#include "EnumClass.h"

EnumClass::EnumClass() : m_value(0)
{
    strcpy(m_name, "");
}

EnumClass::EnumClass(const char *name, uint16_t value) : m_value(value)
{
    strcpy(m_name, name);
}

bool EnumClass::operator==(const EnumClass &other) const
{
    return strcmp(m_name, other.name()) == 0 && m_value == other.value();
}

EnumClass EnumClass::operator|(const EnumClass &other) const
{
    return EnumClass(m_name, m_value | other.value());
}

EnumClass EnumClass::operator&(const EnumClass &other) const
{
    return EnumClass(m_name, m_value & other.value());
}

uint16_t EnumClass::operator|(uint16_t value) const
{
    return m_value | value;
}

uint16_t EnumClass::operator&(uint16_t value) const
{
    return m_value & value;
}

EnumClass EnumClass::operator<<(uint16_t shift)
{
    m_value <<= shift;
    return *this;
}

EnumClass EnumClass::operator>>(uint16_t shift)
{
    m_value >>= shift;
    return *this;
}

uint16_t EnumClass::value() const
{
    return m_value;
}

const char *EnumClass::name() const
{
    return m_name;
}