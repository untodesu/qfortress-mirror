#ifndef CORE_UTILS_EPOCH_HH
#define CORE_UTILS_EPOCH_HH
#pragma once

namespace utils
{
std::uint64_t epoch_seconds(void);
std::uint64_t epoch_milliseconds(void);
std::uint64_t epoch_microseconds(void);
} // namespace utils

namespace utils
{
std::int64_t signed_epoch_seconds(void);
std::int64_t signed_epoch_milliseconds(void);
std::int64_t signed_epoch_microseconds(void);
} // namespace utils

#endif
