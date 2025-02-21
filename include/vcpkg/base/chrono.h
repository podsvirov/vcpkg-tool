#pragma once

#include <vcpkg/base/format.h>
#include <vcpkg/base/optional.h>
#include <vcpkg/base/stringview.h>

#include <atomic>
#include <chrono>
#include <string>

namespace vcpkg
{
    struct ElapsedTime
    {
        using duration = std::chrono::high_resolution_clock::time_point::duration;

        constexpr ElapsedTime() noexcept : m_duration() { }
        constexpr ElapsedTime(duration d) noexcept : m_duration(d) { }

        template<class TimeUnit>
        TimeUnit as() const
        {
            return std::chrono::duration_cast<TimeUnit>(m_duration);
        }

        ElapsedTime& operator+=(const ElapsedTime& other)
        {
            m_duration += other.m_duration;
            return *this;
        }

        std::string to_string() const;
        void to_string(std::string& into) const;

    private:
        duration m_duration;
    };

    struct ElapsedTimer
    {
        static ElapsedTimer create_started();

        constexpr ElapsedTimer() noexcept : m_start_tick() { }

        ElapsedTime elapsed() const
        {
            return ElapsedTime(std::chrono::high_resolution_clock::now() - this->m_start_tick);
        }

        double microseconds() const { return elapsed().as<std::chrono::duration<double, std::micro>>().count(); }
        uint64_t us_64() const { return elapsed().as<std::chrono::duration<uint64_t, std::micro>>().count(); }

        std::string to_string() const;
        void to_string(std::string& into) const;

    private:
        std::chrono::high_resolution_clock::time_point m_start_tick;
    };

    struct StatsTimer
    {
        StatsTimer(std::atomic<uint64_t>& stat) : m_stat(&stat), m_timer(ElapsedTimer::create_started()) { }
        ~StatsTimer() { m_stat->fetch_add(m_timer.us_64()); }

    private:
        std::atomic<uint64_t>* const m_stat;
        const ElapsedTimer m_timer;
    };

    struct CTime
    {
        static Optional<CTime> get_current_date_time();
        static Optional<CTime> parse(ZStringView str);

        constexpr CTime() noexcept : m_tm{} { }
        explicit constexpr CTime(tm t) noexcept : m_tm{t} { }

        CTime add_hours(const int hours) const;

        std::string to_string() const;

        std::string strftime(const char* format) const;

        std::chrono::system_clock::time_point to_time_point() const;

    private:
        mutable tm m_tm;
    };

    Optional<tm> to_utc_time(const std::time_t& t);

    tm get_current_date_time();

    tm get_current_date_time_local();
}

template<class Char>
struct fmt::formatter<vcpkg::ElapsedTime, Char>
{
    constexpr auto parse(format_parse_context& ctx) const -> decltype(ctx.begin())
    {
        return vcpkg::basic_format_parse_impl(ctx);
    }
    template<class FormatContext>
    auto format(const vcpkg::ElapsedTime& time, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::formatter<std::string, Char>{}.format(time.to_string(), ctx);
    }
};
