#pragma once


#include "core/reflection/GenMacros.h" // include gen macros here even if not needed to propagate to all node headers
#include "tinyxml2/tinyxml2.h"
#include "system/Object.h"
#include <bitset>

class AssetManager;
class World;

// Properly pads the dirty flags given to account for the parent class's dirty flags.
#define DECLARE_DIRTY_FLAGSET(...) public: struct DF { enum { _PREV = Parent::DF::_COUNT - 1, __VA_ARGS__, _COUNT }; }; private:

class Node : public Object
{
	//
	// REFLECTION BASE 
	//
	// Stuff required and used by reflection that are specific to base node class.
	// This is similar to the macro code-gen but with a few changes for the base class.

public:
	using Parent = Node;
	
	[[nodiscard]]
	virtual const ReflClass& GetClass() const
	{
		return Node::StaticClass();
	}

	[[nodiscard]]
	virtual const ReflClass& GetParentClass() const
	{
		return Parent::StaticClass();
	}

	[[nodiscard]]
	static const ReflClass& StaticClass()
	{
		static ReflClass cl = ReflClass::Generate<Node>();
		return cl;
	}
private:
	friend class ReflClass;								
	static void GenerateReflection(ReflClass& refl)
	{
		// Add reflected variables here.
	}


	// Base for dirty flagsets, use the macros. DOC
public:
	struct DF
	{
		enum {
			Transform,
			Hierarchy,
			Children,
			Properties,
			Created,
			_COUNT
		};
	};


protected:
	// local
	glm::vec3 m_localTranslation;
	glm::quat m_localOrientation;
	glm::vec3 m_localScale;
	glm::mat4 m_localMatrix;

	// world
	glm::vec3 m_worldTranslation;
	glm::quat m_worldOrientation;
	glm::vec3 m_worldScale;
	glm::mat4 m_worldMatrix;

	std::bitset<64> m_dirty;

protected:
	// TODO: remove
	std::string m_type;
	
	Node* m_parent;

	// for now ownership is given to parent nodes (later on, world should be manager) 
	std::vector<std::shared_ptr<Node>> m_children;
	

private:
	// mark dirty self and children
	void MarkMatrixChanged();
	
	friend class Editor;
	friend class World;
public:
	Node();
	// Nodes have pObject = parentNode->GetWorld() = World
	Node(Node* pNode);
	virtual ~Node() = default;

	glm::vec3 GetLocalTranslation() const { return m_localTranslation; }
	glm::quat GetLocalOrientation() const { return m_localOrientation; }
	glm::vec3 GetLocalPYR() const { return glm::degrees(glm::eulerAngles(m_localOrientation)); }
	glm::vec3 GetLocalScale() const { return m_localScale; }		
	glm::mat4 GetLocalMatrix() const { return m_localMatrix; }

	void SetLocalTranslation(const glm::vec3& lt);
	void SetLocalOrientation(const glm::quat& lo);
	void SetLocalScale(const glm::vec3& ls);
	void SetLocalMatrix(const glm::mat4& lm);

	glm::vec3 GetWorldTranslation() const { return m_worldTranslation; }
	glm::quat GetWorldOrientation() const { return m_worldOrientation; }
	glm::vec3 GetWorldScale() const { return m_worldScale; }
	glm::mat4 GetWorldMatrix() const { return m_worldMatrix; }

	glm::vec3 GetUp() const { return GetWorldOrientation() * glm::vec3(0.f, 1.f, 0.f); }
	glm::vec3 GetRight() const { return GetWorldOrientation() * glm::vec3(1.f, 0.f, 0.f); }
	glm::vec3 GetFront() const { return GetWorldOrientation() * glm::vec3(0.f, 0.f, -1.f); }

	// override if focalLength is defined
	virtual glm::vec3 GetLookAt() const { return GetWorldTranslation() + GetFront(); }

	glm::mat4 GetViewMatrix() const { return glm::lookAt(GetWorldTranslation(), GetLookAt(), GetUp()); }

	bool IsLeaf() const { return m_children.empty(); }
	const std::string& GetType() const { return m_type; }

