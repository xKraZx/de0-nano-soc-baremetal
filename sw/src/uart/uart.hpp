#pragma once

#include <cstddef>
#include <cstdint>

extern "C"
{
#include "alt_16550_uart.h"
}

enum class Command
{
    Start,
    Unknown
};

class Uart
{
public:
    void init();

    bool commandAvailable();

    Command getCommand();

    void send(const uint8_t* data, std::size_t size);

private:
    ALT_16550_HANDLE_t handle_{};
};
