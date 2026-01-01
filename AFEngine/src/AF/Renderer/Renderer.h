#pragma once

#include "RenderCommand.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Renderer/RenderPass.h"

#include "AF/Renderer/Camera.h"
#include "AF/Renderer/EditorCamera.h"

#include "AF/Renderer/Mesh.h"
#include "AF/Renderer/Material.h"
#include "AF/Renderer/UniformBuffer.h"
#include "Shader.h"

namespace AF {

	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene();
		static void EndScene();

		static void BeginRenderPass(const Ref<RenderPass> renderPass);
		static void EndRenderPass();

		static void SubmitFullscreenQuad();

		static void SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const glm::mat4& transform, int entityID = -1);
        static void SubmitMeshShadow(const Ref<Mesh>& mesh, const glm::mat4& transform);
		static void Submit(const std::function<void()>& renderFunc);

        static void ApplyUniforms(const Ref<Shader>& shader, const std::unordered_map<std::string, UniformValue>& m_Uniforms, const bool isPipeline);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:

		// Visitor pattern to apply uniform values to shader
		struct UniformApplier {
            Ref<Shader> shader;
            const std::string& name;
            bool isPipeline;
            int& pipelineUnit;
            int& materialUnit;

            void operator()(int value) const { shader->SetInt(name, value); }
            void operator()(float value) const { shader->SetFloat(name, value); }
            void operator()(const glm::vec2& value) const { shader->SetFloat2(name, value); }
            void operator()(const glm::vec3& value) const { shader->SetFloat3(name, value); }
            void operator()(const glm::vec4& value) const { shader->SetFloat4(name, value); }
            void operator()(const glm::mat3& value) const { shader->SetMat3(name, value); }
            void operator()(const glm::mat4& value) const { shader->SetMat4(name, value); }
            void operator()(const Ref<Texture>& texture) const {
                if (texture) {
                    int texUnit =  pipelineUnit + materialUnit;
                    if (texUnit >= 32) {
                        AF_CORE_ERROR("Texture unit limit exceeded for uniform: {}", name);
                        return;
                    }
                    texture->Bind(texUnit);
                    shader->SetInt(name, texUnit);
                    int& currentCounter = isPipeline ? pipelineUnit : materialUnit;
                    currentCounter++;
                }
                else {
                    AF_CORE_WARN("Attempting to bind null texture for uniform: {}", name);
                }
            }
            //void operator()(const Ref<TextureCube>& texture) const {
            //    if (texture) {
            //        int texUnit = pipelineUnit + materialUnit;
            //        if (texUnit >= 32) {
            //            AF_CORE_ERROR("Texture unit limit exceeded for uniform: {}", name);
            //            return;
            //        }
            //        texture->Bind(texUnit);
            //        shader->SetInt(name, texUnit);
            //        int& currentCounter = isPipeline ? pipelineUnit : materialUnit;
            //        currentCounter++;
            //    }
            //    else {
            //        AF_CORE_WARN("Attempting to bind null texture for uniform: {}", name);
            //    }
            //}
            void operator()(const Ref<UniformBuffer>& ubo) const {
                ubo->Bind();
            }
            void operator()(const Ref<ShaderStorageBuffer>& ssbo) const {
                ssbo->Bind();
            }
            // Add operator() for each type in variant
        };
	};
}
