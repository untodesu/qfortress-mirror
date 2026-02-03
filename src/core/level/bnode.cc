#include "core/pch.hh"

#include "core/level/bnode.hh"

void BNode::enumerate(BNode* node, const Eigen::Vector3f& position, std::vector<BNode*>& out_nodes)
{
    if(node) {
        if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                enumerate(internal->back.get(), position, out_nodes);
                out_nodes.push_back(node);
                enumerate(internal->front.get(), position, out_nodes);
            }
            else {
                enumerate(internal->front.get(), position, out_nodes);
                out_nodes.push_back(node);
                enumerate(internal->back.get(), position, out_nodes);
            }
        }
        else {
            out_nodes.push_back(node);
        }
    }
}

void BNode::enumerate(const BNode* node, const Eigen::Vector3f& position, std::vector<const BNode*>& out_nodes)
{
    if(node) {
        if(const auto internal = std::get_if<BNode::Internal>(&node->data)) {
            auto distance = internal->plane.signedDistance(position);

            if(distance >= 0.0f) {
                enumerate(internal->back.get(), position, out_nodes);
                out_nodes.push_back(node);
                enumerate(internal->front.get(), position, out_nodes);
            }
            else {
                enumerate(internal->front.get(), position, out_nodes);
                out_nodes.push_back(node);
                enumerate(internal->back.get(), position, out_nodes);
            }
        }
        else {
            out_nodes.push_back(node);
        }
    }
}
