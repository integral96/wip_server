#pragma once

#include <atomic>
#include <type_traits>
#include <mutex>
#include <functional>
#include <array>
#include <iostream>

template <typename Indicator, size_t count>
class MultiProgress {
private:
    std::array<std::reference_wrapper<Indicator>, count> bars_;
    std::mutex mutex_;
    std::atomic<bool> started_{false};
public:
    template <typename... Indicators, typename T = std::enable_if_t<(sizeof...(Indicators) == count)> >
    explicit MultiProgress(Indicators &... bars) : bars_({bars...}) {}
    template <size_t index>
        typename std::enable_if_t<(index >= 0 && index < count), void>::type
        update(float value, std::ostream &os = std::cout) {
            bars_[index].get().set_progress(value);
        }
public:
    template <size_t index>
    typename std::enable_if<(index >= 0 && index < count), void>::type 
    update(float value, std::ostream &os = std::cout) {
        write_progress(os);
    }

    void write_progress(std::ostream &os = std::cout) {
        std::unique_lock lock{mutex_};

        // Move cursor up if needed
        if (started_)
        for (size_t i = 0; i < count; ++i)
            os << "\x1b[A";

        // Write each bar
        for (auto &bar : bars_) {
        bar.get().write_progress();
        os << "\n";
        }

        if (!started_)
        started_ = true;
    }
};