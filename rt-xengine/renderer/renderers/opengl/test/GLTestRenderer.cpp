#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "assets/AssetManager.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"

namespace OpenGL
{
	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto vertexSimpleShaderSource = GetAssetManager()->LoadStringFileAsset("test/test.vert");
		//auto vertexInstancedShaderSource = GetDiskAssetManager()->LoadStringFileAsset("test/test_instanced.vert");
		auto fragmentShaderSource = GetAssetManager()->LoadStringFileAsset("test/test.frag");

		m_nonInstancedShader = GetGLAssetManager()->RequestGLShader(vertexSimpleShaderSource.get(), fragmentShaderSource.get());
		m_nonInstancedShader->SetUniformLocation("mvp");
		m_nonInstancedShader->SetUniformLocation("m");
		m_nonInstancedShader->SetUniformLocation("normalMatrix");
		m_nonInstancedShader->SetUniformLocation("mode");
		m_nonInstancedShader->SetUniformLocation("viewPos");
		m_nonInstancedShader->SetUniformLocation("baseColorFactor");
		m_nonInstancedShader->SetUniformLocation("emissiveFactor");
		m_nonInstancedShader->SetUniformLocation("metallicFactor");
		m_nonInstancedShader->SetUniformLocation("roughnessFactor");
		m_nonInstancedShader->SetUniformLocation("normalScale");
		m_nonInstancedShader->SetUniformLocation("occlusionStrength");
		m_nonInstancedShader->SetUniformLocation("alphaMode");
		m_nonInstancedShader->SetUniformLocation("alphaCutoff");
		m_nonInstancedShader->SetUniformLocation("doubleSided");
		m_nonInstancedShader->SetUniformLocation("baseColorSampler");
		m_nonInstancedShader->SetUniformLocation("metallicRoughnessSampler");
		m_nonInstancedShader->SetUniformLocation("emissiveSampler");
		m_nonInstancedShader->SetUniformLocation("normalSampler");
		m_nonInstancedShader->SetUniformLocation("occlusionSampler");
	
		auto* user = GetWorld()->GetAvailableNodeSpecificSubType<FreeformUserNode>();

		RT_XENGINE_ASSERT_RETURN_FALSE(user, "Missing freeform user node!");

		m_camera = user->GetCamera();

		// TODO: fix instanced geometry
		//for (auto* geometryNode : GetWorld()->GetNodeMap<World::TriangleModelInstancedGeometryNode>())
		//	m_instancedGeometries.emplace_back(RequestGLInstancedModel(geometryNode));

		for (auto* geometryNode : GetWorld()->GetNodeMap<TriangleModelGeometryNode>())
			m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, GLTestGeometry>(this, geometryNode));

		//auto* sky = GetWorld()->GetAvailableNodeSpecificSubType<World::SkyHDRNode>();

		//RT_XENGINE_ASSERT_RETURN_FALSE(sky, "Missing freeform user node!");

		//m_skyTexture = RequestGLTexture(sky->GetSkyHDR());

		glViewport(0, 0, width, height);

		return true;
	}

	// REFACT:
	//void GLTestRenderer::WindowResize(int32 width, int32 height)
	//{
	//	glViewport(0, 0, width, height);
	//}

	void GLTestRenderer::Render()
	{
		auto bgcl = GetWorld()->GetRoot()->GetBackgroundColor();

		glm::mat4 vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

		glClearColor(bgcl.r, bgcl.g, bgcl.b, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glUseProgram(m_nonInstancedShader->GetGLHandle());

		// global uniforms
		glUniform3fv(m_nonInstancedShader->GetUniformLocation("viewPos"), 1, &m_camera->GetWorldTranslation()[0]);
		glUniform1i(m_nonInstancedShader->GetUniformLocation("mode"), m_previewMode);

		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = vp * m;

			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("mvp"), 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(m_nonInstancedShader->GetUniformLocation("m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix3fv(m_nonInstancedShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &glm::transpose(glm::inverse(glm::mat3(m)))[0][0]);

			for (auto& glMesh : geometry->glModel->GetGLMeshes())
			{
				glBindVertexArray(glMesh.vao);

				auto& glMaterial = glMesh.material;
				
				glUniform4fv(m_nonInstancedShader->GetUniformLocation("baseColorFactor"), 1, &glMaterial.baseColorFactor[0]);
				glUniform3fv(m_nonInstancedShader->GetUniformLocation("emissiveFactor"), 1, &glMaterial.emissiveFactor[0]);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("metallicFactor"), glMaterial.metallicFactor);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("roughnessFactor"), glMaterial.roughnessFactor);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("normalScale"), glMaterial.normalScale);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("occlusionStrength"), glMaterial.occlusionStrength);
				glUniform1i(m_nonInstancedShader->GetUniformLocation("alphaMode"), glMaterial.alphaMode);
				glUniform1f(m_nonInstancedShader->GetUniformLocation("alphaCutoff"), glMaterial.alphaCutoff);
				glUniform1i(m_nonInstancedShader->GetUniformLocation("doubleSided"), glMaterial.doubleSided);
				
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("baseColorSampler"), glMaterial.baseColorTexture->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("metallicRoughnessSampler"), glMaterial.metallicRoughnessTexture->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("emissiveSampler"), glMaterial.emissiveTexture->GetGLBindlessHandle());
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("occlusionSampler"), glMaterial.occlusionTexture->GetGLBindlessHandle());

				// may not exist
				const auto normalText = glMaterial.normalTexture;
				glUniformHandleui64ARB(m_nonInstancedShader->GetUniformLocation("normalSampler"), normalText ? normalText->GetGLBindlessHandle() : 0);

				glMaterial.doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
									
				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		
				glBindVertexArray(0);
			}
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLTestRenderer::Update()
	{
		if (GetEngine()->GetInput().IsKeyPressed(XVirtualKey::K1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PT_COUNT - 1 : m_previewMode - 1;
			LOG_INFO("Preview Mode set to: {}({})", SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
		else if (GetEngine()->GetInput().IsKeyPressed(XVirtualKey::K2))
		{
			++m_previewMode %= PT_COUNT;
			LOG_INFO("Preview Mode set to: {}({})", SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
	}
}
