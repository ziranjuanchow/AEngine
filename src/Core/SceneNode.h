#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <memory>
#include <string>

namespace AEngine {

    class FSceneNode {
    public:
        FSceneNode(const std::string& name = "Node");
        ~FSceneNode() = default;

        // TRS
        void SetPosition(const glm::vec3& pos) { m_localPosition = pos; m_dirty = true; }
        void SetRotation(const glm::quat& rot) { m_localRotation = rot; m_dirty = true; }
        void SetScale(const glm::vec3& scale) { m_localScale = scale; m_dirty = true; }

        const glm::vec3& GetPosition() const { return m_localPosition; }
        const glm::quat& GetRotation() const { return m_localRotation; }
        const glm::vec3& GetScale() const { return m_localScale; }

        // Hierarchy
        void AddChild(std::unique_ptr<FSceneNode> child);
        void SetParent(FSceneNode* parent) { m_parent = parent; m_dirty = true; }

        // Update matrices
        void UpdateWorldMatrix();
        const glm::mat4& GetWorldMatrix() const { return m_worldMatrix; }

        const std::string& GetName() const { return m_name; }

    private:
        std::string m_name;
        
        glm::vec3 m_localPosition{ 0.0f };
        glm::quat m_localRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 m_localScale{ 1.0f };

        glm::mat4 m_localMatrix{ 1.0f };
        glm::mat4 m_worldMatrix{ 1.0f };

        FSceneNode* m_parent = nullptr;
        std::vector<std::unique_ptr<FSceneNode>> m_children;

        bool m_dirty = true;
    };

}
