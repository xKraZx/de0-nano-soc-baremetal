#include "benchmark_manager.hpp"
#include "benchmarks.hpp"

BenchmarkManager::BenchmarkManager()
{
    benchmarks_.push_back(std::make_unique<Benchmark1>());
    pmu_start_cycle_counter();
}

void BenchmarkManager::start()
{
    data_.fill(0);

    currentBenchmark_ = 0;
    running_ = true;
    finished_ = false;
}

void BenchmarkManager::process()
{
    if (!running_)
    {
        return;
    }

    if (currentBenchmark_ < benchmarks_.size())
    {
        auto& benchmark = benchmarks_[currentBenchmark_];

        benchmark->prepare();
        benchmark->execute();

        uint32_t cycles = benchmark->cycles();

        const std::size_t offset = currentBenchmark_ * sizeof(uint32_t);

        data_[offset + 0] = static_cast<uint8_t>(cycles >> 0);
        data_[offset + 1] = static_cast<uint8_t>(cycles >> 8);
        data_[offset + 2] = static_cast<uint8_t>(cycles >> 16);
        data_[offset + 3] = static_cast<uint8_t>(cycles >> 24);

        ++currentBenchmark_;
    }

    if (currentBenchmark_ >= benchmarks_.size())
    {
        running_ = false;
        finished_ = true;
    }
}


void BenchmarkManager::reset()
{
    currentBenchmark_ = 0;
    finished_ = false;
}

bool BenchmarkManager::isFinished() const
{
    return finished_;
}