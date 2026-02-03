#include "core/pch.hh"

#include "core/level/loader.hh"

#include "core/buffer.hh"
#include "core/exceptions.hh"
#include "core/level/bnode.hh"
#include "core/level/level.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

constexpr static std::uint32_t QFLV_VERSION = 1;

constexpr static std::uint8_t MAGIC_BYTE_0 = 'Q';
constexpr static std::uint8_t MAGIC_BYTE_1 = 'F';
constexpr static std::uint8_t MAGIC_BYTE_2 = 'L';
constexpr static std::uint8_t MAGIC_BYTE_3 = 'V';

void level::load(std::string_view path)
{
    auto path_unfucked = std::string(path);
    auto file = PHYSFS_openRead(path_unfucked.c_str());

    qf::throw_if_not<std::runtime_error>(file, utils::physfs_error());

    ReadBuffer buffer;
    buffer.reset(file);
    PHYSFS_close(file);

    auto magic_0 = buffer.read<std::uint8_t>();
    auto magic_1 = buffer.read<std::uint8_t>();
    auto magic_2 = buffer.read<std::uint8_t>();
    auto magic_3 = buffer.read<std::uint8_t>();

    auto signature_valid = true;
    signature_valid = signature_valid && magic_0 == MAGIC_BYTE_0;
    signature_valid = signature_valid && magic_1 == MAGIC_BYTE_1;
    signature_valid = signature_valid && magic_2 == MAGIC_BYTE_2;
    signature_valid = signature_valid && magic_3 == MAGIC_BYTE_3;

    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");
    qf::throw_if_not<std::runtime_error>(signature_valid, "invalid file signature");

    auto version = buffer.read<std::uint32_t>();

    level::purge();
}

void level::save(std::string_view path)
{
}

bool level::safe_load(std::string_view path)
{
    try {
        level::load(path);
        return true;
    }
    catch(const std::exception& ex) {
        LOG_WARNING("{}: save failed: {}", path, ex.what());
        return false;
    }
    catch(...) {
        LOG_WARNING("{}: save failed", path);
        return false;
    }
}

bool level::safe_save(std::string_view path)
{
    try {
        level::save(path);
        return true;
    }
    catch(const std::exception& ex) {
        LOG_WARNING("{}: load failed: {}", path, ex.what());
        return false;
    }
    catch(...) {
        LOG_WARNING("{}: load failed", path);
        return false;
    }
}
