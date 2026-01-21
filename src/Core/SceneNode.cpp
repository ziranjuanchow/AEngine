#include "SceneNode.h"
#include <glm/gtc/matrix_transform.hpp>

namespace AEngine {

    FSceneNode::FSceneNode(const std::string& name)
        : m_name(name) {}

    void FSceneNode::AddChild(std::unique_ptr<FSceneNode> child) {
        child->SetParent(this);
        m_children.push_back(std::move(child));
    }

    void FSceneNode::RemoveChild(FSceneNode* child) {
        std::erase_if(m_children, [&](const std::unique_ptr<FSceneNode>& ptr) {
            return ptr.get() == child;
        });
    }

    void FSceneNode::UpdateWorldMatrix(bool parentDirty) {
        bool dirty = m_dirty || parentDirty;

        if (dirty) {
            // Compute local matrix: T * R * S
            m_localMatrix = glm::translate(glm::mat4(1.0f), m_localPosition) *
                            glm::mat4_cast(m_localRotation) *
                            glm::scale(glm::mat4(1.0f), m_localScale);

            if (m_parent) {
                m_worldMatrix = m_parent->GetWorldMatrix() * m_localMatrix;
            } else {
                m_worldMatrix = m_localMatrix;
            }

            m_dirty = false;
        }

        // Propagate to children
        for (auto& child : m_children) {
            child->UpdateWorldMatrix(dirty);
        }
    }

}
