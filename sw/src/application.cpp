#include "application.hpp"

void Application::init()
{
    uart_.init();
}

void Application::process()
{
    switch (state_)
    {
    case State::Idle:
        processIdle();
        break;

    case State::Collecting:
        processCollecting();
        break;

    case State::Sending:
        processSending();
        break;
    }
}

void Application::processIdle()
{
    if (!uart_.commandAvailable())
    {
        return;
    }

    const Command command = uart_.getCommand();

    if (command == Command::Start)
    {
        //benchmarkManager_.start();

        state_ = State::Collecting;
    }
}

void Application::processCollecting()
{
    //benchmarkManager_.process();

    //if (benchmarkManager_.isFinished())
    {
        state_ = State::Sending;
    }
}

void Application::processSending()
{
    //const auto& data = benchmarkManager_.data();

    //uart_.send(data.data(), data.size());

    //benchmarkManager_.reset();

    state_ = State::Idle;
}
