#include "GrblInterface.h"

#include <Regexp.h>

GrblInterface::GrblInterface() : m_currentIndex(0)
{
}

bool GrblInterface::requestStatusReport(const Stream &stream)
{
    static uint32_t nextAvailableAt = 0;

    if (millis() < nextAvailableAt)
    {
        return false;
    }

    stream.print(Grbl::STATUS_REPORT_COMMAND);
    stream.print(Grbl::EOL);
    nextAvailableAt += Grbl::STATUS_REPORT_MIN_INTERVAL_MS;
    return true;
}

bool GrblInterface::update(const Stream &stream)
{
    if (!stream.available())
    {
        return false;
    }

    return encode(stream.read());
}

bool GrblInterface::encode(const char c)
{
    if (m_currentIndex >= Grbl::BUFFER_SIZE)
    {
        return false;
    }

    if (c == Grbl::EOL)
    {
        return processBuffer();
    }

    m_buffer[m_currentIndex++] = c;

    return true;
}

Position GrblInterface::getPosition()
{
    return m_pos;
}

bool GrblInterface::processBuffer()
{
    return true;
}