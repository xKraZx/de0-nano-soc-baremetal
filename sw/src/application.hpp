#pragma once

#include "uart.hpp"
#include "benchmark_manager.hpp"

class Application
{
public:
    void init();

    void process();

private:
    enum class State
    {
        Idle,
        Collecting,
        Sending
    };

    void processIdle();
    void processCollecting();
    void processSending();

    Uart uart_;
    //BenchmarkManager benchmarkManager_;

    State state_ = State::Idle;
};
