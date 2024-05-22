#include "../utils/repeating_timer.h"

#include "constants.h"

RepeatingTimer::RepeatingTimer(asio::io_context &io_context)
    : timer_(io_context),
    count_(0), max_count_(constants::send_attempts), is_set_(false)
{
}

void RepeatingTimer::SetCallback(FailCallbackFunc callback)
{
    fail_callback_ = callback;
}

void RepeatingTimer::SetMaxCount(size_t count)
{
    max_count_ = count;
}

void RepeatingTimer::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    is_set_ = false;
}
