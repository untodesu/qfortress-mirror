#ifndef CORE_CONFIG_ARITHMETIC_HH
#define CORE_CONFIG_ARITHMETIC_HH
#pragma once

#include "core/concepts.hh"
#include "core/config/value.hh"

template<qf::arithmetic T>
class ConfigArithmetic final : public ConfigValue {
public:
    constexpr static const T MIN_VALUE = std::numeric_limits<T>::lowest();
    constexpr static const T MAX_VALUE = std::numeric_limits<T>::max();

    explicit ConfigArithmetic(std::string_view name, T default_value);
    explicit ConfigArithmetic(std::string_view name, T default_value, T min_value, T max_value);
    virtual ~ConfigArithmetic(void) override = default;

    constexpr operator T(void) const noexcept;
    constexpr T arithmetic(void) const noexcept;
    void set_arithmetic(T value) noexcept;

    constexpr T min_value(void) const noexcept;
    void set_min_value(T value) noexcept;

    constexpr T max_value(void) const noexcept;
    void set_max_value(T value) noexcept;

    void set_limits(T min_value, T max_value, bool clamp_value = true) noexcept;

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    std::string m_string;
    T m_arithmetic;
    T m_min_value;
    T m_max_value;
};

using ConfigInt = ConfigArithmetic<int>;
using ConfigFloat = ConfigArithmetic<float>;
using ConfigDouble = ConfigArithmetic<double>;
using ConfigUnsigned = ConfigArithmetic<unsigned>;
using ConfigSizeType = ConfigArithmetic<std::size_t>;

template<qf::arithmetic T>
ConfigArithmetic<T>::ConfigArithmetic(std::string_view name, T default_value)
    : ConfigValue(name), m_arithmetic(default_value), m_min_value(MIN_VALUE), m_max_value(MAX_VALUE)
{
    assert(m_arithmetic >= m_min_value);
    assert(m_arithmetic <= m_max_value);
}

template<qf::arithmetic T>
inline ConfigArithmetic<T>::ConfigArithmetic(std::string_view name, T default_value, T min_value, T max_value)
    : ConfigValue(name), m_arithmetic(default_value), m_min_value(min_value), m_max_value(max_value)
{
    assert(m_max_value >= m_min_value);
    assert(m_arithmetic >= m_min_value);
    assert(m_arithmetic <= m_max_value);
}

template<qf::arithmetic T>
inline constexpr ConfigArithmetic<T>::operator T(void) const noexcept
{
    return m_arithmetic;
}

template<qf::arithmetic T>
inline constexpr T ConfigArithmetic<T>::arithmetic(void) const noexcept
{
    return m_arithmetic;
}

template<qf::arithmetic T>
inline void ConfigArithmetic<T>::set_arithmetic(T value) noexcept
{
    m_arithmetic = std::clamp<T>(value, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<qf::arithmetic T>
inline constexpr T ConfigArithmetic<T>::min_value(void) const noexcept
{
    return m_min_value;
}

template<qf::arithmetic T>
inline void ConfigArithmetic<T>::set_min_value(T value) noexcept
{
    assert(value <= m_max_value);

    m_min_value = value;
    m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<qf::arithmetic T>
inline constexpr T ConfigArithmetic<T>::max_value(void) const noexcept
{
    return m_max_value;
}

template<qf::arithmetic T>
inline void ConfigArithmetic<T>::set_max_value(T value) noexcept
{
    assert(value >= m_min_value);

    m_max_value = value;
    m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<qf::arithmetic T>
inline void ConfigArithmetic<T>::set_limits(T min_value, T max_value, bool clamp_value) noexcept
{
    assert(min_value <= max_value);

    m_min_value = min_value;
    m_max_value = max_value;

    if(clamp_value) {
        m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
        m_string = std::to_string(m_arithmetic);
    }
}

template<qf::arithmetic T>
inline std::string_view ConfigArithmetic<T>::value(void) const noexcept
{
    return m_string;
}

template<qf::arithmetic T>
inline bool ConfigArithmetic<T>::set_value(std::string_view value) noexcept
{
    T new_arithmetic;

    auto check = std::from_chars(value.data(), value.data() + value.size(), new_arithmetic);
    auto is_valid = check.ec == std::errc();

    if(is_valid) {
        m_arithmetic = std::clamp<T>(new_arithmetic, m_min_value, m_max_value);
        m_string = std::to_string(m_arithmetic);
    }

    return is_valid;
}

#endif
