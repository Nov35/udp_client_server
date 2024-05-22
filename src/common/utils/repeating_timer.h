#pragma once

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/ip/udp.hpp>
#include <functional>
#include <mutex>

using FailCallbackFunc = std::function<void()>;

class RepeatingTimer
{
public:
    RepeatingTimer(asio::io_context &io_context);

    void SetCallback(FailCallbackFunc callback);
    void SetMaxCount(size_t count);
    void stop();

    template <typename Func, typename... Args>
    void operator()(Func &&function, Args &&...args);

private:
    asio::steady_timer timer_;
    size_t count_;
    size_t max_count_;
    FailCallbackFunc fail_callback_;
    std::mutex mutex_;
    bool is_set_;
};

template <typename Func, typename... Args>
inline void RepeatingTimer::operator()(Func &&function, Args &&...args)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (count_ == 0)
        {
            is_set_ = true;
        }
        else if (is_set_ == false)
        {
            count_ = 0;
            is_set_ = false;

            return;
        }
        else if (count_ == max_count_)
        {
            count_ = 0;
            is_set_ = false;

            lock.unlock();
            return fail_callback_();
        }

        ++count_;
    }

    timer_.expires_from_now(asio::chrono::milliseconds(500));
    timer_.async_wait([this, function,... args = std::forward<Args>(args)](std::error_code ec)
    {
        operator()(function, std::forward<Args>(args)...);
    });

    function(std::forward<Args>(args)...);
}
