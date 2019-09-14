#include "pch.h"

#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"

namespace OpenGL
{
	GLTexture::~GLTexture()
	{
		// TODO: handle bind-less
		glDeleteTextures(1, &m_glId);
	}

	bool GLTexture::Load()
	{
		if (!m_sampler->image)
		{
			return false;
		}

		if (!Engine::GetAssetManager()->Load(m_sampler->image))
			return false;

	
		glGenTextures(1, &m_glId);
		glBindTexture(GL_TEXTURE_2D, m_glId);

		auto minFiltering = GetGLFiltering(m_sampler->minFilter);

		// If you don't use one of the filter values that include mipmaps (like GL_LINEAR_MIPMAP_LINEAR), your mipmaps will not be used in any way.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFiltering(m_sampler->magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);

		GLenum type;
		GLint internalFormat;

		if (m_sampler->image->IsHdr())
		{
			type = GL_FLOAT;
			internalFormat = GL_RGBA32F;
		}
		else
		{
			type = GL_UNSIGNED_BYTE;
			internalFormat = GL_RGBA;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrapping(m_sampler->wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrapping(m_sampler->wrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLWrapping(m_sampler->wrapR));
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_sampler->image->GetWidth(), m_sampler->image->GetHeight(), 0, GL_RGBA, type, m_sampler->image->GetData());


		if (minFiltering == GL_NEAREST_MIPMAP_NEAREST ||
			minFiltering == GL_LINEAR_MIPMAP_NEAREST ||
			minFiltering == GL_NEAREST_MIPMAP_LINEAR ||
			minFiltering == GL_LINEAR_MIPMAP_LINEAR)
			glGenerateMipmap(GL_TEXTURE_2D);

		// TODO if bindless?
		m_bindlessHandle = glGetTextureHandleARB(m_glId);
		glMakeTextureHandleResidentARB(m_bindlessHandle);

		return true;
	}
}
