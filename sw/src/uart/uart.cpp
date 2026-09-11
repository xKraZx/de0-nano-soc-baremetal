#include "uart.hpp"

void Uart::init()
{
    ALT_STATUS_CODE status;

    // UART0 w HPS
    status = alt_16550_init(
        ALT_16550_DEVICE_SOCFPGA_UART0,
        nullptr,
        0,
        &handle_);

    if (status != ALT_E_SUCCESS)
    {
        // tutaj możesz dodać własną obsługę błędu
        while (true)
        {
        }
    }

    // 8-N-1
    status = alt_16550_line_config_set(
        &handle_,
        ALT_16550_DATABITS_8,
        ALT_16550_PARITY_DISABLE,
        ALT_16550_STOPBITS_1);

    if (status != ALT_E_SUCCESS)
    {
        while (true)
        {
        }
    }

    // 115200 baud
    status = alt_16550_baudrate_set(
        &handle_,
        115200);

    if (status != ALT_E_SUCCESS)
    {
        while (true)
        {
        }
    }

    // RX/TX FIFO
    status = alt_16550_fifo_enable(&handle_);

    if (status != ALT_E_SUCCESS)
    {
        while (true)
        {
        }
    }

    // Włącz UART
    status = alt_16550_enable(&handle_);

    if (status != ALT_E_SUCCESS)
    {
        while (true)
        {
        }
    }
}

bool Uart::commandAvailable()
{
    uint32_t status = 0;

    if (alt_16550_line_status_get(&handle_, &status) != ALT_E_SUCCESS)
    {
        return false;
    }

    // LSR.DR = Data Ready
    return (status & ALT_16550_LINE_STATUS_DR) != 0;
}

Command Uart::getCommand()
{
    char c = 0;

    if (alt_16550_fifo_read(&handle_, &c, 1) != ALT_E_SUCCESS)
    {
        return Command::Unknown;
    }

    if (c == 'S')
    {
        return Command::Start;
    }

    return Command::Unknown;
}

void Uart::send(const uint8_t* data, std::size_t size)
{
    if (data == nullptr || size == 0)
    {
        return;
    }

    alt_16550_fifo_write_safe(
        &handle_,
        reinterpret_cast<const char*>(data),
        size,
        true);
}
