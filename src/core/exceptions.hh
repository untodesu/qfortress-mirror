#ifndef CORE_EXCEPTIONS_HH
#define CORE_EXCEPTIONS_HH
#pragma once

#include "core/concepts.hh"

template<qf::derived_exception T>
class FormattedException final : public T {
public:
    template<typename... AT>
    explicit FormattedException(std::format_string<AT...> fmt, AT&&... args);
    explicit FormattedException(std::string_view what);
    virtual ~FormattedException(void) = default;
};

#endif

template<qf::derived_exception T>
template<typename... AT>
inline FormattedException<T>::FormattedException(std::format_string<AT...> fmt, AT&&... args)
    : T(std::vformat(fmt.get(), std::make_format_args(args...)))
{
    // empty
}

template<qf::derived_exception T>
inline FormattedException<T>::FormattedException(std::string_view what) : T(std::string(what))
{
    // empty
}

namespace qf
{
using runtime_error = FormattedException<std::runtime_error>;
using range_error = FormattedException<std::range_error>;
using overflow_error = FormattedException<std::overflow_error>;
using underflow_error = FormattedException<std::underflow_error>;
} // namespace qf

namespace qf
{
using logic_error = FormattedException<std::logic_error>;
using invalid_argument = FormattedException<std::invalid_argument>;
using length_error = FormattedException<std::length_error>;
using out_of_range = FormattedException<std::out_of_range>;
} // namespace qf
