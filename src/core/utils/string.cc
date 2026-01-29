#include "core/pch.hh"

#include "core/utils/string.hh"

template<qf::char_type T>
constexpr static std::array<T, 7> WHITESPACE_CHARS = { ' ', '\t', '\n', '\v', '\f', '\r', 0 };

template<qf::char_type T>
static bool is_whitespace_char(const T character) noexcept
{
    return std::ranges::any_of(WHITESPACE_CHARS<T>, [character](const T whitespace_char) {
        return character == whitespace_char;
    });
}

template<qf::char_type T>
bool utils::is_whitespace(std::basic_string_view<T> string) noexcept
{
    auto result = false;
    result = result || string.empty();
    result = result || string.size() == 1 && string[0] == 0;
    result = result || string.find_first_not_of(WHITESPACE_CHARS<T>.data()) == std::basic_string_view<T>::npos;
    return result;
}

template<qf::char_type T>
bool utils::has_whitespace(std::basic_string_view<T> string) noexcept
{
    return string.find_first_of(WHITESPACE_CHARS<T>.data()) != std::basic_string_view<T>::npos;
}

template<qf::char_type T>
std::basic_string_view<T> utils::skip_whitespace(std::basic_string_view<T> string) noexcept
{
    std::size_t position = 0;

    while(position < string.size() && is_whitespace_char<T>(string[position])) {
        position += 1;
    }

    return string.substr(position);
}

template<qf::char_type T>
std::basic_string_view<T> utils::trim_whitespace(std::basic_string_view<T> string) noexcept
{
    auto start = string.find_first_not_of(WHITESPACE_CHARS<T>.data());
    auto end = string.find_last_not_of(WHITESPACE_CHARS<T>.data());

    if(start == std::basic_string_view<T>::npos || end == std::basic_string_view<T>::npos) {
        return {};
    }

    return string.substr(start, end - start + 1);
}

template bool utils::is_whitespace<char>(std::basic_string_view<char> string) noexcept;
template bool utils::is_whitespace<wchar_t>(std::basic_string_view<wchar_t> string) noexcept;
template bool utils::is_whitespace<char8_t>(std::basic_string_view<char8_t> string) noexcept;
template bool utils::is_whitespace<char16_t>(std::basic_string_view<char16_t> string) noexcept;
template bool utils::is_whitespace<char32_t>(std::basic_string_view<char32_t> string) noexcept;

template bool utils::has_whitespace<char>(std::basic_string_view<char> string) noexcept;
template bool utils::has_whitespace<wchar_t>(std::basic_string_view<wchar_t> string) noexcept;
template bool utils::has_whitespace<char8_t>(std::basic_string_view<char8_t> string) noexcept;
template bool utils::has_whitespace<char16_t>(std::basic_string_view<char16_t> string) noexcept;
template bool utils::has_whitespace<char32_t>(std::basic_string_view<char32_t> string) noexcept;

template std::basic_string_view<char> utils::skip_whitespace<char>(std::basic_string_view<char> string) noexcept;
template std::basic_string_view<wchar_t> utils::skip_whitespace<wchar_t>(std::basic_string_view<wchar_t> string) noexcept;
template std::basic_string_view<char8_t> utils::skip_whitespace<char8_t>(std::basic_string_view<char8_t> string) noexcept;
template std::basic_string_view<char16_t> utils::skip_whitespace<char16_t>(std::basic_string_view<char16_t> string) noexcept;
template std::basic_string_view<char32_t> utils::skip_whitespace<char32_t>(std::basic_string_view<char32_t> string) noexcept;

template std::basic_string_view<char> utils::trim_whitespace<char>(std::basic_string_view<char> string) noexcept;
template std::basic_string_view<wchar_t> utils::trim_whitespace<wchar_t>(std::basic_string_view<wchar_t> string) noexcept;
template std::basic_string_view<char8_t> utils::trim_whitespace<char8_t>(std::basic_string_view<char8_t> string) noexcept;
template std::basic_string_view<char16_t> utils::trim_whitespace<char16_t>(std::basic_string_view<char16_t> string) noexcept;
template std::basic_string_view<char32_t> utils::trim_whitespace<char32_t>(std::basic_string_view<char32_t> string) noexcept;
