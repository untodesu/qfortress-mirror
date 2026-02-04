#include "core/pch.hh"

#include "core/level/loader.hh"

#include "core/buffer.hh"
#include "core/exceptions.hh"
#include "core/level/bnode.hh"
#include "core/level/level.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

constexpr static std::uint8_t MAGIC_BYTE_0 = 'Q';
constexpr static std::uint8_t MAGIC_BYTE_1 = 'F';
constexpr static std::uint8_t MAGIC_BYTE_2 = 'L';
constexpr static std::uint8_t MAGIC_BYTE_3 = 'V';

constexpr static std::uint32_t QFLV_VERSION = 1;

constexpr static std::uint32_t LUMP_GEO = 1; ///< Geometry nodes
constexpr static std::uint32_t LUMP_PVS = 2; ///< Potentially visible set
constexpr static std::uint32_t LUMP_ENT = 3; ///< Entity data as a JSON string
constexpr static std::uint32_t LUMP_RAD = 4; ///< Lightmaps
constexpr static std::uint32_t LUMP_VTX = 5; ///< Vertex soup to be uploaded directly into a VBO
constexpr static std::uint32_t LUMP_IDX = 6; ///< Index soup to be uploaded directly into an IBO

struct ProtoBNode final {
    float plane_coefs[4];
    std::int32_t leaf_index;
    std::int32_t front;
    std::int32_t back;
};

static void read_geometry(ReadBuffer& buffer)
{
    auto nodecnt = buffer.read<std::uint32_t>();

    std::vector<ProtoBNode> protonodes;
    protonodes.reserve(nodecnt);

    qf::throw_if<std::runtime_error>(nodecnt == 0, "empty geometry lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        ProtoBNode protonode;
        protonode.plane_coefs[0] = buffer.read<float>();
        protonode.plane_coefs[1] = buffer.read<float>();
        protonode.plane_coefs[1] = buffer.read<float>();
        protonode.plane_coefs[2] = buffer.read<float>();
        protonode.leaf_index = buffer.read<std::int32_t>();
        protonode.front = buffer.read<std::int32_t>();
        protonode.back = buffer.read<std::int32_t>();

        protonodes.emplace_back(std::move(protonode));
    }

    std::vector<std::shared_ptr<BNode>> nodes;
    nodes.reserve(nodecnt);

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        auto node = std::make_shared<BNode>();
        auto& protonode = protonodes[i];

        if(protonode.leaf_index > 0) {
            BNode::Leaf leaf;
            leaf.index = protonode.leaf_index;

            node->data = std::move(leaf);
        }
        else {
            BNode::Internal internal;
            internal.plane = Eigen::Hyperplane<float, 3>(Eigen::Map<Eigen::Vector3f>(&protonode.plane_coefs[0]), protonode.plane_coefs[3]);
            internal.front = nullptr;
            internal.back = nullptr;

            node->data = internal;
        }

        nodes.emplace_back(std::move(node));
    }

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        auto& node = nodes[i];
        auto& protonode = protonodes[i];

        if(auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto front_index = protonode.front;
            auto back_index = protonode.back;

            qf::throw_if<std::runtime_error>(front_index < 0 || front_index >= nodes.size(), "invalid front child index");
            qf::throw_if<std::runtime_error>(back_index < 0 || back_index >= nodes.size(), "invalid back child index");

            internal->front = nodes[front_index];
            internal->back = nodes[back_index];
        }
    }

    level::root = nodes[0];
}

static void read_pvs(ReadBuffer& buffer)
{
    auto nodecnt = buffer.read<std::uint32_t>();

    qf::throw_if<std::runtime_error>(nodecnt == 0, "empty PVS lump");
    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

    std::vector<std::uint32_t> pvsentry;
    pvsentry.reserve(nodecnt);

    for(std::uint32_t i = 0; i < nodecnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        pvsentry.clear();

        for(std::uint32_t j = 0; j < nodecnt; ++i) {
            qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

            pvsentry.push_back(buffer.read<std::uint32_t>());
        }

        level::PVS.push_back(pvsentry);
    }

    qf::throw_if_not<std::runtime_error>(nodecnt == level::PVS.size(), "PVS size mismatch");
}

static void read_entities(ReadBuffer& buffer)
{
    // empty
}

static void read_lightmaps(ReadBuffer& buffer)
{
    // empty
}

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
    auto lumpcnt = buffer.read<std::uint32_t>();

    qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");
    qf::throw_if_not<std::runtime_error>(version == QFLV_VERSION, "unsupported file version");
    qf::throw_if_not<std::runtime_error>(lumpcnt > 1, "no lumps present");

    level::purge();

    for(std::uint32_t i = 0; i < lumpcnt; ++i) {
        qf::throw_if<std::runtime_error>(buffer.is_ended(), "unexpected end-of-file");

        auto lumptype = buffer.read<std::uint32_t>();

        switch(lumptype) {
            case LUMP_GEO:
                read_geometry(buffer);
                break;

            case LUMP_PVS:
                read_pvs(buffer);
                break;

            case LUMP_ENT:
                read_entities(buffer);
                break;

            case LUMP_RAD:
                read_lightmaps(buffer);
                break;

            default:
                throw qf::runtime_error("unknown lump type: {}", lumptype);
        }
    }

    if(!buffer.is_ended()) {
        LOG_WARNING("{}: garbage data after expected end-of-file", path_unfucked);
    }
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
