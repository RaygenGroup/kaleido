#pragma once

#include "assets/texture/Texture.h"

// rgba T(float, byte, short - w/e stb supports) texture
class CubeMap : public Asset
{
	std::shared_ptr<Texture> m_faces[CMF_COUNT];

	// must be same for all faces
	uint32 m_width;
	uint32 m_height;

	DynamicRange m_dynamicRange;

public:
	CubeMap(AssetManager* assetManager, const std::string& path)
		: Asset(assetManager, path),
		  m_width(0),
		  m_height(0),
		  m_dynamicRange() {}

	bool Load(const std::string& path, DynamicRange dr, bool flipVertically);
	void Clear() override;

	uint32 GetWidth() const { return m_width; }
	uint32 GetHeight() const { return m_height; }

	Texture* GetFace(CubeMapFace faceIndex) const { return m_faces[faceIndex].get(); }

	DynamicRange GetType() const { return m_dynamicRange; }

	void ToString(std::ostream& os) const override { os << "asset-type: CubeMap, name: " << m_fileName << ", type: " << TexelEnumToString(m_dynamicRange); }
};
