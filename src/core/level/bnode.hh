#ifndef CORE_LEVEL_BNODE_HH
#define CORE_LEVEL_BNODE_HH
#pragma once

struct BNode final {
    struct Internal final {
        Eigen::Hyperplane<float, 3> plane;
        std::int32_t front { -1 };
        std::int32_t back { -1 };
    };

    struct Leaf final {
        std::int32_t index;
        std::int32_t ebo_offset;
        std::int32_t ebo_count;
        std::int32_t material;
    };

    std::variant<Internal, Leaf> data;
};

#endif
