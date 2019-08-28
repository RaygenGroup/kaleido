#include "pch.h"

#include "system/Engine.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/DiskAssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"

namespace System
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
	}

	bool Engine::InitDirectories(const std::string& applicationPath, const std::string& dataDirectoryName)
	{
		m_diskAssetManager = std::make_unique<Assets::DiskAssetManager>(this);
		return m_diskAssetManager->Init(applicationPath, dataDirectoryName);
	}

	bool Engine::CreateWorldFromFile(const std::string& filename)
	{
		m_world = std::make_unique<World::World>(this);

		// load scene file
		const auto sceneXML = m_diskAssetManager->LoadXMLDocAsset(filename);

		return m_world->LoadAndPrepareWorldFromXML(sceneXML.get());
	}

	bool Engine::SwitchRenderer(RendererRegistrationIndex registrationIndex)
	{
		// replacing the old renderer will destroy it

		// construct renderer
		m_renderer = std::unique_ptr<Renderer::Renderer>(m_rendererRegistrations[registrationIndex].Construct(this));

		return m_renderer.get();
	}

	void Engine::UnloadDiskAssets()
	{
		m_diskAssetManager->UnloadAssets();
	}
}
