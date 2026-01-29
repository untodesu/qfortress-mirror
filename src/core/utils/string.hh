#ifndef CORE_UTILS_STRING_HH
#define CORE_UTILS_STRING_HH
#pragma once

#include "core/concepts.hh"

namespace utils
{
template<qf::char_type T>
bool is_whitespace(std::basic_string_view<T> string) noexcept;
template<qf::char_type T>
bool has_whitespace(std::basic_string_view<T> string) noexcept;
template<qf::char_type T>
std::basic_string_view<T> skip_whitespace(std::basic_string_view<T> string) noexcept;
template<qf::char_type T>
std::basic_string_view<T> trim_whitespace(std::basic_string_view<T> string) noexcept;
} // namespace utils

#endif
