#ifndef CORE_LEVEL_BNODE_HH
#define CORE_LEVEL_BNODE_HH
#pragma once

class BNode final {
public:
    struct Internal final {
        Eigen::Hyperplane<float, 3> plane;
        std::shared_ptr<BNode> front;
        std::shared_ptr<BNode> back;
    };

    struct Leaf final {
        int index;
        int ebo_offset;
        int ebo_count;
        std::string material;
    };

    static void enumerate(BNode* node, const Eigen::Vector3f& position, std::vector<BNode*>& out_nodes);
    static void enumerate(const BNode* node, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes);

    BNode(void) = default;
    BNode(const BNode& other) = delete;
    BNode& operator=(const BNode& other) = delete;

    std::variant<Internal, Leaf> data;
};

#endif
