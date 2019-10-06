#pragma once

#include "renderer/renderers/opengl/GLAsset.h"
#include "asset/loaders/ShaderLoader.h"

#include "glad/glad.h"
#include "core/timer/Timer.h"

namespace OpenGL
{
	struct GLShader : GLAsset
	{
		GLuint id;

		GLShader(const uri::Uri& assocPath)
			: GLAsset(assocPath) {}
		virtual ~GLShader();
		
		GLint GetUniform(const std::string& uniformName) const;
		void AddUniform(const std::string& uniformName);
		bool Load() override;

	protected:
		// temporary
		std::unordered_map<std::string, GLint> uniformLocations;
	};

}