	const std::vector<std::shared_ptr<Node>>& GetChildren() { return m_children; }
	
	std::bitset<64> GetDirtyFlagset() const { return m_dirty; }

	// Returns nullptr IF AND ONLY IF "this" node is the root node.
	[[nodiscard]] Node* GetParent() const { return m_parent; }

	void AddChild(std::shared_ptr<Node> child) { m_dirty.set(DF::Children); m_children.emplace_back(child); }

	void DeleteChild(Node* child);
	//
	// LOADING
	//
	bool LoadFromXML(const tinyxml2::XMLElement* xmlData);

	virtual bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData);
	virtual void LoadReflectedProperties(const tinyxml2::XMLElement* xmlData);

	virtual void PropertyUpdatedFromEditor(const Property& prop) { }; // the m_dirtyBitset Property is set directly through the editor before this call.

	// Override loading of a specific child. 
	// If you return a non null pointer here the default node creation will be skipped for this child
	virtual Node* LoadSpecificChild(const std::string& type) { return nullptr; }

	// Called after all children have been loaded from the scene file.
	// You can use this to track 'custom' children in 
	virtual bool PostChildrenLoaded() { return true; }
		
	// cache world transform bottom up (and where needed to be updated)
	void UpdateTransforms(const glm::mat4& parentMatrix);

	void AddLocalOffset(const glm::vec3& direction);

	// not tested
	void Orient(float yaw, float pitch, float roll);

	void OrientWithoutRoll(float yaw, float pitch);

	void OrientYaw(float yaw);

	virtual void Update(float deltaSeconds) {};

	// Runs late in the frame, only on nodes that at least one m_dirty is set.
	virtual void DirtyUpdate() {};


	//
	// Child Getter Utilities
	//

	// Returns a valid child if it is the ONLY child of this class
	// Only checks first level children
	template<class NodeClass>
	NodeClass* GetUniqueChildOfClass() const 
	{
		NodeClass* first = nullptr;
		for (auto child : m_children) 
		{
			auto ptr = dynamic_cast<NodeClass*>(child.get());
			if (ptr) 
			{
				// check if we already found something
				if (first)
				{
					return nullptr;
				}
				else 
				{
					first = ptr;
				}
			}
		}
		return first;
	}

	// Only checks first level children
	template<class NodeClass>
	NodeClass* GetFirstChildOfClass() const 
	{
		for (auto child : m_children) 
		{
			auto ptr = dynamic_cast<NodeClass*>(child.get());
			if (ptr) 
			{
				return ptr;
			}
		}
		return nullptr;
	}

	// Returns a valid child if it is the only child of this (class AND type)
	// 
	// eg: 
	// | - VRHead Head
	// | 1 -- CameraNode* "RightEye"
	// | 2 -- CameraNode* "LeftEye"
	// 
	// GetUniqueChildWithType<CameraNode>("rightEye") == *1
	// 
	// You can use the non templated version to not check for the actual cpp class type.
	// Only checks first level children
	//
	template<class NodeClass = Node>
	NodeClass* GetUniqueChildWithType(const std::string& type) const
	{
		NodeClass* first = nullptr;
		for (auto child : m_children)
		{
			auto ptr = dynamic_cast<NodeClass*>(child.get());
			if (ptr && ptr->GetType() == type)
			{
				// check if we already found something
				if (first)
				{
					return nullptr;
				}
				else
				{
					first = ptr;
				}
			}
		}
		return first;
	}

	// Child must match both NodeClass AND type
	// You can use the non templated version to not check for the actual cpp class type.
	// Only checks first level children
	template<class NodeClass = Node>
	NodeClass* GetFirstChildOfType(const std::string& type) const
	{
		auto it = std::find_if(m_children.begin(), m_children.end(), [&](auto child) 
		{
			return child->GetType() == type && dynamic_cast<NodeClass*>(child.get());
		});

		if (it == m_children.end()) 
		{
			return nullptr;
		}
		return dynamic_cast<NodeClass*>(child.get());
	}
};
