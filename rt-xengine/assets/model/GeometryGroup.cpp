#include "pch.h"

#include "assets/model/GeometryGroup.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	GeometryGroup::GeometryGroup(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
		  m_mode(GeometryMode::TRIANGLES),
	      m_material(this, "default-mat")
	{
	}
	
	
	namespace
	{
		namespace tg = tinygltf;

		template<typename Output, typename ComponentType>
		void ExtractBufferData(const tg::Model& modelData, int32 accessorIndex, std::vector<Output>& result)
		{
			//
			// Actual example of a possible complex gltf buffer:
			//                                              |     STRIDE  |
			// [{vertexIndexes} * 1000] [{normals} * 1000] [{uv0, position} * 1000]
			//													  ^ beginPtr for Position.
			//

			size_t elementCount;		// How many elements there are to read
			size_t componentCount;		// How many components of type ComponentType there are to each element.
			
			size_t strideByteOffset;	// The number of bytes to move in the buffer after each read to get the next element.
										// This may be more bytes than the actual sizeof(ComponentType) * componentCount
										// if the data is strided.
			
			byte* beginPtr;				// Pointer to the first byte we care about.
										// This may not be the actual start of the buffer of the binary file.

			{
				size_t beginByteOffset;
				const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
				const tinygltf::BufferView& bufferView = modelData.bufferViews.at(accessor.bufferView);
				const tinygltf::Buffer& gltfBuffer = modelData.buffers.at(bufferView.buffer);

				elementCount = accessor.count;
				beginByteOffset = accessor.byteOffset + bufferView.byteOffset;
				strideByteOffset = accessor.ByteStride(bufferView);
				componentCount = GetElementComponentCount(GetElementTypeFromGltf(accessor.type));
				beginPtr = const_cast<byte*>(&gltfBuffer.data[beginByteOffset]);
			}

			result.resize(elementCount);

			for (int32 i = 0; i < elementCount; ++i)
			{
				byte* elementPtr = &beginPtr[strideByteOffset * i];

				// Component Count can be templated and specialized later for per type & count optmizations (ie vectorization)
				ComponentType* data = reinterpret_cast<ComponentType*>(elementPtr);
				for (int32 c = 0; c < componentCount; ++c)
				{
					result[i][c] = data[c]; // normal type conversion, should be able to convert even ints to floats
				}
			}
		}

		// Extracts the type of the out vector and attempts to load any type of data at accessorIndex to this vector.
		template<typename Output>
		void ExtractBufferDataInto(const tg::Model& modelData, int32 accessorIndex, std::vector<Output>& out)
		{
			const tinygltf::Accessor& accessor = modelData.accessors.at(accessorIndex);
			BufferComponentType componentType = GetComponentTypeFromGltf(accessor.componentType);
			
			// This will generate EVERY possible mapping of Output -> ComponentType conversion
			// fix this later and provide empty specializations of "incompatible" types (eg: float -> int)
			// TODO: This code will produce warnings for every type conversion that is considered 'unsafe'

			switch (componentType)
			{
			case BufferComponentType::BYTE:
				ExtractBufferData<Output, char>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::UNSIGNED_BYTE:
				ExtractBufferData<Output, unsigned char>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::SHORT:
				ExtractBufferData<Output, short>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::UNSIGNED_SHORT:
				ExtractBufferData<Output, unsigned short>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::INT:
				ExtractBufferData<Output, int32>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::UNSIGNED_INT:
				ExtractBufferData<Output, unsigned int>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::FLOAT:
				ExtractBufferData<Output, float>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::DOUBLE:
				ExtractBufferData<Output, double>(modelData, accessorIndex, out);
				return;
			case BufferComponentType::INVALID:
				return;
			}
			return;
		}
	}
	
	bool GeometryGroup::Load(const tinygltf::Model& modelData, const tinygltf::Primitive& primitiveData)
	{
		// mode
		m_mode = GetGeometryModeFromGltf(primitiveData.mode);		
		
		// indexing
		const auto indicesIndex = primitiveData.indices;

		if (indicesIndex != -1)
		{
			ExtractBufferDataInto(modelData, indicesIndex, m_indices);
		}
		
		// attributes
		for (auto& attribute : primitiveData.attributes)
		{
			const auto& attrName = attribute.first;
			int32 index = attribute.second;

			if (Core::CaseInsensitiveCompare(attrName, "POSITION"))
			{
				ExtractBufferDataInto(modelData, index, m_positions);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "NORMAL"))
			{
				ExtractBufferDataInto(modelData, index, m_normals);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TANGENT"))
			{
				ExtractBufferDataInto(modelData, index, m_tangents);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TEXCOORD_0"))
			{
				ExtractBufferDataInto(modelData, index, m_textCoords0);
			}
			else if (Core::CaseInsensitiveCompare(attrName, "TEXCOORD_1"))
			{
				ExtractBufferDataInto(modelData, index, m_textCoords1);
			}

		}

		// material
		const auto materialIndex = primitiveData.material;
		
		if (materialIndex != -1)
		{
			auto& mat = modelData.materials.at(materialIndex);

			m_material.Load(modelData, mat);
			m_material.RenameAssetPart(Core::UnnamedDescription(mat.name));
		}

		// if missing positions it fails
		if (m_positions.empty())
			return false;

		// calculate missing normals (flat)
		if (m_normals.empty())
		{
			m_normals.resize(m_positions.size());
			
			if(UsesIndexing())
			{
				for (int32 i = 0; i < m_indices.size(); i+=3)
				{					
					// triangle
					auto p0 = m_positions[m_indices[i].x];
					auto p1 = m_positions[m_indices[i+1].x];
					auto p2 = m_positions[m_indices[i+2].x];
					
					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
					
					m_normals[m_indices[i].x] += n;
					m_normals[m_indices[i+1].x] += n;
					m_normals[m_indices[i+2].x] += n;
				}	
			}
			else
			{
				for (int32 i = 0; i < m_positions.size(); ++i)
				{					
					// triangle
					auto p0 = m_positions[i];
					auto p1 = m_positions[i+1];
					auto p2 = m_positions[i+2];

					glm::vec3 n = glm::cross(p1 - p0, p2 - p0);    // p1 is the 'base' here

					m_normals[i] += n;
					m_normals[i+1] += n;
					m_normals[i+2] += n;
				}
			}

			std::for_each(m_normals.begin(), m_normals.end(), [](glm::vec3& normal) { normal = glm::normalize(normal); });
		}

		// TODO test better calculations (using uv layer 0?) also text tangent handedness
		// calculate missing tangents (and bitangents)
		if (m_tangents.empty())
		{
			std::transform(m_normals.begin(), m_normals.end(), std::back_inserter(m_tangents), [](const glm::vec3& normal)
				{
					const auto c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
					const auto c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
					if (glm::length(c1) > glm::length(c2))
						return glm::vec4(glm::normalize(c1), 1.0f);
					else
						return glm::vec4(glm::normalize(c2), -1.f);
				});
		}

		// calculate missing bitangents
		if (m_bitangents.empty())
		{
			std::transform(m_normals.begin(), m_normals.end(), m_tangents.begin(),
				std::back_inserter(m_bitangents), [](const glm::vec3& normal, const glm::vec4& tangent)
				{
					return glm::normalize(glm::cross(normal, glm::vec3(tangent)) * tangent.w);
				});
		}

		// calculate missing textCoords0 - init zeros
		if (m_textCoords0.empty())
		{
			m_textCoords0.resize(m_positions.size());
		}

		// calculate missing textCoords1 - copy textCoords0
		if (m_textCoords1.empty())
		{
			m_textCoords1 = m_textCoords0;
		}

		// calculate other baked data
		return true;
	}
}
