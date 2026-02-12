#ifndef CORE_UTILS_PHYSFS_HH
#define CORE_UTILS_PHYSFS_HH
#pragma once

namespace utils
{
bool read_file(std::string_view path, std::string& buffer);
bool read_file(std::string_view path, std::vector<std::byte>& buffer);
bool read_file(std::string_view path, std::istringstream& stream);
} // namespace utils

namespace utils
{
bool write_file(std::string_view path, std::string_view buffer, bool append = false);
bool write_file(std::string_view path, std::span<const std::byte> buffer, bool append = false);
} // namespace utils

namespace utils
{
std::string_view physfs_error(void);
} // namespace utils

#endif
