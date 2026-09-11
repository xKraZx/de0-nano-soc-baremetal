#pragma once

#include <stdint.h>

class Benchmark
{
public:
    virtual ~Benchmark() = default;

    virtual void prepare() = 0;
    virtual void execute() = 0;

    uint32_t cycles() const
    {
        return cycles_;
    }

protected:
    uint32_t cycles_ = 0;
};
