#ifndef CORE_PATHS_HH
#define CORE_PATHS_HH
#pragma once

namespace paths
{
const std::filesystem::path& gamepath(void);
const std::filesystem::path& userpath(void);
} // namespace paths

namespace paths
{
void init(void);
} // namespace paths

#endif
