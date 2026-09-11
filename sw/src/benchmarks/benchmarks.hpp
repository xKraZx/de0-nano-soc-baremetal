#pragma once
extern "C" {
#include <alt_cache.h>
#include <socal.h>
}

#include "benchmark.hpp"
#include "pmu.hpp"

constexpr uint32_t OCM_BASE = 0xC0010000;

class Benchmark1 : public Benchmark
{
public:
    void prepare() override
    {
        alt_cache_system_disable();
    }

    void execute() override
    {
        volatile uint32_t* ocm =
            reinterpret_cast<volatile uint32_t*>(OCM_BASE);

        for (std::size_t i = 0; i < DataSize; ++i)
        {
            source_[i] = static_cast<uint32_t>(i);
        }

        uint32_t start = pmu_read_cycle_counter();

        for (std::size_t i = 0; i < DataSize; ++i)
        {
            ocm[i] = source_[i];
        }

        uint32_t end = pmu_read_cycle_counter();

        cycles_ = end - start;
    }

private:
    static constexpr std::size_t DataSize = 30;

    uint32_t source_[DataSize]{};
};