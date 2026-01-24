#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "../RHI/RenderGraph.h"

namespace AEngine {

    class FSceneNode {
    public:
        FSceneNode(const std::string& name = "Node");
        ~FSceneNode() = default;

        // Components
        void AddRenderable(const FRenderable& renderable) { m_renderables.push_back(renderable); }
        const std::vector<FRenderable>& GetRenderables() const { return m_renderables; }
        std::vector<FRenderable>& GetRenderablesMutable() { return m_renderables; } 

        void SetPointLight(const FPointLight& light) { m_pointLight = light; }
        std::optional<FPointLight>& GetPointLight() { return m_pointLight; }
        bool HasPointLight() const { return m_pointLight.has_value(); }

        // TRS
        void SetPosition(const glm::vec3& pos) { m_localPosition = pos; m_dirty = true; }
        void SetRotation(const glm::quat& rot) { m_localRotation = rot; m_dirty = true; }
        void SetScale(const glm::vec3& scale) { m_localScale = scale; m_dirty = true; }

        const glm::vec3& GetPosition() const { return m_localPosition; }
        const glm::quat& GetRotation() const { return m_localRotation; }
        const glm::vec3& GetScale() const { return m_localScale; }

        // Hierarchy
        void AddChild(std::unique_ptr<FSceneNode> child);
        void RemoveChild(FSceneNode* child);
        void SetParent(FSceneNode* parent) { m_parent = parent; m_dirty = true; }
        FSceneNode* GetParent() const { return m_parent; }

        // Update matrices
        // parentDirty indicates if the parent's world matrix has changed
        void UpdateWorldMatrix(bool parentDirty = false);
        const glm::mat4& GetWorldMatrix() const { return m_worldMatrix; }

        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        bool IsVisible() const { return m_isVisible; }
        void SetVisible(bool visible) { m_isVisible = visible; }

        const std::vector<std::unique_ptr<FSceneNode>>& GetChildren() const { return m_children; }

    private:
        std::string m_name;
        bool m_isVisible = true;
        std::vector<FRenderable> m_renderables;
        std::optional<FPointLight> m_pointLight;
        
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
