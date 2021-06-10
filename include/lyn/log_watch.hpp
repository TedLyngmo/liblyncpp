#pragma once

/*
 * Copied from:
 * https://github.com/TedLyngmo/liblyncpp/blob/main/include/lyn/thread.hpp
 * "This is free and unencumbered software released into the public domain."
*/

/*
 * Adapted from https://stackoverflow.com/a/58866761/7582247
 */

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace lyn {

// A C++11 constexpr function template for counting decimals needed for selected precision.
template<std::size_t V, std::size_t C = 0, typename std::enable_if<(V < 10), int>::type = 0>
constexpr std::size_t log10ish() {
    return C;
}

template<std::size_t V, std::size_t C = 0, typename std::enable_if<(V >= 10), int>::type = 0>
constexpr std::size_t log10ish() {
    return log10ish<V / 10, C + 1>();
}

using unanoseconds = std::chrono::duration<std::uint64_t, std::nano>;

// A class to support using different precisions, chrono clocks and formats
template<class Precision = std::chrono::seconds, class Clock = std::chrono::system_clock>
class log_watch {
public:
    // some convenience typedefs and "decimal_width" for sub second precisions
    using precision_type = Precision;
    using ratio_type = typename precision_type::period;
    using clock_type = Clock;
    using time_point = std::chrono::time_point<Clock>;
    static constexpr auto decimal_width = log10ish<ratio_type{}.den>();

    static_assert(ratio_type{}.num <= ratio_type{}.den, "Only second or sub second precision supported");
    static_assert(ratio_type{}.num == 1, "Unsupported precision parameter");

    // default format: "%Y-%m-%dT%H:%M:%S"
    log_watch(const std::string& format = "%FT%T") : m_format(format) {}

    inline log_watch& operator()(time_point tp) {
        m_tp = std::move(tp);
        return *this;
    }
    inline log_watch& operator()(const Precision& p) { return (*this)(time_point(p)); }

    template<class P, class C>
    friend std::ostream& operator<<(std::ostream&, const log_watch<P, C>&);

private:
    std::string m_format;
    time_point m_tp;
};

template<class Precision, class Clock>
std::ostream& operator<<(std::ostream& os, const log_watch<Precision, Clock>& lw) {
    std::ostringstream oss;

    // extract std::time_t from time_point
    std::time_t t = Clock::to_time_t(lw.m_tp);

    // output the part supported by std::tm
    oss << std::put_time(std::localtime(&t), lw.m_format.c_str());

    // only involve chrono duration calc for displaying sub second precisions
    if(lw.decimal_width) { // if constexpr( ... in C++17
        // get duration since epoch
        auto dur = lw.m_tp.time_since_epoch();

        // extract the sub second part from the duration since epoch
        auto subsec = std::chrono::duration_cast<Precision>(dur) % std::chrono::seconds{1};

        // output the sub second part
        oss << std::setfill('0') << std::setw(lw.decimal_width) << subsec.count();
    }

    return os << oss.str();
}

} // namespace lyn
