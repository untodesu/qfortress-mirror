#ifndef CORE_CONCEPTS_HH
#define CORE_CONCEPTS_HH
#pragma once

namespace qf
{
template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;
template<typename T>
concept signed_arithmetic = std::is_arithmetic_v<T> && std::is_signed_v<T>;
template<typename T>
concept unsigned_arithmetic = std::is_arithmetic_v<T> && std::is_unsigned_v<T>;
} // namespace qf

namespace qf
{
template<typename T>
concept char_type = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t> || std::same_as<T, char16_t>
    || std::same_as<T, char32_t>;
} // namespace qf

namespace qf
{
template<typename T>
concept derived_exception = std::derived_from<T, std::runtime_error> || std::derived_from<T, std::logic_error>;
} // namespace qf

#endif
