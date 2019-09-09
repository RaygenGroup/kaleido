#include "pch.h"

#include "world/nodes/geometry/TriangleModelInstancedGeometryNode.h"


TriangleModelInstancedGeometryNode::TriangleModelInstancedGeometryNode(Node* parent)
	: TriangleModelGeometryNode(parent)
{
}

void TriangleModelInstancedGeometryNode::CacheWorldTransform()
{
	Node::CacheWorldTransform();

	m_instanceGroup.UpdateInstances(m_parent->GetWorldMatrix());
}

std::string TriangleModelInstancedGeometryNode::ToString(bool verbose, uint depth) const
{
	// TODO verbose print instances
	return std::string("    ") * depth + "|--TMIgeometry instances: " + std::to_string(m_instanceGroup.GetCount()) + " " + Node::ToString(verbose, depth);
}

bool TriangleModelInstancedGeometryNode::LoadChildrenFromXML(const tinyxml2::XMLElement * xmlData)
{
	// children
	for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
		xmdChildElement = xmdChildElement->NextSiblingElement())
	{
		const std::string type = xmdChildElement->Name();
		std::shared_ptr<Node> node = nullptr;

		if (type == "instance")
		{
			m_instanceGroup.AddInstanceFromXML(xmdChildElement);
		}
		else
		{
			LOG_WARN("Unexpected entity type '{0}', expected \'instance\', skipping...", type);
			continue;
		}
	}

	return true;
}
