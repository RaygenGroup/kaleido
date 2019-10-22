#pragma once

#include "reflection/GenMacros.h" // include gen macros here even if not needed to propagate to all node headers
#include "system/Object.h"

#include <bitset>

class AssetManager;
class World;
class RootNode;

class Node;

using DirtyFlagset = std::bitset<64>;
using NodeDeleterFunc = void (*)(Node*);
using NodeUniquePtr = std::unique_ptr<Node, NodeDeleterFunc>;

// Properly pads the dirty flags given to account for the parent class's dirty flags.

class Node : public Object {
	//
	// REFLECTION BASE
	//
	// Stuff required and used by reflection that are specific to base node class.
	// This is similar to the macro code-gen but with a few changes for the base class.
	[[nodiscard]] static ReflClass& Z_MutableClass()
	{
		static ReflClass cl = ReflClass::Generate<Node>();
		return cl;
	}

public:
	using Parent = Node;

	[[nodiscard]] virtual const ReflClass& GetClass() const { return Node::StaticClass(); }

	[[nodiscard]] virtual const ReflClass& GetParentClass() const { return Parent::StaticClass(); }

	[[nodiscard]] static const ReflClass& StaticClass() { return Z_MutableClass(); }

private:
	friend class ReflClass;
	static void GenerateReflection(ReflClass& refl)
	{
		// Node class has no base variables
	}


public:
	// Base for dirty flagsets, use the macros. DOC
	struct DF {
		enum
		{
			TRS,
			Hierarchy,
			Children,
			Properties,
			Created,

			_COUNT
		};
	};


private:
	// local
	glm::vec3 m_localTranslation{ 0.f, 0.f, 0.f };
	glm::quat m_localOrientation{ 1.f, 0.f, 0.f, 0.f };
	glm::vec3 m_localScale{ 1.f, 1.f, 1.f };
	glm::mat4 m_localMatrix{};

	Box m_localBB{ glm::vec3{ 0.3f }, glm::vec3{ -0.3f } };

	// world
	glm::vec3 m_worldTranslation{ 0.f, 0.f, 0.f };
	glm::quat m_worldOrientation{ 1.f, 0.f, 0.f, 0.f };
	glm::vec3 m_worldScale{ 1.f, 1.f, 1.f };
	glm::mat4 m_worldMatrix{ glm::identity<glm::mat4>() };

	DirtyFlagset m_dirty{};

	Node* m_parent{ nullptr };

	std::vector<NodeUniquePtr> m_children;

protected:
	std::string m_name;

private:
	// Dirty Functions
	void CallDirtyUpdate() { DirtyUpdate(m_dirty); };

	friend class Editor;
	friend class World;
	friend class NodeFactory;

	void AutoUpdateTransforms();

public:
	virtual ~Node() = default;

	[[nodiscard]] glm::vec3 GetLocalTranslation() const { return m_localTranslation; }
	[[nodiscard]] glm::quat GetLocalOrientation() const { return m_localOrientation; }
	[[nodiscard]] glm::vec3 GetLocalPYR() const { return glm::degrees(glm::eulerAngles(m_localOrientation)); }
	[[nodiscard]] glm::vec3 GetLocalScale() const { return m_localScale; }
	[[nodiscard]] glm::mat4 GetLocalMatrix() const { return m_localMatrix; }

	[[nodiscard]] glm::vec3 GetLocalUp() const { return GetLocalOrientation() * glm::vec3(0.f, 1.f, 0.f); }
	[[nodiscard]] glm::vec3 GetLocalRight() const { return GetLocalOrientation() * glm::vec3(1.f, 0.f, 0.f); }
	[[nodiscard]] glm::vec3 GetLocalForward() const { return GetLocalOrientation() * glm::vec3(0.f, 0.f, -1.f); }

