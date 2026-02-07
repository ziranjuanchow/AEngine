#include "ForwardLitPass.h"

namespace AEngine {

    void FForwardLitPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (m_outputFramebuffer) {
            m_outputFramebuffer->Bind();
            cmdBuffer.SetViewport(0, 0, m_width, m_height);
        }
        cmdBuffer.SetBlendState(false);
        cmdBuffer.SetDepthTest(true, true, ERHICompareFunc::LessEqual);
        cmdBuffer.SetCullMode(ERHICullMode::Back);
        
        for (const auto& renderable : renderables) {
            if (!renderable.Material) continue;

            // 1. Update Material with Global Context (Camera, Lights)
            renderable.Material->SetParameter("lightPosition", context.LightPosition);
            renderable.Material->SetParameter("lightColor", context.LightColor);
            renderable.Material->SetParameter("camPos", context.CameraPosition);
            
            // 2. Set Transformation Matrices
            // Currently our StandardPBR expects 'model', 'view', 'projection'
            // We need a way to pass these to the material. 
            // In a better design, this would be a Uniform Buffer.
            renderable.Material->SetParameter("model_matrix", renderable.WorldMatrix); // Just placeholder names
            renderable.Material->SetParameter("view_matrix", context.ViewMatrix);
            renderable.Material->SetParameter("projection_matrix", context.ProjectionMatrix);
            renderable.Material->SetParameter("lightSpaceMatrix", context.LightSpaceMatrix);

            // 3. Bind Material (sets GL program and uniforms)
            renderable.Material->Bind();

            // 4. Bind Buffers
            cmdBuffer.SetVertexBuffer(renderable.VertexBuffer);
            cmdBuffer.SetIndexBuffer(renderable.IndexBuffer);

            // 5. Draw
            cmdBuffer.DrawIndexed(renderable.IndexCount);
        }

        if (m_outputFramebuffer) {
            m_outputFramebuffer->Unbind();
        }
    }

}
