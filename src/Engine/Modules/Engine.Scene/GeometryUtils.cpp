#include "GeometryUtils.h"
#include <glm/gtc/constants.hpp>

namespace AEngine {

    void FGeometryUtils::CreateSphere(IRHIDevice& device, std::shared_ptr<IRHIBuffer>& outVB, std::shared_ptr<IRHIBuffer>& outIB, uint32_t& outIndexCount) {
        std::vector<FVertex> vertices;
        std::vector<uint32_t> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = glm::pi<float>();

        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                glm::vec3 pos(xPos, yPos, zPos);
                glm::vec3 normal = pos; // For a unit sphere at origin, normal is same as position
                glm::vec2 uv(xSegment, ySegment);

                // Simple tangent calculation for sphere
                // Tangent is perpendicular to Normal and Up (approx)
                // Or better: derivative of position with respect to u
                // dx = -sin(theta)*sin(phi), dy = 0, dz = cos(theta)*sin(phi)
                float theta = xSegment * 2.0f * PI;
                glm::vec3 tangent(-std::sin(theta), 0.0f, std::cos(theta)); 
                tangent = glm::normalize(tangent);
                
                glm::vec3 bitangent = glm::cross(normal, tangent);

                vertices.push_back({ 
                    pos, 
                    normal, 
                    uv, 
                    tangent, 
                    bitangent, 
                    glm::vec4(1.0f) // White color
                });
            }
        }

        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            }
        }

        outIndexCount = (uint32_t)indices.size();
        outVB = device.CreateBuffer(ERHIBufferType::Vertex, (uint32_t)(vertices.size() * sizeof(FVertex)), ERHIBufferUsage::Static, vertices.data());
        outIB = device.CreateBuffer(ERHIBufferType::Index, (uint32_t)(indices.size() * sizeof(uint32_t)), ERHIBufferUsage::Static, indices.data());
    }

    void FGeometryUtils::CreateQuad(IRHIDevice& device, std::shared_ptr<IRHIBuffer>& outVB, std::shared_ptr<IRHIBuffer>& outIB, uint32_t& outIndexCount) {
        std::vector<FVertex> vertices;
        
        auto addVertex = [&](glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) {
            FVertex v;
            v.Position = pos;
            v.Normal = norm;
            v.TexCoords = uv;
            v.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            v.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
            v.Color = glm::vec4(1.0f);
            vertices.push_back(v);
        };

        addVertex({ -10.0f, -2.0f,  10.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f });
        addVertex({  10.0f, -2.0f,  10.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f });
        addVertex({  10.0f, -2.0f, -10.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f });
        addVertex({ -10.0f, -2.0f, -10.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f });

        std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

        outIndexCount = (uint32_t)indices.size();
        outVB = device.CreateBuffer(ERHIBufferType::Vertex, (uint32_t)(vertices.size() * sizeof(FVertex)), ERHIBufferUsage::Static, vertices.data());
        outIB = device.CreateBuffer(ERHIBufferType::Index, (uint32_t)(indices.size() * sizeof(uint32_t)), ERHIBufferUsage::Static, indices.data());
    }

}