	[[nodiscard]] glm::vec3 GetWorldTranslation() const { return m_worldTranslation; }
	[[nodiscard]] glm::quat GetWorldOrientation() const { return m_worldOrientation; }
	[[nodiscard]] glm::vec3 GetWorldPYR() const { return glm::degrees(glm::eulerAngles(m_worldOrientation)); }
	[[nodiscard]] glm::vec3 GetWorldScale() const { return m_worldScale; }
	[[nodiscard]] glm::mat4 GetWorldMatrix() const { return m_worldMatrix; }

	[[nodiscard]] glm::vec3 GetWorldUp() const { return GetWorldOrientation() * glm::vec3(0.f, 1.f, 0.f); }
	[[nodiscard]] glm::vec3 GetWorldRight() const { return GetWorldOrientation() * glm::vec3(1.f, 0.f, 0.f); }
	[[nodiscard]] glm::vec3 GetWorldForward() const { return GetWorldOrientation() * glm::vec3(0.f, 0.f, -1.f); }

	[[nodiscard]] bool IsLeaf() const { return m_children.empty(); }
	[[nodiscard]] const std::string& GetName() const { return m_name; };
	[[nodiscard]] const std::vector<NodeUniquePtr>& GetChildren() const { return m_children; }

	[[nodiscard]] DirtyFlagset GetDirtyFlagset() const { return m_dirty; }

	// Returns nullptr IF AND ONLY IF "this" node is the root node.
	[[nodiscard]] Node* GetParent() const { return m_parent; }
	[[nodiscard]] bool IsRoot() const { return m_parent == nullptr; }
	[[nodiscard]] RootNode* GetWorldRoot() const;

	void SetLocalTranslation(glm::vec3 lt);
	void SetLocalOrientation(glm::quat lo);
	void SetLocalScale(glm::vec3 ls);
	void SetLocalMatrix(const glm::mat4& lm);

	void SetWorldTranslation(glm::vec3 wt);
	void SetWorldOrientation(glm::quat wo);
	void SetWorldScale(glm::vec3 ws);
	void SetWorldMatrix(const glm::mat4& newWorldMatrix);

	void SetLookAt(glm::vec3 lookat);

	void RotateAroundAxis(glm::vec3 worldAxis, float degrees);

	void AddLocalOffset(glm::vec3 direction);
	void AddWorldOffset(glm::vec3 direction);

	void SetName(const std::string& name) { m_name = name; }
	void DeleteChild(Node* child);
	//
	// LOADING
	//

	// Runs late in the frame, only on nodes that at least one m_dirty is set.
	virtual void DirtyUpdate(DirtyFlagset dirtyFlags){};

	virtual void Update(float deltaSeconds){};

	// cache world transform top down (and where needed to be updated)
	void UpdateTransforms(const glm::mat4& parentMatrix);

	void SetDirty(uint32 flagIndex) { m_dirty.set(flagIndex); }
	void SetDirtyMultiple(DirtyFlagset other) { m_dirty |= other; };

	//
	template<typename T>
	bool IsA()
	{
		if constexpr (std::is_same_v<T, Node>) {
			return true;
		}

		auto nodeClass = &GetClass();
		while (nodeClass != &Node::StaticClass()) {
			if (nodeClass == &T::StaticClass()) {
				return true;
			}
			nodeClass = nodeClass->GetParentClass();
		}
		return false;
	}

protected:
	// Can accept node types that are not registered to the factory
	// This will not fetch subclasses but will only match exact classes, name is case sensitive
	// The expected use of this function is to be used on Dirty DF::Children to grab actual node pointers that you can
	// safely store as members. You can be sure that the pointers will always be valid during Update()
	//
	// Any other use may cause iterator invalidation or other undefined behavior.
	//
	// NOTE: requires includes: World.h
	template<typename T>
	T* GetOrCreateChild(const std::string& name)
	{
		for (auto& childUnq : m_children) {
			Node* child = childUnq.get();
			if (child->GetClass() == T::StaticClass() && child->GetName() == name) {
				return static_cast<T*>(child);
			}
		}

		return Engine::GetWorld()->CreateNode<T>(name, this);
	}
};
