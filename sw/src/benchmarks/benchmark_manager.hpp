#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "benchmark.hpp"

class BenchmarkManager
{
public:
    static constexpr std::size_t DataSize = 256;

    BenchmarkManager();

    void start();
    void process();

    bool isFinished() const;

    const std::array<uint8_t, DataSize>& data() const
    {
        return data_;
    }

    void reset();

private:
    std::vector<std::unique_ptr<Benchmark>> benchmarks_;

    std::array<uint8_t, DataSize> data_{};

    std::size_t currentBenchmark_ = 0;
    bool running_ = false;
    bool finished_ = false;
};
