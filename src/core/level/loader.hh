#ifndef CORE_LEVEL_LOADER_HH
#define CORE_LEVEL_LOADER_HH
#pragma once

namespace level
{
void load(std::string_view path);
void save(std::string_view path);
} // namespace level

namespace level
{
bool safe_load(std::string_view path);
bool safe_save(std::string_view path);
} // namespace level

#endif
