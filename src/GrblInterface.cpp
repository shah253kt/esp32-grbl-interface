#include "GrblInterface.h"

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

    stream.print(STATUS_REPORT_COMMAND);
    stream.print(EOL);
    nextAvailableAt += STATUS_REPORT_MIN_INTERVAL_MS;
    return true;
}

bool GrblInterface::encode(const char c)
{
    if (m_currentIndex >= BUFFER_SIZE)
    {
        return false;
    }

    if (c == EOL)
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