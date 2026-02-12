#ifndef CORE_VERSION_HH
#define CORE_VERSION_HH
#pragma once

namespace version
{
extern const unsigned major;
extern const unsigned minor;
extern const unsigned patch;
} // namespace version

namespace version
{
extern const std::string_view scm_branch;
extern const std::string_view scm_revision;
extern const std::string_view semantic;
} // namespace version

namespace version
{
extern const std::string_view full;
} // namespace version

#endif
